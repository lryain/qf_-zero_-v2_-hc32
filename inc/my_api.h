#ifndef my_api_H
#define my_api_H

#include "user.h"

typedef enum
{
    gpio_mode_input,
    gpio_mode_input_pullup,
    gpio_mode_input_pulldown,
    gpio_mode_output_pushpull,
    gpio_mode_output_od,
    gpio_mode_output_od_pullup
} gpio_mode_t;

/**
 * @brief 配置GPIO模式
 *
 * @param pin
 * @param mode
 */
void gpio_set_mode(uint8_t pin, gpio_mode_t mode);

/**
 * @brief 设置GPIO电平
 *
 * @param pin
 * @param level
 */
void gpio_set_level(uint8_t pin, boolean_t level);

/**
 * @brief 获取GPIO电平
 *
 * @param pin
 * @return boolean_t
 */
boolean_t gpio_get_level(uint8_t pin);

/**
 * @brief 配置GPIO中断
 *
 * @param io
 * @param mode 模式
 * @param en 使能
 */
void gpio_set_irq(uint8_t io, en_gpio_irqtype_t mode, boolean_t en);

/**
 * @brief 初始化IIC
 */
void iic_init(void);

/**
 * @brief 获取IIC句柄
 *
 * @return wire_handle_t
 */
wire_handle_t iic_get_handle(void);

/**
 * @brief IIC写字节
 * 
 * @param add 从机地址
 * @param reg 寄存器地址
 * @param dat 数据
 */
void iic_write_byte(uint8_t add, uint8_t reg, uint8_t dat);

/**
 * @brief IIC读字节
 * 
 * @param add 从机地址
 * @param reg 寄存器地址
 * @return uint8_t 
 */
uint8_t iic_read_byte(uint8_t add, uint8_t reg);

/**
 * @brief iic写多字节
 *
 * @param add 从机地址
 * @param reg 寄存器地址
 * @param dat 数据地址
 * @param size 长度（字节）
 */
void iic_write_bytes(uint8_t add, uint8_t reg, uint8_t *dat, size_t size);

/**
 * @brief IIC读多字节
 *
 * @param add 从机地址
 * @param reg 寄存器地址
 * @param dat 数据地址
 * @param size 长度（字节）
 */
void iic_read_bytes(uint8_t add, uint8_t reg, uint8_t *dat, size_t size);

#endif
