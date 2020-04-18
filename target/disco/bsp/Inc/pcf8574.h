#ifndef _pcf8574_h_
#define _pcf8574_h_

#include <stdint.h>

#define PCF8574_I2C_ADDRESS   			0x40 //0x20 // 7-bit address

void pcf8574_Init(void *param);
uint16_t pcf8574_Read(uint8_t *dst, uint16_t size);
uint16_t pcf8574_Write(uint8_t *data, uint16_t size);

#endif