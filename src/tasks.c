#include "tasks.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// 关机延时（ms），非CM4建议5000~10000ms
#define SHUTDOWN_DELAY_MS 3000

// 系统状态变量
static uint8_t system_state = 0; // 0关机，1开机
static uint8_t shutdown_pending = 0;
static uint8_t cm4_flag = 0; // 1为CM4，0为其它型号
static uint32_t shutdown_timer = 0;

// 定义一个函数来恢复高电平
void sys_dwn_active_l_delay(void)
{
    gpio_set_level(sys_dwn_active_l, 0); // 恢复高电平
    ticker_delay(300);
    gpio_set_level(sys_dwn_active_l, 1); // 恢复高电平
}

static void btn_task()
{
    static btn_event_t ret;
    // if (btn_available() && gpio_get_level(key_io) == 1)
    if (btn_available())
    {
        btn_read_event(NULL, &ret);
        switch (ret)
        {
        case btn_click:
            // printf("(click)\n");
            if (system_state == 0) { // 关机状态
                // printf("(power on)\n");
                gpio_set_level(sys_on, 1); // 上电
                system_state = 1;
                sys_dwn_active_l_delay(); // 延时恢复高电平
            }
            break;
        case btn_double_click:
            if (system_state == 1) { // 开机状态
                // printf("(power off)\n");
                // 通知树莓派关机
                gpio_set_level(sys_dwn_active_l, 0); // 输出低电平
                sys_dwn_active_l_delay(); // 延时恢复高电平

                shutdown_pending = 1;
                shutdown_timer = ticker_get_time_ms();
            }
            break;
        case btn_long_press:
            // printf("(long)\n");
            break;
        case btn_long_press_8s:
            if (system_state == 1) { // 开机状态
                // printf("(force power off)\n");
                gpio_set_level(sys_on, 0); // 强制断电
                system_state = 0;
                shutdown_pending = 0;
                devices_deep_sleep_start();
                // 立即返回，防止关机流程继续
                return;
            }
            break;
        default:
            break;
        }
    }
}

void tasks_init()
{
    // esp_trans_init(); // 初始化主机通信
    stc_rtc_time_t time;
    memset(&time, 0, sizeof(stc_rtc_time_t));
    time.u8Year = 0x23;
    time.u8Month = 0x04;
    time.u8Day = 0x22;
    time.u8Hour = 0x12;
    time.u8Minute = 0x34;
    rtc_task_init(&time); // 初始化RTC
    // bat_task_init();      // 初始化电池信息
    ticker_attch_ms(1, btn_task, 0, "btn", NULL); // 1ms上报一次数据
}

void task_loop_handler()
{
    ticker_task_handler();
    if (shutdown_pending) {
        // 关机信号保持一段时间后拉低
        if (ticker_get_time_ms() - shutdown_timer > 800) {
            // gpio_set_level(sys_dwn_active_l, 0); // 关机信号拉高
        }

        if (system_state == 0) {
            // 已经断电，直接清除关机流程
            shutdown_pending = 0;
            devices_deep_sleep_start();
            return;
        }
        if (cm4_flag) {
            // 检测CM4关机完成引脚
            if (gpio_get_level(pi_shutdown_done_pin) == 0) {
                gpio_set_level(sys_on, 0); // 断电
                shutdown_pending = 0;
                system_state = 0;
                // 关机完成，进入低功耗模式
                devices_deep_sleep_start();
            }
        } else {
            // 非CM4，延时关机
            if (ticker_get_time_ms() - shutdown_timer > SHUTDOWN_DELAY_MS) {
                gpio_set_level(sys_on, 0); // 断电
                shutdown_pending = 0;
                system_state = 0;
                // 关机完成，进入低功耗模式
                devices_deep_sleep_start();
            }
        }
    }
    
}
