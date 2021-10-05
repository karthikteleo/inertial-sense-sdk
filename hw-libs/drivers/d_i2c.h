/*
MIT LICENSE

Copyright 2014-2019 Inertial Sense, Inc. - http://inertialsense.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT, IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef D_I2C_H_
#define D_I2C_H_

#include "compiler.h"
#include "d_dma.h"
#include "conf_d_i2c.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	I2C_TXSTATUS_IDLE	= 0,
	I2C_TXSTATUS_TRANSMIT_DMA,
	I2C_TXSTATUS_TRANSMIT_DMA_WAIT_TXRDY,
	I2C_TXSTATUS_ERROR,	
};

enum 
{	
	I2C_RXSTATUS_IDLE = 0,
	I2C_RXSTATUS_READ_DMA,
	I2C_RXSTATUS_READ_DMA_WAIT_RXRDY,
	I2C_RXSTATUS_READ_PENULTIMATE,
	I2C_RXSTATUS_READ_LAST,
	I2C_RXSTATUS_ERROR,	
};

typedef struct
{
	Twihs 					*instance;
	uint32_t				instance_id;
	dma_channel_config_t 	rxdma;
	dma_channel_config_t	txdma;
	twihs_options_t 		cfg;
	uint8_t					tx_last_byte;
	uint8_t					txstatus;
	uint8_t					rxstatus;
	uint8_t					*rx_buf;
	uint8_t					rx_len;
} i2c_t;

extern i2c_t i2c0;

int i2c_get_defaults(i2c_t *init);
int i2c_init(i2c_t *init);

int i2c_transmit(i2c_t *init, uint16_t addr, uint8_t *buf, uint8_t len);
int i2c_read(i2c_t *init, uint16_t addr, uint8_t *buf, uint8_t len);

void XDMAC_i2c_Handler(void);

#ifdef __cplusplus
}
#endif

#endif  // D_I2C_H_
