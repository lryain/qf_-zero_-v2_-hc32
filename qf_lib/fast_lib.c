#include "fast_lib.h"

#if c99_mode
inline int strcmp_my(const char *str1, const char *str2)
{
    for (; *str1 == *str2; ++str1, ++str2)
    {
        if (*str1 == '\0')
            return (0);
    }
    return 1;
}
#else
int strcmp_my(const char *str1, const char *str2)
{
    for (; *str1 == *str2; ++str1, ++str2)
    {
        if (*str1 == '\0')
            return (0);
    }
    return 1;
}

#endif

uint8_t system_day_count_week(uint8_t year, uint8_t month, uint8_t day)
{
    int buf, buf2;
    if (month < 3)
    {
        buf = month + 12;
        buf2 = year - 1;
    }
    else
    {
        buf = month;
        buf2 = year;
    }
    buf++;
    return (buf2 + (buf2 >> 2) + (26 * buf / 10) + day - 36) % 7;
}
