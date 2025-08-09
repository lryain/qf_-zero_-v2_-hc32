#include "ring_buffer.h"

ring_buffer_handle_t ring_buffer_create(ring_buffer_sample_t sample, size_t size)
{
    ring_buffer_t *handle = malloc(sizeof(ring_buffer_t));
    if (handle == NULL)
        return NULL;
    handle->buffer = malloc(sample * size);
    if (handle->buffer == NULL)
    {
        free(handle);
        return NULL;
    }
    handle->buffer_16 = (uint16_t *)handle->buffer;
    handle->buffer_32 = (uint32_t *)handle->buffer;
    handle->add_p = 0;
    handle->read_p = 0;
    handle->size = size;
    handle->type = sample;
    handle->count = 0;
    return handle;
}

void ring_buffer_write(ring_buffer_handle_t handle, void *dat)
{
    switch (handle->type)
    {
    case sample_8bit:
        handle->buffer[handle->add_p++] = *(uint8_t *)dat;
        break;
    case sample_16bit:
        handle->buffer_16[handle->add_p++] = *(uint16_t *)dat;
        break;
    case sample_32bit:
        handle->buffer_32[handle->add_p++] = *(uint32_t *)dat;
        break;
    default:
        break;
    }
    handle->count++;
    if (handle->add_p == handle->size)
        handle->add_p = 0;
}

size_t ring_buffer_available(ring_buffer_handle_t handle)
{
    return handle->count;
}

void ring_buffer_read(ring_buffer_handle_t handle, void *dat)
{
    if (handle->count == 0)
        return;
    switch (handle->type)
    {
    case sample_8bit:
        *(uint8_t *)dat = handle->buffer[handle->read_p++];
        break;
    case sample_16bit:
        *(uint16_t *)dat = handle->buffer_16[handle->read_p++];
        break;
    case sample_32bit:
        *(uint32_t *)dat = handle->buffer_32[handle->read_p++];
        break;
    default:
        break;
    }
    if (handle->read_p == handle->size)
        handle->read_p = 0;
    handle->count--;
}

void ring_buffer_del(ring_buffer_handle_t handle)
{
    free(handle->buffer);
    free(handle);
}
