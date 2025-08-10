#ifndef fast_lib_H
#define fast_lib_H

#ifdef __cplusplus
extern "C"
{
#endif

    /*
        Chinese is encoded in Unicode. If it is garbled, please change the encoding method of the editor.
        小API库 By启凡科创
        version:v1.0.1  2023-7-21
    */

#include <stdlib.h>
#include <stdint.h>

#define c99_mode 1//1：支持C99，0：远古规范

    /**
     * @brief 快速对比字符串
     *
     * @param str1
     * @param str2
     * @return int 0：一致，1：不一致
     */
    int strcmp_my(const char *str1, const char *str2);

    /**
     * @brief 给定年月日计算出周数
     * 
     * @param year 年
     * @param month 月
     * @param day 日
     * @return uint8_t 
     */
    uint8_t system_day_count_week(uint8_t year, uint8_t month, uint8_t day);

#ifdef __cplusplus
}
#endif

#endif
