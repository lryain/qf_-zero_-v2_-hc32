#include "Ticker.h"
#if TICKER_LIB_USE

static ticker_datas_t *list_head = NULL;
static ticker_datas_t *list_tail = NULL;
static ticker_cb_t delay_cb = NULL;
static ticker_event_t delay_userdata;
static int32_t ticker_delay_t = 0;
static uint32_t (*_timer_get_count)() = NULL;
static ticker_timer_count_mode _count_mode;
static uint32_t tic_count = 0;
static uint8_t tic_flg = 0;
static uint32_t pass_time_ms = 0;

void ticker_attach_timer_count_cb(uint32_t (*timer_get_count)(), ticker_timer_count_mode count_mode)
{
    _timer_get_count = timer_get_count;
    _count_mode = count_mode;
}

void ticker_detach_timer_count_cb()
{
    _timer_get_count = NULL;
}

void ticker_attach_delay_cb(ticker_cb_t call_back, void *userdata)
{
    delay_cb = call_back;
    delay_userdata.userdata = userdata;
}

void ticker_detach_delay_cb()
{
    delay_cb = NULL;
}

ticker_handle_t ticker_attch_ms(uint32_t million, ticker_cb_t call_back, int16_t count, const char *name_str, void *userdata)
{

    ticker_datas_t *tmp = NULL;

    if (count < 0)
        return NULL;
    if (call_back == NULL)
        return NULL;
    if (million == 0)
        return NULL;

    tmp = malloc(sizeof(ticker_datas_t));
    if (tmp == NULL)
        return NULL;

    if (list_tail != NULL)
    {
        list_tail->next = tmp;
        tmp->last = list_tail;
    }
    if (list_head == NULL)
        list_head = tmp;

    list_tail = tmp;
    tmp->name_str = name_str;
    tmp->next = NULL;
    tmp->detach_cb = NULL;
    tmp->tick_p = call_back;                         // 注册回调函数
    tmp->tick_t_count = 0;                           // 更新计数
    tmp->tick_t_count_max = million;                 // 更新定时
    tmp->tick_t_mode_flg = count;                    // 更新次数
    tmp->ticker_userdata.handle = (const void *)tmp; // 获取句柄地址
    tmp->ticker_userdata.userdata = userdata;        // 更新用户指针
    return tmp->ticker_userdata.handle;              // 返回句柄
}

ticker_handle_t ticker_attch(uint16_t second, ticker_cb_t call_back, int16_t count, const char *name_str, void *userdata)
{
    return ticker_attch_ms((uint32_t)second * 1000, call_back, count, name_str, userdata);
}

uint8_t ticker_attch_detach_cb(ticker_handle_t handle, ticker_cb_t call_back, void *userdata)
{
    ticker_datas_t *tmp = (ticker_datas_t *)handle;
    tmp->detach_cb = call_back;
    return 1;
}

static ticker_datas_t *_find_task_handle(const char *name_str)
{
    ticker_datas_t *move = list_head;
    if (list_head == NULL)
        return NULL;
    for (;;)
    {
        if (strcmp(move->name_str, name_str) == 0)
            return move;
        if (move->next == NULL)
            return NULL;
        move = move->next;
    }
}

uint8_t ticker_has_task(ticker_handle_t handle, const char *name_str)
{
    if (list_head == NULL)
        return 0;
    if (handle == NULL)
    {
        if (_find_task_handle(name_str) == NULL)
            return 0;
        else
            return 1;
    }
    else
    {
        ticker_datas_t *move = list_head;
        ticker_datas_t *tmp = (ticker_datas_t *)handle;
        for (;;)
        {
            if (move == tmp)
                return 1;
            if (move->next == NULL)
                return 0;
            move = move->next;
        }
    }
}

void ticker_detach(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    ticker_datas_t *move;

    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;

    if (tmp->detach_cb != NULL)
        tmp->detach_cb(&tmp->ticker_userdata);

    if (tmp == list_head)
    {
        if (list_head->next == NULL)
        {
            move = NULL;
            list_tail = NULL;
        }
        else
        {
            move = list_head->next;
            move->last = NULL;
        }
            
        free(list_head);
        list_head = move;
    }
    else if (tmp == list_tail)
    {
        if (list_tail->last == NULL)
        {
            move = NULL;
            list_head = NULL;
        }
        else
        {
            move = list_tail->last;
            move->next = NULL;
        }
        free(list_tail);
        list_tail = move;
    }
    else
    {
        ticker_datas_t *_next;
        move = tmp->last;
        _next = tmp->next;
        move->next = tmp->next;
        _next->last = move;
        free(tmp);
    }
}

/**
 * @brief 暂停任务
 *
 * @param handle 册时返回的句柄
 */
void ticker_pause(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    if (tmp->tick_t_mode_flg < 0) // 已经暂停
        return;
    tmp->tick_t_mode_flg = -tmp->tick_t_mode_flg - 1; //-1 -- -32768
}

/**
 * @brief 恢复任务
 *
 * @param handle 册时返回的句柄
 */
void ticker_resume(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    if (tmp->tick_t_mode_flg >= 0) // 已经暂停
        return;
    tmp->tick_t_mode_flg = -tmp->tick_t_mode_flg - 1; //-1 -- -32768
}

void ticker_run_once(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    tmp->tick_p(&tmp->ticker_userdata);
}

/**
 * @brief 重置任务计时
 *
 * @param handle 册时返回的句柄
 */
void ticker_reset(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    tmp->tick_t_count = 0;
}

void ticker_change_time(ticker_handle_t handle, const char *name_str, uint32_t ms)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    tmp->tick_t_count = 0;
    tmp->tick_t_count_max = ms;
}

void ticker_set_count(ticker_handle_t handle, const char *name_str, int16_t count)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return;
    }
    else
        tmp = (ticker_datas_t *)handle;
    if (count < 0)
        return;
    tmp->tick_t_mode_flg = count;
}

int16_t ticker_get_count(ticker_handle_t handle, const char *name_str)
{
    ticker_datas_t *tmp;
    if (handle == NULL)
    {
        tmp = _find_task_handle(name_str);
        if (tmp == NULL)
            return -1;
    }
    else
        tmp = (ticker_datas_t *)handle;

    return tmp->tick_t_mode_flg;
}

void ticker_heartbeat_ms(uint8_t _ms)
{
    tic_count += _ms;
    pass_time_ms += _ms;
    tic_flg = 1;
    ticker_delay_t -= _ms;
}

void ticker_task_handler()
{
    static uint32_t _tic_count;

    ticker_datas_t *next = list_head;
    if (tic_flg == 0)
        return;

    _tic_count = tic_count;
    tic_count = 0;
    tic_flg = 0;

    if (list_head == NULL)
        return;
    for (;;)
    {
        next->tick_t_count += _tic_count;
        // 注册了
        if (next->tick_t_count < next->tick_t_count_max) // 定时未到
            goto chek_null;

        next->tick_t_count -= next->tick_t_count_max; // 定时到了减去一次计时
        next->tick_p(&next->ticker_userdata);         // 调用回调

        if (next->tick_t_mode_flg != 0) // 不是无限次执行
        {
            next->tick_t_mode_flg--;        // 次数减1
            if (next->tick_t_mode_flg == 0) // 执行结束
            {
                if (next->detach_cb != NULL)
                    next->detach_cb(&next->ticker_userdata);
                ticker_detach((ticker_handle_t)next, NULL);
            }
        }
    chek_null:
        if (next->next == NULL)
            return;
        next = next->next;
    }
}

void ticker_delay(uint32_t ms)
{
    uint32_t timer_count = 0;
    if (_timer_get_count != NULL)
        timer_count = _timer_get_count();
    ticker_delay_t = ms;
    while (ticker_delay_t > 0)
    {
        ticker_task_handler();
        if (delay_cb != NULL)
            delay_cb(&delay_userdata);
    }
    if (_timer_get_count == NULL)
        return;
    if (_count_mode == count_down)
    {
        while (_timer_get_count() > timer_count)
        {
            ticker_task_handler();
            if (delay_cb != NULL)
                delay_cb(&delay_userdata);
        }
        return;
    }
    while (_timer_get_count() < timer_count)
    {
        ticker_task_handler();
        if (delay_cb != NULL)
            delay_cb(&delay_userdata);
    }
}

void ticker_delay_without_task(uint32_t ms)
{
    uint32_t timer_count = 0;
    if (_timer_get_count != NULL)
        timer_count = _timer_get_count();
    ticker_delay_t = ms;
    while (ticker_delay_t > 0)
    {
        if (delay_cb != NULL)
            delay_cb(&delay_userdata);
    }
    if (_timer_get_count == NULL)
        return;
    if (_count_mode == count_down)
    {
        while (_timer_get_count() > timer_count)
        {
            if (delay_cb != NULL)
                delay_cb(&delay_userdata);
        }
        return;
    }
    while (_timer_get_count() < timer_count)
    {
        if (delay_cb != NULL)
            delay_cb(&delay_userdata);
    }
}

uint32_t ticker_get_time_ms()
{
    return pass_time_ms;
}

#endif
