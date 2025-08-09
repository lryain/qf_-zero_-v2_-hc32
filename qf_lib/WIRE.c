#include <WIRE.h>
#include "devices.h"
#if WIRE_EN

void wire_begin()
{
	// 自行添加对应GPIO初始化为上拉输出模式
}

#if fast_mode

#if fast_mode_delay
static uint8_t fast_delay = fast_mode_delay;
void wire_fast_set_delay(uint8_t wait)
{
	fast_delay = wait;
}
#endif

static void fast_iic_delay()
{
#if fast_mode_delay
	uint8_t i = fast_delay;
	while (--i)
		;
#endif
}

void fast_RecvACK()
{
	SCL_SET;
	fast_iic_delay();
	SCL_CLR;
}

static void fast_I2C_Start()
{
	SDA_SET;
	SCL_SET;
	fast_iic_delay();
	SDA_CLR;
	fast_iic_delay();
	SCL_CLR;
}

void wire_fast_write(uint8_t dat)
{
	static uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if ((dat & 0x80))
			SDA_SET;
		else
			SDA_CLR;
		dat <<= 1;
		dat++;
		SCL_SET;
		fast_iic_delay();
		SCL_CLR;
	}
	fast_RecvACK();
}

void wire_fast_endTransmission()
{
	SDA_CLR;
	fast_iic_delay();
	SCL_SET;
	fast_iic_delay();
	SDA_SET;
	fast_iic_delay();
}

void wire_fast_beginTransmission(uint8_t slave_add)
{
	fast_I2C_Start();
#if wire_work_on_7bit_address
	slave_add <<= 1;
#endif
	wire_fast_write(slave_add);
}

void wire_fast_write_byte(uint8_t slave_add, uint8_t reg_add, uint8_t value)
{
	wire_fast_beginTransmission(slave_add);
	wire_fast_write(reg_add);
	wire_fast_write(value);
	wire_fast_endTransmission();
}

void wire_fast_write_bytes(uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint16_t len)
{
	wire_fast_beginTransmission(slave_add);
	wire_fast_write(reg_add);
	while (len--)
	{
		wire_fast_write(*dat++);
	}
	wire_fast_endTransmission();
}

#endif // if fast_mode

#if wire_slow_mode

void wire_set_delay(wire_handle_t handle, uint8_t wait)
{
	handle->delay = wait;
}

void wire_set_timeout(wire_handle_t handle, uint16_t tic)
{
	handle->timeout = tic;
}

static void slow_iic_delay(wire_handle_t handle)
{
	uint8_t i = handle->delay;
	while (i--)
		;
}

static uint8_t slow_RecvACK(wire_handle_t handle)
{
	uint16_t i;
	handle->config.gpio_set_level(handle->config.sda, 1);
	i = 0;
	handle->config.gpio_set_level(handle->config.scl, 1);
	slow_iic_delay(handle);
	gpio_set_mode(sda_io, gpio_mode_input);
	while (handle->config.gpio_get_level(handle->config.sda))
	{
		i++;
		if (i == handle->timeout)
		{
			handle->config.gpio_set_level(handle->config.scl, 0);
			gpio_set_mode(sda_io, gpio_mode_output_od);
			return 0;
		}
	}
	handle->config.gpio_set_level(handle->config.scl, 0);
	gpio_set_mode(sda_io, gpio_mode_output_od);
	return 1;
}

static void slow_SendAck(wire_handle_t handle)
{
	handle->config.gpio_set_level(handle->config.sda, 0);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 1);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 0);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.sda, 1);
	slow_iic_delay(handle);
}

static void slow_NAck(wire_handle_t handle)
{
	handle->config.gpio_set_level(handle->config.scl, 0);
	handle->config.gpio_set_level(handle->config.sda, 1);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 1);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 0);
}

static void slow_I2C_Start(wire_handle_t handle)
{
	handle->config.gpio_set_level(handle->config.sda, 1);
	handle->config.gpio_set_level(handle->config.scl, 1);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.sda, 0);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 0);
}

uint8_t wire_write(wire_handle_t handle, uint8_t dat)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if ((dat & 0x80) == 0x80)
			handle->config.gpio_set_level(handle->config.sda, 1);
		else
			handle->config.gpio_set_level(handle->config.sda, 0);
		dat <<= 1;
		slow_iic_delay(handle);
		handle->config.gpio_set_level(handle->config.scl, 1);
		slow_iic_delay(handle);
		handle->config.gpio_set_level(handle->config.scl, 0);
		slow_iic_delay(handle);
	}
	return slow_RecvACK(handle);
}

static uint8_t slow_read_byte(wire_handle_t handle)
{
	uint8_t i;
	uint8_t dat = 0;
	gpio_set_mode(sda_io, gpio_mode_input);
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		slow_iic_delay(handle);
		handle->config.gpio_set_level(handle->config.scl, 1);
		slow_iic_delay(handle);
		dat |= handle->config.gpio_get_level(handle->config.sda);
		handle->config.gpio_set_level(handle->config.scl, 0);
	}
	gpio_set_mode(sda_io, gpio_mode_output_od);
	return dat;
}

wire_handle_t wire_creat(wire_config_t *para)
{
	wire_handle_t handle = (wire_handle_t)malloc(sizeof(wire_para_t));
	if (handle == NULL)
		return NULL;
	handle->config = *para;
	handle->buffer_r_c = 0;
	handle->buffer_r_p = 0;
	handle->buffer_w_p = 0;
	handle->delay = slow_mode_delay;
	handle->timeout = slow_mode_timeout_tic;
	return handle;
}

void wire_delete(wire_handle_t handle)
{
	if (handle != NULL)
		free(handle);
	handle = NULL;
}

uint8_t wire_beginTransmission(wire_handle_t handle, uint8_t slave_add)
{
	slow_I2C_Start(handle);
#if wire_work_on_7bit_address
	slave_add <<= 1;
#endif
	return wire_write(handle, slave_add);
}

void wire_endTransmission(wire_handle_t handle)
{
	handle->config.gpio_set_level(handle->config.sda, 0);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.scl, 1);
	slow_iic_delay(handle);
	handle->config.gpio_set_level(handle->config.sda, 1);
	slow_iic_delay(handle);
}

uint8_t wire_requestFrom(wire_handle_t handle, uint8_t slave_add, uint8_t size)
{
	uint8_t i;
#if wire_work_on_7bit_address
	slave_add <<= 1;
#endif
	slave_add |= 1;

	slow_I2C_Start(handle);

	if (wire_write(handle, slave_add) == 0)
	{
		wire_endTransmission(handle);
		return 0;
	}

	for (i = 0; i < size; i++)
	{
		handle->buffer[handle->buffer_w_p++] = slow_read_byte(handle);
		if (i < (size - 1))
			slow_SendAck(handle);
		else
			slow_NAck(handle);
		if (handle->buffer_w_p == recv_buffer_size)
			handle->buffer_w_p = 0;
		handle->buffer_r_c++;
		if (handle->buffer_r_c > recv_buffer_size)
		{
			handle->buffer_r_c = recv_buffer_size;
			handle->buffer_r_p++;
			if (handle->buffer_r_p == recv_buffer_size)
				handle->buffer_r_p = 0;
		}
	}
	wire_endTransmission(handle);
	return 1;
}

uint8_t wire_available(wire_handle_t handle)
{
	return handle->buffer_r_c;
}

uint8_t wire_read(wire_handle_t handle)
{
	uint8_t i;
	if (handle->buffer_r_c == 0)
		return 255;
	i = handle->buffer[handle->buffer_r_p++];
	if (handle->buffer_r_p == recv_buffer_size)
		handle->buffer_r_p = 0;
	handle->buffer_r_c--;
	return i;
}

uint8_t wire_read_byte(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add)
{
	wire_beginTransmission(handle, slave_add);
	wire_write(handle, reg_add);
	wire_requestFrom(handle, slave_add, 1);
	return wire_read(handle);
}

uint8_t wire_read_bytes(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint8_t len)
{
	uint8_t i;
	if (wire_beginTransmission(handle, slave_add) == 0)
	{
		wire_endTransmission(handle);
		return 0;
	}
	wire_write(handle, reg_add);

#if wire_work_on_7bit_address
	slave_add <<= 1;
#endif
	slave_add |= 1;

	slow_I2C_Start(handle);
	wire_write(handle, slave_add);

	// wire_beginTransmission(handle, slave_add);
	for (i = 0; i < len; i++)
	{
		*dat++ = slow_read_byte(handle);
		if (i < (len - 1))
			slow_SendAck(handle);
		else
			slow_NAck(handle);
	}
	wire_endTransmission(handle);
	return 1;
}

uint8_t wire_write_byte(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t value)
{
	if (wire_beginTransmission(handle, slave_add) == 0)
		return 0;
	wire_write(handle, reg_add);
	wire_write(handle, value);
	wire_endTransmission(handle);
	return 1;
}

uint8_t wire_write_bytes(wire_handle_t handle, uint8_t slave_add, uint8_t reg_add, uint8_t *dat, uint16_t len)
{
	if (wire_beginTransmission(handle, slave_add) == 0)
		return 0;
	wire_write(handle, reg_add);
	while (len--)
	{
		wire_write(handle, *dat++);
	}
	wire_endTransmission(handle);
	return 1;
}

void wire_scan_device(wire_handle_t handle, uint8_t *result)
{
	uint8_t i;
	uint8_t *tmp;
	uint8_t addr_tmp;
	*result = 0;
	tmp = result + 1;
	for (i = 0; i < 128; i++)
	{
		addr_tmp = i;
#if (wire_work_on_7bit_address == 0)
		addr_tmp <<= 1;
#endif
		if (wire_beginTransmission(handle, addr_tmp))
		{
			*result += 1;
			*tmp++ = addr_tmp;
		}
		wire_endTransmission(handle);
	}
	tmp = NULL;
}

#endif // if wire slow mode

#endif // wire lib en
