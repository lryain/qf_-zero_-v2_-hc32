#ifndef devices_H
#define devices_H

#include "user.h"
#include "my_api.h"
// #include "trans_task.h"
#include "rtc_task.h"
// #include "bat_task.h"


/**
 * @brief 初始化设备
 */
void devices_init(void);

/**
 * @brief 进入休眠
 */
void devices_deep_sleep_start(void);

// /**
//  * @brief 设置振动器强度,仅在ESP休眠时可用
//  *
//  * @param duty 0-10
//  */
// void per_motor_set(uint8_t duty);

#endif
