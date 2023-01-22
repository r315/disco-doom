#ifndef _serial_h_
#define _serial_h_

#include <stdint.h>

uint8_t SERIAL_GetChar(char *c);
void SERIAL_Init(void);
void SERIAL_DeInit(void);
uint32_t SERIAL_Write(char *data, uint32_t len);
#endif