
#include "cw2015.h"

/*CW2015 寄存器定义*/
#define REG_VERSION 0x0
#define REG_VCELL 0x2
#define REG_SOC 0x4
#define REG_RRT_ALERT 0x6
#define REG_CONFIG 0x8
#define REG_MODE 0xA
#define REG_BATINFO 0x10

enum
{
    MODE_SLEEP,
    MODE_NORMAL,
    MODE_QUICK_START,
    MODE_RESTART,
    CONFIG_UPDATE_FLG,
    ATHD_DEFAULT
};

static uint8_t reg_buffer[] = {(0x3 << 6), (0x0 << 6), (0x3 << 4), (0xf << 0), (0x1 << 1), (3 << 3)};
static cw2015_trans_cb_t _cw_trans_cb;

void cw2015_attach_trans_cb(cw2015_trans_cb_t *cbs)
{
    _cw_trans_cb = *cbs;
}

/**
 * 更新IC内的电池profile信息，当IC VDD掉电后再上电会执行
 *
 * @param none
 * @return 1：i2c读写错 2：芯片处于sleep模式 3：写入的profile与读出的不一致
 */
static void cw_update_config_info(void)
{
    /* make sure no in sleep mode */
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_MODE, &reg_buffer[MODE_NORMAL], 1);

    /* update new battery info */
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_BATINFO, (uint8_t *)cw_bat_config_info, sizeof(cw_bat_config_info));

    uint8_t tmp = reg_buffer[CONFIG_UPDATE_FLG] | reg_buffer[ATHD_DEFAULT];

    /* set cw2015/cw2013 to use new battery info */
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_CONFIG, &tmp, 1);

    // restart
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_MODE, &reg_buffer[MODE_RESTART], 1);

    // work
    tmp = reg_buffer[MODE_NORMAL] | reg_buffer[MODE_QUICK_START];
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_MODE, &tmp, 1);
}

void cw2015_set_athd(uint8_t new_athd)
{
    new_athd <<= 3;
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_CONFIG, &new_athd, 1);
}

void cw2015_release_alrt_pin(void)
{
    uint8_t reg_val;
    _cw_trans_cb.iic_read_bytes(CW2015_ADDR, REG_RRT_ALERT, &reg_val, 1);
    reg_val &= 0x7f;
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_RRT_ALERT, &reg_val, 1);
}

void cw2015_init(cw2015_init_mode_t force_reset)
{
    // /* wake up cw2015/13 from sleep mode */
    _cw_trans_cb.iic_write_bytes(CW2015_ADDR, REG_MODE, &reg_buffer[MODE_NORMAL], 1);

    if (force_reset == cw_force_init) // force reset info
    {
#if CW_DEBUG
        printf("cw2015 info well reset\n");
#endif
        cw_update_config_info(); // reset info

        return;
    }

    uint8_t buffer[sizeof(cw_bat_config_info)];
    _cw_trans_cb.iic_read_bytes(CW2015_ADDR, REG_BATINFO, buffer, sizeof(cw_bat_config_info)); // read all info out

    if (memcmp(buffer, cw_bat_config_info, sizeof(cw_bat_config_info)) != 0) // chek info is err
    {
#if CW_DEBUG
        printf("cw2015 info error,well reset\n");
#endif
        cw_update_config_info(); // reset info
    }
}

void cw2015_get_info(cw2015_bat_info_t *info)
{
    uint8_t buffer[7];
    _cw_trans_cb.iic_read_bytes(CW2015_ADDR, REG_VCELL, buffer, sizeof(buffer));
    info->voltage = ((buffer[0] << 8) | buffer[1]) * 305 / 1000;
    info->soc = buffer[2];
    info->rtt = ((buffer[4] & 0x1f) << 8) | buffer[5];
    info->low_soc = buffer[6] >> 3;
}
