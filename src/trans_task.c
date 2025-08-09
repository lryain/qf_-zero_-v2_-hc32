#include "trans_task.h"
#include "my_api.h"
#include "devices.h"
#include "trans_packer.h"
#include "ring_buffer.h"

#define uart_printf_to_usb 0

static ring_buffer_handle_t ring_buffer;
static trans_packer_handle_t *handle = NULL;

trans_packer_handle_t *esp_trans_get_handle()
{
    return handle;
}

static void number_to_bcd(uint8_t *num)
{
    uint8_t shi;
    if (*num >= 10)
    {
        shi = *num / 10;
        *num -= shi * 10;
        *num |= shi << 4;
    }
}

static void time_to_bcd(stc_rtc_time_t *time)
{
    number_to_bcd(&time->u8Year);
    number_to_bcd(&time->u8Month);
    number_to_bcd(&time->u8Day);
    number_to_bcd(&time->u8Hour);
    number_to_bcd(&time->u8Minute);
    number_to_bcd(&time->u8Second);
}

void scan_iic_devices()
{
    wire_handle_t iic = iic_get_handle();
    trans_packer_send_pack(handle, "log", "start scan devices", pack_send_log);

    if (wire_beginTransmission(iic, 0x76))
        trans_packer_send_pack(handle, "log", "BMP280 is online,address:0x76", pack_send_log);
    else
        trans_packer_send_pack(handle, "log", "BMP280 is offline,please chek hardware", pack_send_log);
    wire_endTransmission(iic);

    if (wire_beginTransmission(iic, 0x62))
        trans_packer_send_pack(handle, "log", "CW2015 is online,address:0x62", pack_send_log);
    else
        trans_packer_send_pack(handle, "log", "CW2015 is offline,please chek hardware", pack_send_log);
    wire_endTransmission(iic);

    if (wire_beginTransmission(iic, 0x0D))
        trans_packer_send_pack(handle, "log", "QMC5883L is online,address:0x0D", pack_send_log);
    else
        trans_packer_send_pack(handle, "log", "QMC5883L is offline,please chek hardware", pack_send_log);
    wire_endTransmission(iic);

    if (wire_beginTransmission(iic, 0x6A))
        trans_packer_send_pack(handle, "log", "LSM6DSL is online,address:0x6A", pack_send_log);
    else
        trans_packer_send_pack(handle, "log", "LSM6DSL is offline,please chek hardware", pack_send_log);
    wire_endTransmission(iic);
}

static void tic_cb(ticker_event_t *e)
{
    if (ring_buffer_available(ring_buffer)) // 读出串口缓冲区的数据推送到编解码器
    {
        uint8_t tmp;
        ring_buffer_read(ring_buffer, &tmp);
        trans_packer_push_byte(handle, tmp);
    }

    if (trans_packer_get_pack_num(handle)) // 编解码器接收到可用包
    {
        const char *name;
        uint8_t *dat;
        name = malloc(trans_packer_get_pack_str_lenth(handle));
        dat = malloc(trans_packer_get_pack_data_lenth(handle));
        trans_packer_get_pack(handle, name, dat);

        // printf("\nrec cmd :%s\n", name);

        if (strcmp(name, "get_time") == 0)
        {
            rtc_upload_time();
        }
        else if (strcmp(name, "hc_sleep") == 0)
        {
            // trans_packer_send_pack(handle, "esp sleep", NULL, 0);
            //  ticker_delay_without_task(10);
            devices_deep_sleep_start(); // 自身休眠
        }
        else if (strcmp(name, "set_time") == 0)
        {
            stc_rtc_time_t time;

            //uint8_t time_tmp[6];

            //trans_packer_string_to_number((char *)dat, time_tmp, 1, 0);

            time.u8Year = dat[0];
            time.u8Month = dat[1];
            time.u8Day = dat[2];
            time.u8Hour = dat[3];
            time.u8Minute = dat[4];
            time.u8Second = dat[5];

            time_to_bcd(&time);

            rtc_task_init(&time);
            //rtc_upload_time();
        }
        else if (strcmp(name, "scan") == 0)
        {
            scan_iic_devices();
        }
        else if (strcmp(name, "set motor") == 0)
        {
            per_motor_set(dat[0]);
        }
        free((void *)name);
        free(dat);
    }
}

//////////////////////////////////////////////////////////////////////

int fputc(int ch, FILE *f)
{
    Uart_SendData(UARTCH1, ch);
    return ch;
}

void uart_write_bytes(const char *dat, size_t size)
{
    while (size--)
    {
        Uart_SendData(UARTCH1, *dat++);
    }
}

void RxIntCallback(void)
{
    uint8_t tmp = M0P_UART1->SBUF;
    ring_buffer_write(ring_buffer, &tmp);
}

static void uart_init()
{

    uint16_t timer = 0;
    uint32_t pclk = 0;

    stc_uart_config_t stcConfig;
    stc_uart_irq_cb_t stcUartIrqCb;
    stc_uart_multimode_t stcMulti;
    stc_uart_baud_config_t stcBaud;
    stc_bt_config_t stcBtConfig;

    DDL_ZERO_STRUCT(stcUartIrqCb);
    DDL_ZERO_STRUCT(stcMulti);
    DDL_ZERO_STRUCT(stcBaud);
    DDL_ZERO_STRUCT(stcBtConfig);

#if uart_printf_to_usb
    gpio_set_mode(txd0_io, gpio_mode_output_pushpull);
    gpio_set_mode(rxd0_io, gpio_mode_input_pullup);

    // 通道端口配置
    Gpio_SetFunc_UART1TX_P35();
    Gpio_SetFunc_UART1RX_P36();
#else
    gpio_set_mode(txd1_io, gpio_mode_output_pushpull);
    gpio_set_mode(rxd1_io, gpio_mode_input);
    // Gpio_SetFunc_UART1TX_P23();
    // Gpio_SetFunc_UART1RX_P24();
    Gpio_SetFunc_UART1TX_P35();
    Gpio_SetFunc_UART1RX_P36();
#endif

    // 外设时钟使能
    Clk_SetPeripheralGate(ClkPeripheralBt, TRUE);    // 定时器0时钟使能 模式0/2可以不使能
    Clk_SetPeripheralGate(ClkPeripheralUart1, TRUE); // 串口1时钟使能

    stcUartIrqCb.pfnRxIrqCb = RxIntCallback; // 接收回调
    stcUartIrqCb.pfnTxIrqCb = NULL;
    stcUartIrqCb.pfnRxErrIrqCb = NULL;

    stcMulti.enMulti_mode = UartNormal; // 常规模式

    stcConfig.pstcIrqCb = &stcUartIrqCb; // 绑定回调结构体
    stcConfig.pstcMultiMode = &stcMulti; // 绑定模式

    stcConfig.bTouchNvic = 1;        // 中断使能
    stcConfig.enRunMode = UartMode1; // 工作在模式3

    stcBaud.bDbaud = 0;                                // 双倍波特率功能
    stcBaud.u32Baud = serial_band;                     // 更新波特率位置
    stcBaud.u8Mode = UartMode1;                        // 计算波特率需要模式参数
    pclk = Clk_GetPClkFreq();                          // 获取系统时钟频率
    timer = Uart_SetBaudRate(UARTCH1, pclk, &stcBaud); // 设置串口波特率，返回定时器初始值

    stcBtConfig.enMD = BtMode2;  // 定时器模式2，16位重装载
    stcBtConfig.enCT = BtTimer;  // 定时模式
    Bt_Init(TIM1, &stcBtConfig); // 初始化定时器1
    Bt_ARRSet(TIM1, timer);      // 设置定时器重载值
    Bt_Cnt16Set(TIM1, timer);    // 设置定时器初始值
    Bt_Run(TIM1);                // 开始计时

    Uart_Init(UARTCH1, &stcConfig);      // 初始化串口1
    Uart_EnableIrq(UARTCH1, UartRxIrq);  // 使能中断
    Uart_ClrStatus(UARTCH1, UartRxFull); // 清除标志
    Uart_DisableIrq(UARTCH1, UartTxIrq); // 关闭发送中断
    Uart_EnableFunc(UARTCH1, UartRx);    // 使能接收中断

    ring_buffer = ring_buffer_create(sample_8bit, 64);
}

void esp_trans_init()
{
    uart_init();
// #if !uart_printf_to_usb
//     gpio_set_mode(txd0_io, gpio_mode_input);
//     gpio_set_mode(rxd0_io, gpio_mode_input);
// #endif
    ticker_attch_ms(1, tic_cb, 0, NULL, NULL);

    handle = trans_packer_creat_trans(24, 64, look_cmd);
    trans_packer_set_write_cb(handle, uart_write_bytes);
}
