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
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
	bool			master;
	uint32_t 		speed;
} i2c_cfg_t;

typedef struct
{
	Twihs 			*instance;
	Xdmac 			*rxdma;
	Xdmac		 	*txdma;
	i2c_cfg_t 		cfg;
} i2c_t;

int i2c_get_defaults(i2c_t *init);
int i2c_init(i2c_t *init);

int i2c_transmit(i2c_t *init, uint16_t address, uint8_t *buf, uint8_t length, bool blocking);
int i2c_read(i2c_t *init, uint16_t address, uint8_t *buf, uint8_t length, bool blocking);

#ifdef __cplusplus
}
#endif

#endif  // D_I2C_H_
