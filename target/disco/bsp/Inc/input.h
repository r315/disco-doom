#ifndef _input_h_
#define _input_h_

#include <stdint.h>

typedef struct {  
  void       (*init)(void *param);
  uint16_t   (*read)(uint8_t *dst, uint16_t size);
  uint16_t   (*write)(uint8_t *src, uint16_t size);
  void        *data;
}input_drv_t;

enum Benvent{
    BTN_EMPTY = 0,
    BTN_PRESSED,
    BTN_HOLD,
    BTN_RELEASED	
};

int BUTTON_Read(void);
int BUTTON_GetEvent(int *key);

void INPUT_Init(void);
uint32_t INPUT_Read(uint32_t *dst, uint32_t size);

void INPUT_I2C_Init(void);
void INPUT_I2C_Error(uint8_t Addr);

#endif