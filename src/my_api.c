#include "my_api.h"
#include "trans_packer.h"
// #include "trans_task.h"

static wire_handle_t iic;

wire_handle_t iic_get_handle(void)
{
    return iic;
}

void gpio_set_mode(uint8_t pin, gpio_mode_t mode)
{
    uint8_t buf[7];
    buf[0] = pin / 10;
    buf[1] = pin % 10;
    switch (mode)
    {
    case gpio_mode_input:
        buf[2] = GpioDirIn;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        break;
    case gpio_mode_input_pulldown:
        buf[2] = GpioDirIn;
        buf[3] = 0;
        buf[4] = 1;
        buf[5] = 0;
        buf[6] = 0;
        break;
    case gpio_mode_input_pullup:
        buf[2] = GpioDirIn;
        buf[3] = 1;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        break;
    case gpio_mode_output_pushpull:
        buf[2] = GpioDirOut;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 0;
        buf[6] = 0;
        break;
    case gpio_mode_output_od:
        buf[2] = GpioDirOut;
        buf[3] = 0;
        buf[4] = 0;
        buf[5] = 1;
        buf[6] = 0;
        break;
    case gpio_mode_output_od_pullup:
        buf[2] = GpioDirOut;
        buf[3] = 1;
        buf[4] = 0;
        buf[5] = 1;
        buf[6] = 0;
        break;
    default:
        break;
    }
    Gpio_InitIOExt(buf[0], buf[1], (en_gpio_dir_t)buf[2], buf[3], buf[4], buf[5], buf[6]);
}

void gpio_set_level(uint8_t pin, boolean_t level)
{
    Gpio_SetIO(pin / 10, pin % 10, level);
}
boolean_t gpio_get_level(uint8_t pin)
{
    return Gpio_GetIO(pin / 10, pin % 10);
}

void iic_write_bytes(uint8_t add, uint8_t reg, uint8_t *dat, size_t size)
{
    wire_write_bytes(iic, add, reg, dat, size);
}

void iic_read_bytes(uint8_t add, uint8_t reg, uint8_t *dat, size_t size)
{
    wire_read_bytes(iic, add, reg, dat, size);
}

void iic_write_byte(uint8_t add, uint8_t reg, uint8_t dat)
{
    wire_write_byte(iic, add, reg, dat);
}

uint8_t iic_read_byte(uint8_t add, uint8_t reg)
{
    return wire_read_byte(iic, add, reg);
}

void iic_init()
{
    wire_config_t cfg;
    cfg.gpio_get_level = gpio_get_level;
    cfg.gpio_set_level = gpio_set_level;
    cfg.scl = scl_io;
    cfg.sda = sda_io;
    iic = wire_creat(&cfg);
    gpio_set_mode(sda_io, gpio_mode_output_od);
    gpio_set_mode(scl_io, gpio_mode_output_od);
    gpio_set_level(sda_io, 1);
    gpio_set_level(scl_io, 1);
}

void gpio_set_irq(uint8_t io, en_gpio_irqtype_t mode, boolean_t en)
{
    uint8_t port, pin;
    uint32_t irq;
    port = io / 10;
    pin = io % 10;
    Gpio_ClearIrq(port, pin);
    Gpio_EnableIrq(port, pin, mode);
    switch (port)
    {
    case 0:
        irq = PORT0_IRQn;
        break;
    case 1:
        irq = PORT1_IRQn;
        break;
    case 2:
        irq = PORT2_IRQn;
        break;
    case 3:
        irq = PORT3_IRQn;
        break;
    default:
        break;
    }
    EnableNvic(irq, DDL_IRQ_LEVEL_DEFAULT, en);
}
