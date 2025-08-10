#ifndef user_H
#define user_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gpio.h"
#include "lpm.h"
#include "clk.h"
#include "uart.h"
#include "bt.h"
#include "ddl.h"
#include "Ticker.h"
#include "button.h"
#include "WIRE.h"
#include "myprintf.h"
// #include "cw2015.h"
#include "stdbool.h"
#include "rtc.h"
#include "key_value_transation.h"
#include "fast_lib.h"


#define serial_band 9600
#define mcu_sys_frq ClkFreq4Mhz

#define sys_on 32           // 连接到电源使能 SYS_EN
#define sys_dwn_active_h 33          // 连接到树莓派的GPIO_INT高电平
#define sys_dwn_active_l 34          // 连接到树莓派的GPIO_INT低电平
#define pi_shutdown_done_pin 24 // CM4关机完成检测IO（如有不同请自行调整）

#define key_io 26
// #define usb_in_io 02
// #define esp_printf_io 03
#define txd1_io 35
#define rxd1_io 36
// #define sda_io 25
// #define scl_io 26
#define sda_io 01
#define scl_io 02

#define led_io 23
// #define esp_sta_io 31
// #define intr_lp_io 32
// #define intr2_lsm_io 33
// #define intr1_lsm_io 34
// #define txd0_io 35
// #define rxd0_io 36

#endif
