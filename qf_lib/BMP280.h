

#ifndef _BMP280_H
#define _BMP280_H

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif
    /**
     * @brief BMP280库by启凡科创
     * @param Version:1.0.0
     * @param DAY:2023-5-29
     */

#define bmp280_lib_compile_en 1

#if bmp280_lib_compile_en

#define BMP280_IIC_ADDRESS_8BIT 0 // 1:8位地址模式，0：7位地址模式

#if BMP280_IIC_ADDRESS_8BIT
#define BMP280_ADDR (0xEC) // 8bit器件IIC地址
#else
#define BMP280_ADDR (0x76) // 7bit器件IIC地址
#endif
#define BMP280_DEFAULT_CHIP_ID (0x58)  // ID寄存器
#define BMP280_DEBUG 0                 // 调试输出信息,1:启用，0：关闭
#define BMP280_DEBUG_LOG_PRINTF printf // 日志输出使用的函数名

    typedef void (*bmp280_write_byte_cb_t)(uint8_t add, uint8_t reg, uint8_t dat);
    typedef void (*bmp280_read_bytes_cb_t)(uint8_t add, uint8_t reg, uint8_t *dat, size_t size);

    typedef struct
    {
        bmp280_write_byte_cb_t bmp_write_byte; // 从机地址add，寄存器reg，数据dat
        bmp280_read_bytes_cb_t bmp_read_bytes; // 从机地址add，起始寄存器reg，始读size字节，存放到dat指向的内存
    } bmp280_trans_cb_t;

    /**
     * @brief 注册IIC通信函数
     *
     * @param cbs 结构体，包含读和写的回调函数指针
     */
    void bmp280_attach_trans_cb(bmp280_trans_cb_t *cbs);

    /**
     * @brief 初始化BMP280
     *
     * @return uint8_t 0：成功，1：IIC通信函数未绑定，2：硬件地址错误或硬件连接存在问题
     */
    uint8_t bmp280_begin(void);

    /**
     * @brief 读取温度、气压
     *
     * @param temp 指向存储温度的变量，如23.5°结果为235，不读取填NULL
     * @param pressure 指向存储气压的变量，不读取填NULL
     * @return uint8_t 1：成功，0：失败
     */
    uint8_t bmp280_read(int16_t *temp, int32_t *pressure);

#endif

#ifdef __cplusplus
}
#endif

#endif
