#include "myprintf.h"

#if MY_PRINTF_EN

static void (*_cb_ptr)(char dat) = NULL;
static char printf_buffer[pintf_buffer_size];

void myprintf_attach(void (*func_putchar)(char chr))
{
    _cb_ptr = func_putchar;
}

uint8_t myprintf(const char *format, ...)
{
    static va_list aptr;
    static uint8_t i, ret;
    if (_cb_ptr == NULL)
    {
        return 0;
    }

    va_start(aptr, format);
    ret = vsprintf(printf_buffer, format, aptr);
    va_end(aptr);

    for (i = 0; i < ret; i++)
    {
        _cb_ptr(printf_buffer[i]);
    }

    return (ret);
}

#endif
