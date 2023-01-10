
#include "pcf8574.h"
#include "board.h"

static i2cbus_t *i2cbus;

input_drv_t input_drv_pcf8574 = {
    pcf8574_Init,
    pcf8574_Read,
    pcf8574_Write,
};

void pcf8574_Init(void *param){
    i2cbus = (i2cbus_t*)param;
    // I/Os should be high before being used as inputs.
    uint8_t data = 0xFF;    
    i2cbus->write(PCF8574_I2C_ADDRESS, &data, 1);
}

uint16_t pcf8574_Read(uint8_t *dst, uint16_t size){
    return i2cbus->read(PCF8574_I2C_ADDRESS, dst, size) == 0;
}

uint16_t pcf8574_Write(uint8_t *data, uint16_t size){
    return i2cbus->write(PCF8574_I2C_ADDRESS, data, size) == 0;
}