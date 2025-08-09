#include "bat_task.h"
#include "my_api.h"
#include "devices.h"
#include "trans_packer.h"
#include "trans_task.h"

// static void tic_cb(ticker_event_t *e)
// {
//     static cw2015_bat_info_t bat;

//     trans_packer_handle_t *handle = esp_trans_get_handle();

//     cw2015_get_info(&bat);
//     // 不是低电量休眠
//     uint8_t tmp[2] = {bat.voltage >> 8, bat.voltage & 0xff};
//     trans_packer_send_pack(handle, "vol", tmp, 2); // 上报数据
//     trans_packer_send_pack(handle, "soc", &bat.soc, 1);
//     tmp[0] = bat.rtt >> 8;
//     tmp[1] = bat.rtt & 0xff;
//     trans_packer_send_pack(handle, "rtt", tmp, 2);
//     trans_packer_send_pack(handle, "low_soc", &bat.low_soc, 1);
// }

// void bat_task_init()
// {
//     cw2015_trans_cb_t cbs;
//     cbs.iic_read_bytes = iic_read_bytes;
//     cbs.iic_write_bytes = iic_write_bytes;
//     cw2015_attach_trans_cb(&cbs);
//     cw2015_init(cw_auto_init);

//     //ticker_attch(1, tic_cb, 0, "bat", NULL); // 1s上报一次数据
// }
