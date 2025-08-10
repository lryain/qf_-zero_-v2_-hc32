#ifndef _key_value_transation_H__
#define _key_value_transation_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /*
        Chinese is encoded in Unicode. If it is garbled, please change the encoding method of the editor.
        简单键值对数据交互库 By启凡科创
        version:v1.0.1  2023-7-21
    */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define key_value_transation_compile_en 1

#define key_value_malloc_func(x) malloc(x)
#define key_value_free_func(x) free(x)

#if key_value_transation_compile_en

    typedef void (*key_value_cb_t)(void *value, size_t lenth);

    typedef struct _key_value_register_t
    {
        key_value_cb_t _cb;
        const char *key;
        struct _key_value_register_t *next;
        struct _key_value_register_t *last;
        int key_sum;
        uint8_t del_flg;
    } key_value_register_t;

    typedef key_value_register_t *key_value_handle_t;

    /**
     * @brief 注册(订阅)有指定键时的回调函数
     *
     * @param handle 用于接收返回句柄
     * @param key 键名
     * @param cb 回调函数
     * @return int 0：成功，1：键值对内存申请失败,2:键名内存申请失败
     */
    int key_value_register(key_value_handle_t *handle, const char *key, key_value_cb_t cb);

    /**
     * @brief 删除已注册的键
     *
     * @param handle 注册的句柄
     * @return int 0：成功，1：不存在句柄
     */
    int key_value_del(key_value_handle_t handle);

    /**
     * @brief 创建(发布)一条键值对通知
     *
     * @param key 键名，可以是动态的
     * @param value 指向数据，可以是动态的
     * @param lenth 数据长度，字节
     * @return int 0：成功，1：无注册键，2：无指定键
     */
    int key_value_msg(const char *key, void *value, size_t lenth);

#endif

#ifdef __cplusplus
}
#endif
#endif
