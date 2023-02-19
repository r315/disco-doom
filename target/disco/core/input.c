
#include <stdio.h>
#include <stdint.h>
#include "input.h"
#include "board.h"

#define BTN_MAX             8
#define BTN_UP_MASK         (1 << 0)
#define BTN_DOWN_MASK       (1 << 1)
#define BTN_LEFT_MASK       (1 << 2)
#define BTN_RIGHT_MASK      (1 << 3)
#define BTN_MID_MASK        (1 << 4)
#define BTN1_MASK           (1 << 5)
#define BTN2_MASK           (1 << 6)
#define BTN3_MASK           (1 << 7)

typedef struct{
    uint8_t key;
    uint8_t state;
    uint8_t laststate;
    uint8_t state_changed;
}btnstate_t;

//up, down, left, righ, mid, btn1, btn2, btn3
const uint8_t btn_values[BTN_MAX] = {173, 175, 172, 174, 13, 'e', 27, ' '};
static btnstate_t inbtn[BTN_MAX];
static uint8_t btn_nevents;

void INPUT_Init(void)
{
    // User push button
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);

#if ACCELEROMETER || IO_EXPANDER
    //I2C bus
    INPUT_I2C_Init();

    input_drv.init(&ext_i2cbus);
#endif

    for (uint8_t i = 0; i < BTN_MAX; i++)
    {
        inbtn[i].key = btn_values[i];
        inbtn[i].state = BTN_EMPTY;
        inbtn[i].laststate = BTN_EMPTY;
    }
    

#ifdef ACCELEROMETER
    uint16_t (*func)(void);
    func  = (uint16_t(*)(void))input_drv.data;

    uint16_t id = func();

    if (id == LIS302_ID)
    {
        printf("LIS302 linear accelerometer detected!\n");
    }    
#endif
}

uint32_t INPUT_Read(uint32_t *dst, uint32_t size)
{
    uint32_t read = 0;    
#ifdef IO_EXPANDER    
    uint8_t raw_data;
   
    input_drv.read((uint8_t*)&raw_data, 1);

    if(raw_data != 255){
        *dst = ~raw_data;
        //printf("BTN: %x\n", *dst);
        read = 1;
    }else{
        *dst = 0;
    }

    return read;    
#elif ACCELEROMETER
    int8_t raw_data[6], x, y;
    uint8_t buttons = 0;

    //if(vc_getCharNonBlocking((char*)&key)){
    //    printf("new key %c\n", key);
    //}

    if (BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET)
    {
        buttons |= BTN3_MASK;
        read = 1; 
    }

    if(input_drv.read((uint8_t*)raw_data, sizeof(raw_data)) == 0){        
        *dst = buttons;
        return read;
    }

    y = raw_data[0];
    x = raw_data[2];
    
    if(y > 10){
        buttons |= BTN_UP_MASK;
        read = 1;
    }else if(y < -10){
        buttons |= BTN_DOWN_MASK;
        read = 1;
    }

    if(x > 10){
        buttons |= BTN_RIGHT_MASK;
        read = 1;
    }else if(x < -10){
        buttons |= BTN_LEFT_MASK;
        read = 1;
    }
    *dst = buttons;   
    return read;
#else
    if (BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_RESET)
    {
        *dst = BTN3_MASK;
        read = 1; 
    }
    return read;
#endif
}

/**
 * Button handling
 *
 */

static uint8_t btnState(btnstate_t *btn, uint8_t pressed){

    btn->laststate = btn->state;

    switch(btn->state){

        case BTN_EMPTY:            
            if (pressed){                
                btn->state = BTN_PRESSED;
            }
            break;

        case BTN_PRESSED:
            if (pressed){
                btn->state = BTN_HOLD;
            }else{
                btn->state = BTN_RELEASED;
            }
            break;

        case BTN_HOLD:
            if (!pressed){
                btn->state = BTN_RELEASED;
            }
            break;

        case BTN_RELEASED:
            if (pressed){
                btn->state = BTN_PRESSED;
            }else{
                btn->state = BTN_EMPTY;
            }
            break;
        }

    btn->state_changed = btn->laststate != btn->state;
   /*  if(btn->state_changed){
        char *msg = "";
        switch(btn->state){
            case BTN_PRESSED: msg = "pressed"; break;
            case BTN_HOLD: msg = "hold"; break;
            case BTN_RELEASED: msg = "released"; break;            
        }
        printf("%d, %s\n", btn->key, msg);
    } */
    return btn->state_changed;
}

int BUTTON_Read(void)
{
    uint32_t scanned;
    btn_nevents = 0;
    
    if(INPUT_Read(&scanned, 1)){    
        for (uint8_t i = 0; i < BTN_MAX; i++, scanned >>= 1)
        {
            btn_nevents += btnState(inbtn + i, scanned & 1);        
        }
    }
    
    return btn_nevents;
}

int BUTTON_GetEvent(int *key){
    for(uint8_t idx = 0; idx < BTN_MAX; idx++){
        if(inbtn[idx].state_changed){
            inbtn[idx].state_changed = 0;
            *key = inbtn[idx].key;
            return inbtn[idx].state;
        }
    }
    return 0;
}