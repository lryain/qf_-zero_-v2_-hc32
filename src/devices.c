#include "devices.h"
#include "my_api.h"
#include "trans_task.h"

static uint8_t pwm_duty = 0;
static uint8_t pwm_en = 0;
static uint8_t pwm_count = 0;

void per_motor_en(uint8_t en)
{
    pwm_en = en;
    if (en)
    {
        gpio_set_mode(led_io, gpio_mode_output_pushpull);
    }
    else
    {
        gpio_set_level(led_io, 0);
        gpio_set_mode(led_io, gpio_mode_input);
        pwm_count = 0;
    }
}

void SysTick_Handler(void) // 系统心跳
{
    if (pwm_en)
    {
        pwm_count++;
        if (pwm_count == 10)
        {
            pwm_count = 0;
            if (pwm_duty)
                setBit((uint32_t)&M0P_GPIO->P0OUT + 2 * GPIO_GPSZ, 7, 1);
        }
        if (pwm_count == pwm_duty)
            setBit((uint32_t)&M0P_GPIO->P0OUT + 2 * GPIO_GPSZ, 7, 0);
    }
    if (pwm_en) // 电机使能
    {
        if (getBit((uint32_t)&M0P_GPIO->P0IN + 3 * GPIO_GPSZ, 1) == 0) // ESP开机则强制关闭
            per_motor_en(0);
    }

    ticker_heartbeat_ms(1);
    btn_tic_ms(1);
}

static void gpio_init()
{
    // gpio_set_mode(esp_printf_io, gpio_mode_input_pullup);

    gpio_set_mode(key_io, gpio_mode_input);
    gpio_set_mode(sys_on, gpio_mode_output_pushpull);
    gpio_set_mode(sys_dwn, gpio_mode_output_pushpull);
    gpio_set_level(sys_on, 0);
    gpio_set_level(sys_dwn, 0);

    gpio_set_mode(led_io, gpio_mode_output_pushpull);
    gpio_set_level(led_io, 0);

    // gpio_set_mode(intr_lp_io, gpio_mode_input);
    // gpio_set_mode(intr1_lsm_io, gpio_mode_input);
    // gpio_set_mode(intr2_lsm_io, gpio_mode_input);

    // gpio_set_mode(usb_in_io, gpio_mode_input);
}

static void sys_clk_init()
{
    stc_clk_config_t stcCfg;

    stcCfg.enClkSrc = ClkRCH;   // RCH  = 4M
    stcCfg.enHClkDiv = ClkDiv1; // HCLK = 4M/128
    stcCfg.enPClkDiv = ClkDiv1; // PCLK = 4M/128/8

    Clk_Init(&stcCfg);
    Clk_SetRCHFreq(mcu_sys_frq);
    while (Clk_GetClkRdy(ClkRCH) == 0)
        ;
}

static void sys_tic_set(uint32_t sys_frq)
{
    stc_clk_systickcfg_t stcCfg;
    stcCfg.enClk = ClkRCH;
    stcCfg.u32LoadVal = sys_frq / 1000 - 1; // 1ms
    Clk_SysTickConfig(&stcCfg);
    SysTick_Config(stcCfg.u32LoadVal);
}

void devices_init(void)
{
    // Clk_SetFunc(ClkFuncSwdPinIOEn, 1); // 关闭SWD接口

    stc_lpm_config_t stcLpmCfg; // 配置休眠模式为deep sleep
    stcLpmCfg.enSEVONPEND = SevPndDisable;
    stcLpmCfg.enSLEEPDEEP = SlpDpEnable;
    stcLpmCfg.enSLEEPONEXIT = SlpExtDisable;
    Lpm_Config(&stcLpmCfg);

    sys_clk_init();                          // 初始化时钟
    sys_tic_set(Clk_GetHClkFreq());          // 初始化滴答定时器
    gpio_init();                             // 初始化gpio
    iic_init();                              // 初始化iic
    btn_attach_read_io_func(gpio_get_level); // 初始化按键接口
    btn_attach(key_io, 0);                   // 注册按键
}

void devices_deep_sleep_start() // 开始休眠
{
    gpio_set_mode(txd1_io, gpio_mode_input_pullup); // 关串口
    gpio_set_irq(key_io, GpioIrqFalling, 1);        // 打开按键唤醒中断
    // gpio_set_irq(usb_in_io, GpioIrqRising, 1);      // 打开USB唤醒中断

    Lpm_GotoLpmMode(); // 进入休眠

    // 唤醒
    while (Clk_GetClkRdy(ClkRCH) == 0) // 等待时钟稳定
        ;
    gpio_set_irq(key_io, GpioIrqFalling, 0);   // 关闭按键中断
    // gpio_set_irq(usb_in_io, GpioIrqRising, 0); // 关闭USB唤醒中断

    gpio_set_mode(txd1_io, gpio_mode_output_pushpull); // 切为输出
    gpio_set_level(txd1_io, 0);                        // 拉低唤醒ESP32
    gpio_set_level(txd1_io, 1);

    Gpio_SetFunc_UART1TX_P23(); // 开串口
}

void Gpio_IRQHandler(uint8_t u8Param)
{
    *((uint32_t *)((uint32_t)&M0P_GPIO->P0ICLR + 3 * 0x40)) = 0; // 清P3中断
    *((uint32_t *)((uint32_t)&M0P_GPIO->P0ICLR)) = 0;            // P0
}

void per_motor_set(uint8_t duty)
{
    if (getBit((uint32_t)&M0P_GPIO->P0IN + 3 * GPIO_GPSZ, 1) == 0) // ESP开机则强制关闭)
    {
        per_motor_en(0);
        return;
    }
    pwm_duty = duty;
    if (pwm_duty)
        per_motor_en(1);
    else
        per_motor_en(0);
}
