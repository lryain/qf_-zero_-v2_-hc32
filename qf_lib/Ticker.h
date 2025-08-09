#ifndef __TICKER_H__
#define __TICKER_H__
#ifdef __cplusplus
extern "C"
{
#endif
    /*
        Chinese is encoded in Unicode. If it is garbled, please change the encoding method of the editor.
        易移植软件定时器库 By启凡科创 适用于任意C/C++平台MCU

        功能：
        1）实时动态创建、删除任务
        2）对任务可进行：暂停、恢复、计时重置的操作
        3）支持用户带入用户数据指针
        4）任务支持指定运行次数，运行结束后自行销毁

        移植步骤：
        1）确保malloc函数可申请的内存池大小足够，1个任务使用内存大概为72字节
        2）在当前平台MCU中配置好中断在ms等级的定时器，将ticker_heartbeat_ms()函数放置于定时器的中断函数里提供心跳
        3）将ticker_task_handler()接口放置于主函数mian的死循环下进行循环扫描，或将ticker_delay()接口放进去也是一样的，区别在于有没有延时，
            ticker_delay时也在循环扫描ticker_task_handler(),所以在使用delay时可不用ticker_task_handler;

        至此，你已经完成了Ticker库的移植

        使用步骤:
        1）使用attach相关接口将要定时执行的子函数注册到ticker，定时到达后将自动执行
        2）使用detach可注销指定任务,指定了次数的任务可中途销毁也可任其执行完次数后自动销毁
        3）使用attach注册成功后，会返回一个类型为ticker_handle_t的句柄，后续对任务的操作都将采用此句柄，也可使用注册时给定的任务名字进行操作
            如需对任务进行干预，请务必创建好一个静态局部句柄或全局句柄用于保存任务对象，或给任务取好名字

        注意：ticker内执行的任务尽量不要阻塞（如大量的串口发送等任务），耗时短的任务才是应该采用ticker处理的
            请确保内存池可用内存充足，库将使用malloc、free函数进行任务创建和删除
            任务的名字字符串不能是动态的，需要是静态的

        一些小细节：
            使用句柄管理任务：效率相对于使用字符串管理效率更高，但是跨文件管理任务需要传递句柄，不跨文件时推荐使用句柄进行管理

        PS:仅支持最多开启255个定时任务(建议够用就好，越多的任务开辟会占用更多的内存资源)
        version:v3.0.1  2023-5-29
    */

// #include "LIB_CONFIG.h" //移植删除
#include <stdint.h> //类型报错取消注释,C51无用需自己定义好stdint的类型名
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef TICKER_LIB_USE   // LIB_CONFIG配置
#define TICKER_LIB_USE 1 // 1:启用库编译，0：不编译库
#endif

#if TICKER_LIB_USE

    typedef const void *ticker_handle_t;

    /**
     * @brief 定时事件回调函数带入的数据指针指向的结构
     */
    typedef struct
    {
        ticker_handle_t handle; // 指向定时器句柄
        void *userdata;         // 用户数据
    } ticker_event_t;

    typedef void (*ticker_cb_t)(ticker_event_t *e); // 回调函数格式类型

    /**
     * @brief 内部数据结构体
     */
    typedef struct _ticker_datas_t
    {
        ticker_cb_t tick_p;
        ticker_cb_t detach_cb;
        ticker_event_t ticker_userdata;
        uint32_t tick_t_count;
        uint32_t tick_t_count_max;
        int16_t tick_t_mode_flg;
        struct _ticker_datas_t *next;
        struct _ticker_datas_t *last;
        const char *name_str;
    } ticker_datas_t;

    typedef enum
    {
        count_down,
        count_up
    } ticker_timer_count_mode;

    /**
     * @brief 注册在延时阻塞delay时的循环回调函数，如将喂狗等操作绑定防止阻塞导致重启
     *
     * @param call_back 回调函数，格式为void function(ticker_event_t *e)，形参说明如下：
     * @param ··e 指向回调结构体，其内部成员包括：
     * @param ····handle 当前执行任务的句柄,当然，delay任务没有句柄，此参数无实际意义
     * @param ····userdata 指向注册时的用户数据，类型为void *，类型自行转换为需要使用的类型
     * @param --
     * @param userdata 指向用户数据，，未使用请填入NULL
     */
    void ticker_attach_delay_cb(ticker_cb_t call_back, void *userdata);

    /**
     * @brief 注销在延时阻塞delay时的循环回调函数
     */
    void ticker_detach_delay_cb(void);

    /**
     * @brief 注册定时多少ms执行一次任务,同一函数可进行多任务注册，可利用句柄在回调函数内进行区分
     *
     * @param million ms数，最大值2^31
     * @param --
     * @param call_back 回调函数，格式为void function(ticker_event_t *e)，形参说明如下：
     * @param ··e 指向回调结构体，其内部成员包括：
     * @param ····handle 当前执行任务的定时器句柄
     * @param ····userdata 指向注册时的用户数据，类型为void *，类型自行转换为需要使用的类型
     * @param --
     * @param count 执行多少次，0为无限次，不能为负数，为负数注册失败，最大指定32767次
     * @param --
     * @param name_str 字符串名字，可用于管理任务，不使用则填NULL以节省内存
     * @param --
     * @param userdata 指向用户数据，未使用请填入NULL
     *
     * @return ticker_handle --返回句柄，可用于判断是否注册成功和用于注销任务，为NULL则注册失败，任务数已达上限或其他错误
     */
    ticker_handle_t ticker_attch_ms(uint32_t million, ticker_cb_t call_back, int16_t count, const char *name_str, void *userdata);

    /**
     * @brief 注册定时多少s执行一次任务,同一函数可进行多任务注册，可利用句柄在回调函数内进行区分
     *
     * @param second 秒数,最大值65535
     * @param --
     * @param call_back 回调函数，格式为void function(ticker_event_t *e)，形参说明如下：
     * @param ··e 指向回调结构体，其内部成员包括：
     * @param ····handle 当前执行任务的定时器句柄
     * @param ····userdata 指向注册时的用户数据，类型为void *，类型自行转换为需要使用的类型
     * @param --
     * @param count 执行多少次，0为无限次，不能为负数，为负数注册失败，最大指定32767次
     * @param --
     * @param name_str 字符串名字，可用于管理任务，不使用则填NULL以节省内存
     * @param --
     * @param userdata 指向用户数据，未使用请填入NULL
     *
     * @return ticker_handle --返回句柄，可用于判断是否注册成功和用于注销任务，为NULL则注册失败，任务数已达上限或其他错误
     */
    ticker_handle_t ticker_attch(uint16_t second, ticker_cb_t call_back, int16_t count, const char *name_str, void *userdata);

    /**
     * @brief 检查是否存在任务，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     * @param --
     * @return uint8_t 1：存在任务，0：无此任务
     */
    uint8_t ticker_has_task(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 注册任务销毁后的回调函数
     *
     * @param handle 注册时返回的任务句柄
     * @param call_back 格式与任务的回调函数相同
     * @param userdata 指向用户数据，未使用请填入NULL
     * @return uint8_t 1成功，0失败
     */
    uint8_t ticker_attch_detach_cb(ticker_handle_t handle, ticker_cb_t call_back, void *userdata);

    /**
     * @brief 绑定定时器计数寄存器值读取函数，绑定后可获得准确的短时间delay，不绑定精度将无法保证，如心跳提供1ms，则delay(1)可能在0到1ms之间
     *
     * @param timer_get_count 回调函数
     * @param ··
     * @param count_mode 定时器计数模式
     * @param ··count_down 减计数
     * @param ··count_up 加计数
     */
    void ticker_attach_timer_count_cb(uint32_t (*timer_get_count)(), ticker_timer_count_mode count_mode);

    /**
     * @brief 通知指定任务直接执行一次，计时数不受影响，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    void ticker_run_once(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 注销定时器计数寄存器值读取函数
     */
    void ticker_detach_timer_count_cb(void);

    /**
     * @brief 注销任务，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    void ticker_detach(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 暂停任务，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    void ticker_pause(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 恢复任务，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    void ticker_resume(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 重置任务计时，参数二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    void ticker_reset(ticker_handle_t handle, const char *name_str);

    /**
     * @brief   改变任务计时，会重置已经计的时，
     *          如任务A在创建时定时500ms执行一次，现调用此接口将500改为100ms，如更改时已经计时300ms，则会清空300ms从头计时，
     *          句柄和名字二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     * @param --
     * @param ms 更改后的计时时长ms
     */
    void ticker_change_time(ticker_handle_t handle, const char *name_str, uint32_t ms);

    /**
     * @brief 更改任务次数，句柄和名字二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     * @param --
     * @param count 执行次数，0为无数次，1-32767为指定次数
     */
    void ticker_set_count(ticker_handle_t handle, const char *name_str, int16_t count);

    /**
     * @brief 获取任务剩余次数，0为无限次，-1为任务未注册，句柄和名字二选一，另一个填入NULL
     *
     * @param handle 注册时返回的任务句柄
     * @param --
     * @param name_str 任务字符串名字
     */
    int16_t ticker_get_count(ticker_handle_t handle, const char *name_str);

    /**
     * @brief 提供心态，放于配置好的的定时器中断下提供心跳节拍
     *
     * @param _ms _ms:心跳多少ms一次就写多少,如1ms一次写1，
     * 建议提供标准1ms心跳，这样delay函数才能比较准确
     */
    void ticker_heartbeat_ms(uint8_t _ms);

    /**
     * @brief 任务检测函数，放在主函数循环扫描才会执行任务!
     * 如果程序死循环阻塞时，任务需要继续执行，则可在死循环内调用此接口
     */
    void ticker_task_handler(void);

    /**
     * @brief ms延时,延时过程中会继续执行创建的任务
     *
     * @param ms 范围2^31,必须提供心跳后才能工作！
     */
    void ticker_delay(uint32_t ms);

    /**
     * @brief ms延时,延时过程中断执行创建的任务
     *
     * @param ms 范围2^31,必须提供心跳后才能工作！
     */
    void ticker_delay_without_task(uint32_t ms);

#endif
#ifdef __cplusplus
}
#endif
#endif
