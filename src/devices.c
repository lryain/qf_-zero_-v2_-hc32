#include "devices.h"
#include "my_api.h"
#include "trans_task.h"
#include "button.h" // 加入头文件，便于调用 btn_reset()

static uint8_t pwm_duty = 0;
static uint8_t pwm_en = 0;
static uint8_t pwm_count = 0;

// void per_motor_en(uint8_t en)
// {
//     pwm_en = en;
//     if (en)
//     {
//         gpio_set_mode(led_io, gpio_mode_output_pushpull);
//     }
//     else
//     {
//         gpio_set_level(led_io, 0);
//         gpio_set_mode(led_io, gpio_mode_input);
//         pwm_count = 0;
//     }
// }

void SysTick_Handler(void) // 系统心跳
{
    // if (pwm_en)
    // {
    //     pwm_count++;
    //     if (pwm_count == 10)
    //     {
    //         pwm_count = 0;
    //         if (pwm_duty)
    //             setBit((uint32_t)&M0P_GPIO->P0OUT + 2 * GPIO_GPSZ, 7, 1);
    //     }
    //     if (pwm_count == pwm_duty)
    //         setBit((uint32_t)&M0P_GPIO->P0OUT + 2 * GPIO_GPSZ, 7, 0);
    // }
    // if (pwm_en) // 电机使能
    // {
    //     if (getBit((uint32_t)&M0P_GPIO->P0IN + 3 * GPIO_GPSZ, 1) == 0) // ESP开机则强制关闭
    //         per_motor_en(0);
    // }

    ticker_heartbeat_ms(1);
    btn_tic_ms(1);
}

static void gpio_init()
{
    // gpio_set_mode(esp_printf_io, gpio_mode_input_pullup);

    gpio_set_mode(key_io, gpio_mode_input);
    gpio_set_mode(sys_on, gpio_mode_output_pushpull);
    // gpio_set_mode(sys_dwn_active_h, gpio_mode_output_pushpull);
    gpio_set_mode(sys_dwn_active_l, gpio_mode_output_pushpull);
    gpio_set_level(sys_on, 0); // 默认关机
    // gpio_set_level(sys_dwn_active_h, 0);
    gpio_set_level(sys_dwn_active_l, 1);

    // gpio_set_mode(led_io, gpio_mode_output_pushpull);
    // gpio_set_level(led_io, 0);

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

// 统一外设门控：运行态尽量关闭不用外设，保留 GPIO/Flash/Tick
static void peripherals_gate_run_mode(void)
{
    const en_clk_peripheral_gate_t toOff[] = {
        ClkPeripheralUart0,
        ClkPeripheralUart1,
        ClkPeripheralLpUart,
        ClkPeripheralI2c,
        ClkPeripheralSpi,
        ClkPeripheralBt,
        ClkPeripheralLpTim,
        ClkPeripheralAdt,
        ClkPeripheralPca,
        ClkPeripheralWdt,
        ClkPeripheralVcLvd,
        ClkPeripheralClkTrim,
        ClkPeripheralCrc
        // 如未使用，可继续加入：ClkPeripheralAdcBgr, ClkPeripheralRtc
    };
    for (unsigned i = 0; i < sizeof(toOff) / sizeof(toOff[0]); ++i)
    {
        Clk_SetPeripheralGate(toOff[i], FALSE);
    }
    // 必要外设门控保持开启
    Clk_SetPeripheralGate(ClkPeripheralGpio, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralFlash, TRUE);
    Clk_SetPeripheralGate(ClkPeripheralTick, TRUE);
}

// 休眠前门控：在运行态基础上，进一步关闭 Tick（配合停用 SysTick）
static void peripherals_gate_sleep_mode(void)
{
    peripherals_gate_run_mode();
    Clk_SetPeripheralGate(ClkPeripheralTick, FALSE);
    // 视需要可关闭更多外设门控
}

// 唤醒后门控：恢复 Tick，其他保持关闭，按需再开启
static void peripherals_gate_after_wakeup(void)
{
    Clk_SetPeripheralGate(ClkPeripheralTick, TRUE);
}

void devices_init(void)
{
    // 延后关闭SWD到休眠前，避免调试连接失败
    // Clk_SetFunc(ClkFuncSwdPinIOEn, 1); // 关闭SWD接口
    stc_lpm_config_t stcLpmCfg; // 配置休眠模式为deep sleep
    stcLpmCfg.enSEVONPEND = SevPndDisable;
    stcLpmCfg.enSLEEPDEEP = SlpDpEnable;
    stcLpmCfg.enSLEEPONEXIT = SlpExtDisable;
    Lpm_Config(&stcLpmCfg);

    sys_clk_init();                          // 初始化时钟
    sys_tic_set(Clk_GetHClkFreq());          // 初始化滴答定时器
    peripherals_gate_run_mode();             // 运行态统一关闭不需要的外设门控
    gpio_init();                             // 初始化gpio
    // iic_init();                              // 初始化iic
    btn_attach_read_io_func(gpio_get_level); // 初始化按键接口
    btn_attach(key_io, 0);                   // 注册按键

}

void devices_deep_sleep_start() // 开始休眠
{
    // printf("(devices_deep_sleep_start...)\n");
    // gpio_set_mode(txd1_io, gpio_mode_input_pullup); // 关串口

    gpio_set_irq(key_io, GpioIrqFalling, 1);        // 打开按键唤醒中断

// gpio_set_mode(sys_on, gpio_mode_input);
gpio_set_mode(sys_dwn_active_l, gpio_mode_input);
// gpio_set_mode(led_io, gpio_mode_input);

    // 停用 SysTick，避免1ms唤醒；并关闭 Tick 门控
    SysTick->CTRL = 0;
    peripherals_gate_sleep_mode();

    // 休眠前关闭 SWD 接口，降低待机漏电
    Clk_SetFunc(ClkFuncSwdPinIOEn, 1);

    Clk_SetFunc(ClkFuncWkupRCH, TRUE);
    
    btn_reset(); // 唤醒后立即清空按键库状态
    
    SCB->SCR |= 0x4;       //sleepdeep
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    __NOP();
    Lpm_GotoLpmMode(); // 进入休眠
    // 唤醒
    while (Clk_GetClkRdy(ClkRCH) == 0) // 等待时钟稳定
        ;

    // 唤醒后恢复 SWD 接口，便于调试
    Clk_SetFunc(ClkFuncSwdPinIOEn, 0);

    peripherals_gate_after_wakeup();        // 恢复必要门控（Tick）
    sys_tic_set(Clk_GetHClkFreq());         // 恢复 SysTick

    gpio_set_irq(key_io, GpioIrqFalling, 0);   // 关闭按键中断

    gpio_set_mode(sys_on, gpio_mode_output_pushpull);
    gpio_set_mode(sys_dwn_active_l, gpio_mode_output_pushpull);
    // gpio_set_mode(led_io, gpio_mode_output_pushpull);

    // gpio_set_mode(txd1_io, gpio_mode_output_pushpull); // 切为输出
    // gpio_set_level(txd1_io, 0);                        // 拉低唤醒ESP32
    // gpio_set_level(txd1_io, 1);
    // Gpio_SetFunc_UART1TX_P35(); // 开串口
    // ticker_delay(10); // 等待串口稳定
    // // systemWakeUp();
    // printf("(out of sleep...)\n");
}

void Gpio_IRQHandler(uint8_t u8Param)
{
    *((uint32_t *)((uint32_t)&M0P_GPIO->P0ICLR + 3 * 0x40)) = 0; // 清P3中断
    *((uint32_t *)((uint32_t)&M0P_GPIO->P0ICLR)) = 0;            // P0

    // *((uint32_t *)((uint32_t)&M0P_GPIO->P2ICLR + 2 * 0x40)) = 0; // 清P2中断
}

// void per_motor_set(uint8_t duty)
// {
//     if (getBit((uint32_t)&M0P_GPIO->P0IN + 3 * GPIO_GPSZ, 1) == 0) // ESP开机则强制关闭)
//     {
//         per_motor_en(0);
//         return;
//     }
//     pwm_duty = duty;
//     if (pwm_duty)
//         per_motor_en(1);
//     else
//         per_motor_en(0);
// }
