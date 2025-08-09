#include "tasks.h"


static void btn_task()
{
    static btn_event_t ret;
    if (btn_available() && gpio_get_level(key_io) == 1)
    {
        btn_read_event(NULL, &ret);
        switch (ret)
        {
        // case btn_down:
        //     printf("(btn_down)\n");
        //     break;
        // case btn_up:
        //     printf("(btn_up)\n");
        //     break;
        case btn_click:
            printf("(click)\n");
            break;
        case btn_double_click:
            printf("(double)\n");
            break;
        case btn_triple_click:
            printf("(triple)\n");
            break;
        case btn_long_press:
            printf("(long)\n");
            break;
        case btn_long_press_8s:
            printf("(long_8s)\n");
            break;
        default:
            break;
        }
    }
}

void tasks_init()
{
    esp_trans_init(); // 初始化主机通信
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
}
