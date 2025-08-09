#ifndef wire_H
#define wire_H
#ifdef __cplusplus
extern "C"
{
#endif
    /*
        Chinese is encoded in Unicode. If it is garbled, please change the encoding method of the editor.
        模拟IIC主机库 By启凡科创 适用于任意C/C++平台MCU

        version:v3.0.0  2023-3-8
    */

// #include "LIB_CONFIG.h" //包含启凡科创配置文件，如不是在此环境下自行注释并包含必要的变量类型定义等头文件
#include <stdlib.h> //依赖
#include <stdint.h> //整形类型定义依赖

    /*********************************************************/
    /*                       库使能配置                     */
    /*********************************************************/

#ifndef WIRE_EN
#define WIRE_EN 1 // 如果配置文件未使能，则可更改此参数用户自定义，0禁用模块不不编译，1启用模块
#endif

#if WIRE_EN

/*********************************************************/
/*                       全局配置                     */
/*********************************************************/

/*
    IIC地址类型定义，0为8位地址类型，1为7位地址类型
        根据自己的应用场景修改地址模式  如SSD1306等OLED驱动8位地址为0x78，7位为0x3c
        如实在无法确认地址，可接好硬件后将user_set_clk配置为5以上，用wire_scan_device函数对总线上设备进行扫描
*/
#define wire_work_on_7bit_address 1

    /**
     * @brief IIC GPIO初始化函数，请根据自己的平台和接线到.c开头进行添加，对应GPIO应初始化为上拉输出模式，速度自行根据需求定义
     */
    void wire_begin(void);

/*********************************************************/
/*                       高速模式配置                     */
/*********************************************************/
/*
    IIC功能模式配置，此选项使能后将可以有一组高速IIC总线可以使用，此总线使用下列定义的IO口操作方式进行通信
*/
#define fast_mode 0 // 0禁用，1启用
#if fast_mode

// 根据自己的平台修改为对应的设置IO高低电平和读取SDA电平(返回值要为0或1)
// 要正常工作请务必外部初始化两个IO口为输出模式，使用寄存器操作方式以获得最高速率
#define SDA_CLR (P24 = 0) // SDA设置为低电平函数
#define SDA_SET (P24 = 1) // SDA设置为高电平函数
#define SCL_CLR (P25 = 0) // SCL设置为低电平函数
#define SCL_SET (P25 = 1) // SCL设置为高电平函数
#define READ_SDA (P24)    // 读取SDA电平状态函数

/*
    高速模式下时钟默认等待间隔 0-255  等待时间越短，IIC速率越快,但是过快的速度可能导致从机跟不上无法正常使用，根据实际情况调整
    仅保留发送数据功能且无判定是否成功，如要检测硬件通信是否连接正常，请用常规模式配置到相同IO上，扫描总线上的设备查看
    开启后由wire_set_fast_delay接口可进行动态配置更改
    开启等待延时,适合有低速设备时使用，如仅使用如OLED的仅需发送数据的高速场景，设置为0关闭自延时能大大提升效率提高刷新帧率
*/
#define fast_mode_delay 0

    /*************************************************************************/
    /* 下面至常规模式配置之前的所有函数均为高速模式专用函数，未启用高速模式时不可用 */
    /*************************************************************************/

    /**
     * @brief 高速固定IO模式，开始对指定地址通信，有设备应答返回1，无应答返回0
     *
     * @param slave_add 从机地址
     */
    void wire_fast_beginTransmission(uint8_t slave_add);

    /**
     * @brief 高速固定IO模式，结束IIC通信
     */
    void wire_fast_endTransmission();

    /**
     * @brief 写一个字节
     *
     * @param dat 待发送的数据
     */
    void wire_fast_write(uint8_t dat);

    /**
     * @brief 向指定从机指定寄存器写入一个字节
     *
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     * @param value 写入值
     */
    void wire_fast_write_byte(uint8_t slave_add, uint8_t reg_add, uint8_t value);

    /**
     * @brief 向指定从机指定寄存器起始连续写入指向的内容，长度len字节
     *
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     * @param dat 指向待写入的数据
     * @param len 写入字节数
     */
    void wire_fast_write_bytes(uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint16_t len);

#if fast_mode_delay
    /**
     * @brief 高速固定IO模式，设置通信时钟延迟，值越大总线速率越慢，请根据设备速率和平台自行测试延时大小多少通信正常
     *
     * @param wait (1-255)，默认10
     */
    void wire_fast_set_delay(uint8_t wait);
#endif

#endif // if fast mode

    /*********************************************************/
    /*                       常规模式配置                     */
    /*********************************************************/

#define wire_slow_mode 1 // 0关闭，1启用
#if wire_slow_mode

#define recv_buffer_size 16 // IIC读取数据环形缓冲区大小（字节）1-255

/*
    常规模式下时钟默认等待间隔 0-255  等待时间越短，IIC速率越快,但是过快的速度可能导致从机跟不上无法正常使用，根据实际情况调整
    由wire_set_delay接口可进行动态配置更改，默认5
*/
#define slow_mode_delay 1

/*
    等待设备响应的默认时间上限，超过此时间则判定为设备无响应,默认512
*/
#define slow_mode_timeout_tic 256

    typedef struct
    {
        uint8_t sda;                                       // sda io号
        uint8_t scl;                                       // scl io号
        void (*gpio_set_level)(uint8_t io, uint8_t level); // 设置管脚高低电平函数,io:io号，level：电平0/1
        uint8_t (*gpio_get_level)(uint8_t io);             // 读取管脚高低电平函数，io:io号，返回高低电平0/1
    } wire_config_t;                                       // IIC配置参数结构体

    typedef struct
    {
        wire_config_t config;
        uint8_t buffer[recv_buffer_size];
        uint8_t buffer_w_p;
        uint8_t buffer_r_p;
        uint8_t buffer_r_c;
        uint8_t delay;
        uint16_t timeout;
    } wire_para_t;

    typedef wire_para_t *wire_handle_t; // 句柄类型定义

    /**
     * @brief 创建一个IIC对象
     *
     * @param para
     * @param ··sda 对应IO号
     * @param ··scl 对应IO号
     * @param ··sda 对应IO号
     * @param ··void (*gpio_set_level)(uint8_t io, uint8_t level) 设置管脚电平的回调函数
     * @param ··uint8_t (*gpio_get_level)(uint8_t io) 读取管脚电平的回调函数
     * @param ··
     * @return wire_handle_t 对象句柄,返回NULL则创建失败，内存不足
     */
    wire_handle_t wire_creat(wire_config_t *para);

    /**
     * @brief 删除IIC对象
     *
     * @param handle 创建对象时返回的对象句柄
     */
    void wire_delete(wire_handle_t handle);

    /**
     * @brief 设置IIC通信等待
     *
     * @param handle 创建对象时返回的对象句柄
     * @param wait 0-255，值越小速率越高
     */
    void wire_set_delay(wire_handle_t handle, uint8_t wait);

    /**
     * @brief 设置IIC通信时间溢出上限
     *
     * @param handle 创建对象时返回的对象句柄
     * @param tic 100-65535
     */
    void wire_set_timeout(wire_handle_t handle, uint16_t tic);

    /**
     * @brief 开始IIC通信
     *
     * @param handle IIC对象
     * @param slave_add 通信对象从机地址
     *
     * @return uint8_t 1成功，0失败
     */
    uint8_t wire_beginTransmission(wire_handle_t handle, uint8_t slave_add);

    /**
     * @brief 结束IIC通信
     *
     * @param handle IIC对象
     */
    void wire_endTransmission(wire_handle_t handle);

    /**
     * @brief IIC写一个字节
     *
     * @param handle IIC对象
     * @param dat 数据
     * @return uint8_t  1成功，0失败
     */
    uint8_t wire_write(wire_handle_t handle, uint8_t dat);

    /**
     * @brief 从指定地址从机读出size字节到内部缓存，从内部缓存取出数据用wire_read接口
     *
     * @param handle IIC对象
     * @param slave_add 从机地址，跟写地址相同，无需最低位为1
     * @param size 字节数
     *
     * @return uint8_t 1成功，0失败
     */
    uint8_t wire_requestFrom(wire_handle_t handle, uint8_t slave_add, uint8_t size);

    /**
     * @brief 从内部环形缓冲区读取一个字节，读出的字节会自动销毁
     *
     * @param handle IIC对象
     * @return uint8_t
     */
    uint8_t wire_read(wire_handle_t handle);

    /**
     * @brief 返回FIFO缓冲区还有多少个数据
     *
     * @return uint8_t 还未取出的缓冲字节数,无数据返回0
     */
    uint8_t wire_available(wire_handle_t handle);

    /**
     * @brief 一站式指令，从指定从机指定寄存器地址读出一个字节
     *
     * @param handle IIC对象
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     *
     * @return uint8_t
     */
    uint8_t wire_read_byte(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add);

    /**
     * @brief 一站式指令，从指定从机指定寄存器取出n个字节写入到dat指向的缓存区域
     *
     * @param handle IIC对象
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     * @param dat 指向用于接收结果的缓存
     * @param len 读取长度
     *
     * @return 成功返回1，失败返回0
     */
    uint8_t wire_read_bytes(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint8_t len);

    /**
     * @brief 一站式指令，向指定从机指定寄存器写入一个字节
     *
     * @param handle IIC对象
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     * @param value 写入值
     *
     * @return 成功返回1，失败返回0
     */
    uint8_t wire_write_byte(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t value);

    /**
     * @brief 一站式指令，向指定从机指定寄存器起始连续写入指向的内容，长度len字节
     *
     * @param handle IIC对象
     * @param slave_add 从机地址
     * @param reg_add 寄存器地址
     * @param dat 指向待写入的数据
     * @param len 写入字节数
     *
     * @return 成功返回1，失败返回0
     */
    uint8_t wire_write_bytes(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint16_t len);

    /**
     * @brief 扫描IIC总线上的设备并返回数量、地址，
     * 返回结果：
                result[0]:总线上扫描到的设备数量n
                result[1] -- result[n]:总线上设备地址从小到大排列
     *
     * @param handle IIC对象
     * @param result 用于接收返回结果的数组，长度必须大于等于设备数+1，否则将造成宕机
     */
    void wire_scan_device(wire_handle_t handle, uint8_t *result);

#endif // if slow mode
#endif // wire lib en

#ifdef __cplusplus
}
#endif

#endif
