#ifndef _board_h_
#define _board_h_


#include "main.h"
#include "input.h"

typedef struct _i2cbus_t{
    uint32_t   (*read)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
    uint32_t   (*write)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
}i2cbus_t;

#define IO_EXPANDER
//#define ACCELEROMETER

#ifdef ACCELEROMETER
#include "lis302.h"
#elif defined (IO_EXPANDER)
#include "pcf8574.h"
#endif

typedef struct {
	uint32_t scanned;
	uint32_t last;
	uint32_t counter;
	uint32_t events;
    uint32_t htime;
}BUTTON_Controller;

enum Benvent{
    BUTTON_EMPTY = 0,
    BUTTON_PRESSED,
    BUTTON_RELEASED	
};


uint8_t vc_getCharNonBlocking(char *c);
void OnError_Handler(uint32_t condition);

void TS_IO_Init(void);
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg);

uint32_t INPUT_I2C_Read(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
uint32_t INPUT_I2C_Write(uint8_t Addr, uint8_t *Buffer, uint16_t Length);


static i2cbus_t ext_i2cbus = {
    .write = INPUT_I2C_Write,
    .read = INPUT_I2C_Read
};


#ifdef ACCELEROMETER

static input_drv_t input_drv = {
    .init = lis302_Init,
    .read = lis302_read,
    .write = lis302_write,
    .data = lis302_ReadID
};


#elif defined (IO_EXPANDER)

static input_drv_t input_drv = {
    pcf8574_Init,
    pcf8574_Read,
    pcf8574_Write,
};

#endif

#endif