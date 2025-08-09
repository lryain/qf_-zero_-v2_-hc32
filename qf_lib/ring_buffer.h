#ifndef ring_buffer_H
#define ring_buffer_H

//#include <LIB_CONFIG.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /****************************************************************/
    /*
        整形数据环形缓冲库 By 启凡科创。
        Version:1.0.0 2023-5-20
    */
    /****************************************************************/

#define ring_buffer_compile_en 1 // 库编译时能

#if ring_buffer_compile_en

    typedef enum
    {
        sample_8bit = 1,
        sample_16bit = 2,
        sample_32bit = 4
    } ring_buffer_sample_t;

    typedef struct
    {
        uint8_t *buffer;
        uint16_t *buffer_16;
        uint32_t *buffer_32;
        size_t size;
        size_t add_p;
        size_t read_p;
        ring_buffer_sample_t type;
        size_t count;
    } ring_buffer_t;

    typedef ring_buffer_t *ring_buffer_handle_t;

    /**
     * @brief 创建一个环形缓冲区
     *
     * @param sample 样本宽度，sample_8bit，sample_16bit，sample_32bit
     * @param size 样本数量
     * @return ring_buffer_handle_t 句柄，用于后续操作样本，NULL：创建失败
     */
    ring_buffer_handle_t ring_buffer_create(ring_buffer_sample_t sample, size_t size);

    /**
     * @brief 删除环形缓冲区
     *
     * @param handle 缓冲区句柄
     */
    void ring_buffer_del(ring_buffer_handle_t handle);

    /**
     * @brief 向缓冲区写一个样本
     * 
     * @param handle 缓冲区句柄
     * @param dat 指向待写入的数据，类型宽度必须等于创建的缓冲区数据宽度
     */
    void ring_buffer_write(ring_buffer_handle_t handle, void *dat);

    /**
     * @brief 环形缓冲区是否可用
     *
     * @param handle 缓冲区句柄
     * @return size_t 可用样本数
     */
    size_t ring_buffer_available(ring_buffer_handle_t handle);

    /**
     * @brief 从缓冲区读取一个样本
     *
     * @param handle 缓冲区句柄
     * @param dat 用于接收数据的变量，类型宽度必须等于创建的缓冲区数据宽度
     */
    void ring_buffer_read(ring_buffer_handle_t handle, void *dat);

#endif
#ifdef __cplusplus
}
#endif
#endif
