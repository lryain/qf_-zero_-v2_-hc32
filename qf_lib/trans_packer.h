#ifndef trans_packer_H
#define trans_packer_H

#ifdef __cplusplus
extern "C"
{
#endif

    /****************************************************************/
    /*
        数据通信打包解包库 By 启凡科创。
        Version:1.0.2 2023-6-9
    */
    /****************************************************************/

    // #include <LIB_CONFIG.h>

//#include "freertos/FreeRTOS.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h> //依赖库

#define trans_packer_compile_en 1 // 库编译时能

#if trans_packer_compile_en

#define cmd_monitor_en 1    // 指令监视使能
#define string_monitor_en 1 // 字符串监视使能 格式如：#@!time=230520215530\n  #@!为识别头 time为指令 = 后面接字符串数据，以\n结束

#define trans_packer_debug_log 1           // 是否打印日志
#define trans_packer_log_printf_use printf // 格式化打印函数名

#define trans_packer_malloc(x) malloc(x)
#define trans_packer_free(x) free(x)

    typedef void (*trans_packer_write_cb_t)(const char *str, size_t size);

    typedef enum
    {
        look_none,
        look_cmd,
        look_string,
        look_all
    } trans_look_t;

    typedef enum
    {
        rec_head = 0,
        rec_name_len = 1,
        rec_name = 2,
        rec_data_len = 3,
        rec_data = 4,
        rec_crc = 5,
        pack_send_log = 65535,
    } trans_rec_sta_t;

    typedef struct _get_pack_t
    {
        char *name_str;
        uint8_t *dat_buffer;
        size_t _name_lenth;
        size_t _data_lenth;
        struct _get_pack_t *next;
    } trans_get_pack_t;

    typedef struct
    {
        trans_packer_write_cb_t _write_cb;
        size_t _name_len_max;
        size_t _data_len_max;

#if (cmd_monitor_en || string_monitor_en)
        size_t get_pack_size;
        trans_get_pack_t *head;
        trans_get_pack_t *tail;
        uint8_t look_at_en;
#endif

#if trans_packer_debug_log
        uint8_t log_out_en;
#endif

#if cmd_monitor_en
        uint8_t dat_head[2];
        size_t data_count;
        uint8_t *rec_buffer_name;
        uint8_t *rec_buffer_data;
        size_t rec_p;
        size_t name_lenth;
        size_t data_lenth;
        uint16_t crc;
        trans_rec_sta_t rec_sta;
        uint8_t head_count;

#endif

#if string_monitor_en
        uint8_t dat_head_s[3];
        uint8_t *rec_buffer_name_s;
        uint8_t *rec_buffer_data_s;
        size_t rec_p_s;
        size_t data_lenth_s;
        size_t name_lenth_s;
        trans_rec_sta_t rec_sta_s;
        uint8_t head_count_s;
#endif
    } trans_packer_handle_t;

/**
 * @brief 定义三个非0字节为包头，如果两端设备包头定义不一致则无法通信
 */
// cmd通信包头,建议三个相差较大的无规律数字
#define trans_head_cmd_0 5
#define trans_head_cmd_1 124
#define trans_head_cmd_2 247

// 字符串包头，建议使用不常用字符
#define trans_head_string_0 '@'
#define trans_head_string_1 '!'
#define trans_head_string_2 '#'

    /**
     * @brief 创建通信，返回对象句柄
     *
     * @param name_len_max 包名最大长度
     * @param data_len_max 数据最大长度
     * @param en look_none,look_cmd,look_string,look_all
     * @return trans_packer_handle_t*  NULL：失败
     */
    trans_packer_handle_t *trans_packer_creat_trans(size_t name_len_max, size_t data_len_max, trans_look_t en);

#if (cmd_monitor_en || string_monitor_en)
    /**
     * @brief 设置监视模式使能
     *
     * @param handle 通信句柄
     * @param en look_none,look_cmd,look_string,look_all
     */
    void trans_packer_set_look_en(trans_packer_handle_t *handle, trans_look_t en);
#endif

#if trans_packer_debug_log

    /**
     * @brief 设置日志输出使能，默认关闭
     *
     * @param handle 通信句柄
     * @param en 1：输出日志，0：不输出
     */
    void trans_packer_set_log_en(trans_packer_handle_t *handle, uint8_t en);

#endif

    /**
     * @brief 删除通信
     *
     * @param handle 通信句柄
     */
    void trans_packer_del_trans(trans_packer_handle_t *handle);

    /**
     * @brief 绑定串口发送指定字节数的回调函数
     *
     * @param handle 通信句柄
     * @param _cb 格式为 void uart_write_bytes(const char *str, size_t size)
     * @param str 指向需要发送的数据
     * @param size 需要发送的长度
     */

    void trans_packer_set_write_cb(trans_packer_handle_t *handle, trans_packer_write_cb_t _cb);

    /**
     * @brief 发送一个包
     *
     * @param handle 通信句柄
     * @param name 包名
     * @param dat 数据
     * @param size 数据长度（字节）
     */
    void trans_packer_send_pack(trans_packer_handle_t *handle, const char *name, uint8_t *dat, size_t size);

    /**
     * @brief 格式化发送一个包
     *
     * @param handle 通信句柄
     * @param name 包名
     * @param fmt 格式化内容
     * @param ...
     */
    void trans_packer_send_pack_fmt(trans_packer_handle_t *handle, const char *name, const char *fmt, ...);

#if (cmd_monitor_en || string_monitor_en)
    /**
     * @brief 将串口接收到的一个字节同步给库缓存
     *
     * @param handle 通信句柄
     * @param dat 接收到的字节
     */
    void trans_packer_push_byte(trans_packer_handle_t *handle, uint8_t dat);

    /**
     * @brief 获取可用包数量
     *
     * @param handle 通信句柄
     * @return size_t 无可用包返回0
     */
    size_t trans_packer_get_pack_num(trans_packer_handle_t *handle);

    /**
     * @brief 获取缓存区可用的第一个包的名称长度
     *
     * @param handle 通信句柄
     * @return size_t 无可用包返回0
     */
    size_t trans_packer_get_pack_str_lenth(trans_packer_handle_t *handle);

    /**
     * @brief 获取缓存区可用的第一个包的数据长度
     *
     * @param handle 通信句柄
     * @return size_t 无可用包返回0
     */
    size_t trans_packer_get_pack_data_lenth(trans_packer_handle_t *handle);

    /**
     * @brief 获取一个数据包，只有当包名和缓存都可用才会读出数据后自动销毁已读出的包
     *
     * @param handle 通信句柄
     * @param name 用于接收包名，长度必须大于等于包名，可用trans_packer_get_pack_str_lenth获取包名长度
     * @param buffer 用于接收包数据，长度必须大于等于包数据，可用trans_packer_get_pack_data_lenth获取数据长度
     * @return size_t 1：成功，0：没有可用包
     */
    size_t trans_packer_get_pack(trans_packer_handle_t *handle, const char *name, uint8_t *buffer);
#endif

    /**
     * @brief 解析字符串为十进制数字
     *
     * @param dat 字符串数据
     * @param buffer 指向用于接收解析完成后的数字的数组，长度必须大于等于结果样本数
     * @param bytes_width 单个样本的位宽 1：8bit，2：16bit，4：32bit
     * @param dat_width 单个样本字符串宽度，为0时自动识别每个样本的任意间隔符自动分割样本，为指定宽度时在间隔符的基础上进行单个宽度限定
     */
    void trans_packer_string_to_number(char *dat, void *buffer, uint8_t bytes_width, uint8_t dat_width);

#endif
#ifdef __cplusplus
}
#endif
#endif
