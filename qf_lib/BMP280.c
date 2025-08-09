
#include "bmp280.h"

#if bmp280_lib_compile_en

#define BMP280_CHIP_ID (0xD0)              /* Chip ID Register */
#define BMP280_RST_REG (0xE0)              /* Softreset Register */
#define BMP280_STAT_REG (0xF3)             /* Status Register */
#define BMP280_CTRL_MEAS_REG (0xF4)        /* Ctrl Measure Register */
#define BMP280_CONFIG_REG (0xF5)           /* Configuration Register */
#define BMP280_PRESSURE_MSB_REG (0xF7)     /* Pressure MSB Register */
#define BMP280_PRESSURE_LSB_REG (0xF8)     /* Pressure LSB Register */
#define BMP280_PRESSURE_XLSB_REG (0xF9)    /* Pressure XLSB Register */
#define BMP280_TEMPERATURE_MSB_REG (0xFA)  /* Temperature MSB Reg */
#define BMP280_TEMPERATURE_LSB_REG (0xFB)  /* Temperature LSB Reg */
#define BMP280_TEMPERATURE_XLSB_REG (0xFC) /* Temperature XLSB Reg */

#define BMP280_SLEEP_MODE (0x00)
#define BMP280_FORCED_MODE (0x01)
#define BMP280_NORMAL_MODE (0x03)

#define BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG (0x88)
#define BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH (24)
#define BMP280_DATA_FRAME_SIZE (6)

#define BMP280_OVERSAMP_SKIPPED (0x00)
#define BMP280_OVERSAMP_1X (0x01)
#define BMP280_OVERSAMP_2X (0x02)
#define BMP280_OVERSAMP_4X (0x03)
#define BMP280_OVERSAMP_8X (0x04)
#define BMP280_OVERSAMP_16X (0x05)

#define BMP280_PRESSURE_OSR (BMP280_OVERSAMP_8X)
#define BMP280_TEMPERATURE_OSR (BMP280_OVERSAMP_16X)
#define BMP280_MODE (BMP280_PRESSURE_OSR << 2 | BMP280_TEMPERATURE_OSR << 5 | BMP280_NORMAL_MODE)

typedef struct
{
    uint16_t dig_T1; /* calibration T1 data */
    int16_t dig_T2;  /* calibration T2 data */
    int16_t dig_T3;  /* calibration T3 data */
    uint16_t dig_P1; /* calibration P1 data */
    int16_t dig_P2;  /* calibration P2 data */
    int16_t dig_P3;  /* calibration P3 data */
    int16_t dig_P4;  /* calibration P4 data */
    int16_t dig_P5;  /* calibration P5 data */
    int16_t dig_P6;  /* calibration P6 data */
    int16_t dig_P7;  /* calibration P7 data */
    int16_t dig_P8;  /* calibration P8 data */
    int16_t dig_P9;  /* calibration P9 data */
} bmp280Calib;

static bmp280Calib bmp280Cal;
static bmp280_trans_cb_t _bmp_trans_cb = {NULL, NULL};

void bmp280_attach_trans_cb(bmp280_trans_cb_t *cbs)
{
    _bmp_trans_cb = *cbs;
}

#include "trans_packer.h"
#include "trans_task.h"
#include "my_api.h"

//**************************************************************

// 初始化BMP280，根据需要请参考pdf进行修改**************
uint8_t bmp280_begin()
{
    uint8_t buffer[BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH]; //

    if (_bmp_trans_cb.bmp_read_bytes == NULL)
    {
#if BMP280_DEBUG
        BMP280_DEBUG_LOG_PRINTF("bmp280 initialization failed, iic trans cb is not attach\n");
#endif
        return 1;
    }

    _bmp_trans_cb.bmp_read_bytes(BMP280_ADDR, BMP280_CHIP_ID, buffer, 1);

    if (buffer[0] != BMP280_DEFAULT_CHIP_ID)
    {
#if BMP280_DEBUG
        BMP280_DEBUG_LOG_PRINTF("bmp280 initialization failed,please chek hardware connect and iic slave address\n");
#endif
        return 2;
    }

    _bmp_trans_cb.bmp_read_bytes(BMP280_ADDR, BMP280_TEMPERATURE_CALIB_DIG_T1_LSB_REG, buffer, BMP280_PRESSURE_TEMPERATURE_CALIB_DATA_LENGTH);

    bmp280Cal.dig_T1 = (buffer[1] << 8) | buffer[0];
    bmp280Cal.dig_T2 = (buffer[3] << 8) | buffer[2];
    bmp280Cal.dig_T3 = (buffer[5] << 8) | buffer[4];
    bmp280Cal.dig_P1 = (buffer[7] << 8) | buffer[6];
    bmp280Cal.dig_P2 = (buffer[9] << 8) | buffer[8];
    bmp280Cal.dig_P3 = (buffer[11] << 8) | buffer[10];
    bmp280Cal.dig_P4 = (buffer[13] << 8) | buffer[12];
    bmp280Cal.dig_P5 = (buffer[15] << 8) | buffer[14];
    bmp280Cal.dig_P6 = (buffer[17] << 8) | buffer[16];
    bmp280Cal.dig_P7 = (buffer[19] << 8) | buffer[18];
    bmp280Cal.dig_P8 = (buffer[21] << 8) | buffer[20];
    bmp280Cal.dig_P9 = (buffer[23] << 8) | buffer[22];

    _bmp_trans_cb.bmp_write_byte(BMP280_ADDR, BMP280_CTRL_MEAS_REG, BMP280_MODE);
    _bmp_trans_cb.bmp_write_byte(BMP280_ADDR, BMP280_CONFIG_REG, 5 << 2);

    return 0;
}
//***********************************************************************

uint8_t bmp280_read(int16_t *temp, int32_t *pressure)
{
    int32_t adc_T;
    int32_t adc_P;
    int32_t var1, var2, t_fine, p;

    if (_bmp_trans_cb.bmp_read_bytes == NULL)
        return 0;

    uint8_t tmp[3];
    _bmp_trans_cb.bmp_read_bytes(BMP280_ADDR, 0xFA, tmp, 3); // 读取温度
    adc_T = (tmp[0] << 12) | (tmp[1] << 4) | (tmp[2] >> 4);

    _bmp_trans_cb.bmp_read_bytes(BMP280_ADDR, 0xF7, tmp, 3); // 读取压强
    adc_P = (tmp[0] << 12) | (tmp[1] << 4) | (tmp[2] >> 4);

    if (adc_P == 0)
        return 0;

    // Temperature
    var1 = (((float)adc_T) / 16384.0 - ((float)bmp280Cal.dig_T1) / 1024.0) * ((float)bmp280Cal.dig_T2);
    var2 = ((((float)adc_T) / 131072.0 - ((float)bmp280Cal.dig_T1) / 8192.0) * (((float)adc_T) / 131072.0 - ((float)bmp280Cal.dig_T1) / 8192.0)) * ((float)bmp280Cal.dig_T3);
    t_fine = (uint32_t)(var1 + var2);
    if (temp != NULL)
        *temp = (var1 + var2) / 512;

    if (pressure == NULL)
        return 1;

    var1 = ((float)t_fine / 2.0) - 64000.0;
    var2 = var1 * var1 * ((float)bmp280Cal.dig_P6) / 32768.0;
    var2 = var2 + var1 * ((float)bmp280Cal.dig_P5) * 2.0;
    var2 = (var2 / 4.0) + (((float)bmp280Cal.dig_P4) * 65536.0);
    var1 = (((float)bmp280Cal.dig_P3) * var1 * var1 / 524288.0 + ((float)bmp280Cal.dig_P2) * var1) / 524288.0;
    var1 = (1.0 + var1 / 32768.0) * ((float)bmp280Cal.dig_P1);
    p = 1048576.0 - (float)adc_P;
    p = (p - (var2 / 4096.0)) * 6250.0 / var1;
    var1 = ((float)bmp280Cal.dig_P9) * p * p / 2147483648.0;
    var2 = p * ((float)bmp280Cal.dig_P8) / 32768.0;
    p = p + (var1 + var2 + ((float)bmp280Cal.dig_P7)) / 16.0;
    *pressure = p;
    return 1;
}

#endif
