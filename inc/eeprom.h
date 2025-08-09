#ifndef trans_task_H
#define trans_task_H

#include "user.h"

typedef enum
{
    alarm_none,
    alarm_standby,
    alarm_using,
} alarm_sta_t;

typedef struct
{
    stc_rtc_alarmset_t time;
    alarm_sta_t alarm_sta;
} eeprom_alarm_t;

typedef struct
{
    uint16_t chek_head;       // 识别头
    uint32_t step[7];         // 计步数
    eeprom_alarm_t alarm[30]; // 闹钟组
} eeprom_datas_t;

#define eeprom_start_add 0x7e00                   // sector63
#define eeprom_buffer_size sizeof(eeprom_datas_t) // max 512 bytes
#define eeprom_chek_head 0x9808
/*
    v1.0.0 0x9808
*/

void eeprom_init(void);

#endif
