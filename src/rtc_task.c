#include "rtc_task.h"
#include "my_api.h"
#include "trans_packer.h"
// #include "trans_task.h"
#include "devices.h"

uint8_t flg;

static void RtcCycCb(void) // 周期回调函数
{
    Rtc_ClrIrqStatus(RtcPrdf);
    flg |= 1;
}

static void RtcAlarmCb(void) // 闹钟回调函数
{
    Rtc_ClrIrqStatus(RtcAlmf);
    flg |= 2;
}

// void rtc_upload_time()
// {
//     stc_rtc_time_t time;
//     Rtc_ReadDateTime(&time);
//     uint8_t time_tmp[7] = {time.u8Year, time.u8Month, time.u8Day, time.u8DayOfWeek, time.u8Hour, time.u8Minute, time.u8Second};
//     for (size_t i = 0; i < sizeof(time_tmp); i++)
//         time_tmp[i] = Change_DateTimeFormat(time_tmp[i]);
//     trans_packer_send_pack(esp_trans_get_handle(), "time", time_tmp, sizeof(time_tmp));
// }

// 新增一个定时器任务
void led_flash_task(void)
{
    // printf("(led_flash_task)\n");
    gpio_set_level(led_io, 1);
    ticker_delay(10);
    gpio_set_level(led_io, 0);
    ticker_delay(10);
}

void rtc_task_init(stc_rtc_time_t *time)
{

#if 1
    stc_rtc_config_t stcRtcConfig;
    stc_rtc_irq_cb_t stcIrqCb;
    stc_rtc_time_t stcTime;
    stc_rtc_alarmset_t stcAlarm;
    stc_rtc_cyc_sel_t stcCycSel;

    DDL_ZERO_STRUCT(stcRtcConfig);
    DDL_ZERO_STRUCT(stcIrqCb);
    DDL_ZERO_STRUCT(stcAlarm);
    DDL_ZERO_STRUCT(stcTime);

    if (ticker_has_task(NULL, "rtc"))
        Rtc_DeInit();

    Clk_Enable(ClkXTL, TRUE); // 开启外部晶振32.768

    Clk_SetPeripheralGate(ClkPeripheralRtc, TRUE); // 使能rtc时钟

    stcRtcConfig.enClkSel = RtcClk32768; // 外部32.768
    stcRtcConfig.enAmpmSel = Rtc24h;     // Rtc12h;

    stcTime = *time;

    stcTime.u8DayOfWeek = Rtc_CalWeek(&stcTime.u8Day);
    stcRtcConfig.pstcTimeDate = &stcTime; // 初始化时间参数

    stcIrqCb.pfnAlarmIrqCb = RtcAlarmCb;
    stcIrqCb.pfnTimerIrqCb = RtcCycCb;
    stcRtcConfig.pstcIrqCb = &stcIrqCb; // 初始化回调函数

    stcCycSel.enCyc_sel = RtcPrads;       // 周期类型
    stcCycSel.enPrds_sel = Rtc_None;      // 1min中断一次
    stcRtcConfig.pstcCycSel = &stcCycSel; // 初始化周期中断

    stcRtcConfig.bTouchNvic = TRUE; // 中断使能

    Rtc_DisableFunc(RtcCount); // 关闭计数
    Rtc_Init(&stcRtcConfig);   // 初始化RTC
    // 在初始化时注册
    // ticker_attch_ms(2000, led_flash_task, 0, "led_flash", NULL);
#if 0
    Gpio_SetFunc_RTC1HZ_P03();
    Clk_SetRTCAdjustClkFreq(CLK_XTH_VAL); // 以高速Pclk来进行补偿,此处要根据具体PCLK时钟源确定
    Rtc_Set1HzMode(1);                    // 0普通模式，1高精度模式
    Rtc_EnableFunc(Rtc1HzOutEn);
    Rtc_SetCompCr(0x0ff); // 此处补偿值要根据实际1hz误差偏差大小来设置，此处只是举例说明
    Rtc_EnableFunc(Rtc_ComenEn);
#endif
    Rtc_EnableFunc(RtcCount); // 开启计数

    // Rtc_EnableFunc(RtcAlarmEn); // 使能闹钟

#endif
    // if (ticker_has_task(NULL, "rtc") == 0)
    //     ticker_attch_ms(1000, tic_cb, 0, "rtc", NULL);
}
