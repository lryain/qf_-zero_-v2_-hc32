#ifndef rtc_task_H
#define rtc_task_H

#include "user.h"


void rtc_task_init(stc_rtc_time_t *time);
void rtc_upload_time(void);

#endif
