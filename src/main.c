#include "devices.h"
#include "tasks.h"
#include "BMP280.h"
#include "my_api.h"

int main()
{
    // gpio_set_mode(esp_sta_io, gpio_mode_output_od);
    // gpio_set_level(esp_sta_io, 0); // 下拉初始化状态，主处理器等待

    devices_init(); // 初始化设备
    tasks_init();   // 初始化任务

    // gpio_set_mode(esp_sta_io, gpio_mode_input_pullup); // 初始化完成，释放状态，由主处理器管控状态

    // bmp280_trans_cb_t cb;
    // cb.bmp_read_bytes = iic_read_bytes;
    // cb.bmp_write_byte = iic_write_byte;
    // bmp280_attach_trans_cb(&cb);

    // uint8_t code = bmp280_begin();

    // if (code == 0)
    // {
    //     trans_packer_send_pack(esp_trans_get_handle(), "log", "bmp280 init ok", pack_send_log);
    // }
    // else
    //     trans_packer_send_pack_fmt(esp_trans_get_handle(), "log", "bmp280 init err:%d", code);

    for (;;)
    {
        // ticker_delay(1000);
        // int16_t temp;
        // int32_t pre;
        // bmp280_read(&temp, &pre);
        // trans_packer_send_pack_fmt(esp_trans_get_handle(), "log", "temp:%d,pre:%d", 30, 40);
        task_loop_handler(); // 任务循环处理
    }
}
