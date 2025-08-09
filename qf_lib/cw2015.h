#ifndef __CW2015_H__
#define __CW2015_H__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define CW2015_ADDR 0x62

#define CW_DEBUG 1 // 调试输出信息,如果启用，信息输出会从printf打印，请先重定向好printf

    /*电池建模信息，客户拿到自己电池匹配的建模信息后请替换*/
    static const unsigned char cw_bat_config_info[] = {
        0X15, 0X7E, 0X7C, 0X5C, 0X64, 0X6A, 0X65, 0X5C, 0X55, 0X53, 0X56, 0X61, 0X6F, 0X66, 0X50, 0X48,
        0X43, 0X42, 0X40, 0X43, 0X4B, 0X5F, 0X75, 0X7D, 0X52, 0X44, 0X07, 0XAE, 0X11, 0X22, 0X40, 0X56,
        0X6C, 0X7C, 0X85, 0X86, 0X3D, 0X19, 0X8D, 0X1B, 0X06, 0X34, 0X46, 0X79, 0X8D, 0X90, 0X90, 0X46,
        0X67, 0X80, 0X97, 0XAF, 0X80, 0X9F, 0XAE, 0XCB, 0X2F, 0X00, 0X64, 0XA5, 0XB5, 0X11, 0XD0, 0X11};

    /*电池信息结构体*/
    typedef struct
    {
        uint8_t soc;
        uint8_t low_soc;
        uint16_t voltage;
        uint16_t rtt;
    } cw2015_bat_info_t;

    typedef struct
    {
        void (*iic_write_bytes)(uint8_t add, uint8_t reg, uint8_t *dat, size_t size); // 从机地址add，起始寄存器reg，写dat指向的内存，长度size字节
        void (*iic_read_bytes)(uint8_t add, uint8_t reg, uint8_t *dat, size_t size);  // 从机地址add，起始寄存器reg，始读size字节，存放到dat指向的内存
    } cw2015_trans_cb_t;

    typedef enum
    {
        cw_auto_init,
        cw_force_init
    } cw2015_init_mode_t;

#define cw2015_rtt_max 8191 // 预估可用时长超出最大量程

    /**
     * @brief 初始化库仑计
     *
     * @param mode cw_force_init：强制初始化,cw_auto_init：如已经初始化过则会跳过执行
     */
    void cw2015_init(cw2015_init_mode_t mode);

    /**
     * @brief 获取电池信息
     *
     * @param info
     */
    void cw2015_get_info(cw2015_bat_info_t *info); // 更新数据

    /**
     * @brief 清除中断，释放IO
     */
    void cw2015_release_alrt_pin(void);

    /**
     * @brief 设置ATHD
     *
     * @param new_athd 0-31%
     */
    void cw2015_set_athd(uint8_t new_athd);

    /**
     * @brief 注册IIC通信函数
     *
     * @param cbs 结构体，包含读和写的回调函数指针
     */
    void cw2015_attach_trans_cb(cw2015_trans_cb_t *cbs);

#ifdef __cplusplus
}
#endif

#endif
