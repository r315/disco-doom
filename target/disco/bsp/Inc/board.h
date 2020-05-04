#ifndef _board_h_
#define _board_h_


#include "main.h"
#include "input.h"
#include "lis302.h"
#include "pcf8574.h"

typedef struct _i2cbus_t{
    uint32_t   (*read)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
    uint32_t   (*write)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
}i2cbus_t;

#define IO_EXPANDER
//#define ACCELEROMETER

#define HW_DBG_PIN_HIGH		HAL_GPIO_WritePin(DBG_PIN_PORT, DBG_PIN, GPIO_PIN_SET)
#define HW_DBG_PIN_LOW		HAL_GPIO_WritePin(DBG_PIN_PORT, DBG_PIN, GPIO_PIN_RESET)
#define HW_DBG_PIN_TOGGLE	HAL_GPIO_TogglePin(DBG_PIN_PORT, DBG_PIN)

void OnError_Handler(uint32_t condition);

void TS_IO_Init(void);
uint8_t TS_IO_Read(uint8_t Addr, uint8_t Reg);

uint32_t INPUT_I2C_Read(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
uint32_t INPUT_I2C_Write(uint8_t Addr, uint8_t *Buffer, uint16_t Length);

extern i2cbus_t ext_i2cbus;
#ifdef ACCELEROMETER
#define input_drv input_drv_lis302
#elif defined (IO_EXPANDER)
#define input_drv input_drv_pcf8574
#endif /* ACCELEROMETER */

#endif