
#include <stdio.h>
#include "input.h"
#include "board.h"

#define PB_VALUE ' '

extern input_drv_t input_drv;
extern i2cbus_t ext_i2cbus;

void INPUT_Init(void)
{
    // User push button
    BSP_PB_Init(BUTTON_WAKEUP, BUTTON_MODE_GPIO);

    //I2C bus
    INPUT_I2C_Init();

    input_drv.init(&ext_i2cbus);

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
    int8_t raw_data[6], x, y;
    static uint8_t key = 'a';
    uint32_t read = 0;

    //if(vc_getCharNonBlocking((char*)&key)){
    //    printf("new key %c\n", key);
    //}

    if (BSP_PB_GetState(BUTTON_WAKEUP) == GPIO_PIN_SET)
    {
        key = PB_VALUE;
        read = 1; 
    }
    
    input_drv.read((uint8_t*)raw_data, 1);
    
    if(raw_data[0] != -1){
        uint8_t btn = ~(uint8_t)raw_data[0];
        printf("IO %x\n", btn);
        if(btn & 1){
            key = 'w';
            read = 1;
        }else if(btn & 2){
            key = 's';
            read = 1;
        }else if(btn & 4){
            key = 'a';
            read = 1;
        }else if(btn & 8){
            key = 'd';
            read = 1;
        }
    }

     *dst = key;
return read;

    if(input_drv.read((uint8_t*)raw_data, sizeof(raw_data)) == 0){
        // No new data, return previous data and state
        *dst = key;
        return 1;
    }

    y = raw_data[0];
    x = raw_data[2];

    //printf("%d,%d\n",x,y);
    
    if(y > 10){
        key = 'w';
        read = 1;
    }else if(y < -10){
        key = 's';
        read = 1;
    }

    // if(x > 10){
    //     *dst = 'a';
    //     read = 1;
    // }else if(x < -10){
    //     *dst = 'd';
    //     read = 1;
    // }
     *dst = key;   
    return read;
}

/**
 * Button handling
 *
 */
static BUTTON_Controller __button = {
    .scanned = BUTTON_EMPTY,
    .last = BUTTON_EMPTY,
    .events = BUTTON_EMPTY,
    .htime = 2000};

int BUTTON_Read(void)
{
    uint32_t scanned;

    if (INPUT_Read(&scanned, 1) == 0)
    {
        scanned = BUTTON_EMPTY;  // no button, or button released
    }

    switch (__button.events)
    {

    case BUTTON_EMPTY:
        //New key pressed
        if (scanned != BUTTON_EMPTY && __button.last == BUTTON_EMPTY)
        {
            __button.scanned = scanned;
            __button.last = scanned;
            __button.events = BUTTON_PRESSED;
            break;
        }

        //is Key still pressed?
        if (scanned == __button.last)
        {
            break;
        }

        //was the key was released?
        if (scanned == BUTTON_EMPTY && __button.last != BUTTON_EMPTY)
        {
            __button.scanned = __button.last;
            __button.events = BUTTON_RELEASED;
            break;
        }
        break;

    case BUTTON_PRESSED:
        //key was released
        if (scanned == BUTTON_EMPTY)
        {
            __button.scanned = __button.last;
            __button.events = BUTTON_RELEASED;
            break;
        }

        //Key still pressed
        if (scanned == __button.last)
        {
            // no new events occurred
            __button.events = BUTTON_EMPTY;
            break;
        }

        //TODO: handle multiple key presses
        __button.scanned = scanned;
        __button.last = scanned;
        break;

    case BUTTON_RELEASED:
        __button.scanned = BUTTON_EMPTY;
        __button.last = BUTTON_EMPTY;
        __button.events = BUTTON_EMPTY;
        break;

    default:
        break;
    }
    return __button.events;
}

int BUTTON_GetScanned(void)
{
    return __button.scanned;
}

int BUTTON_GetEvent(void)
{
    return __button.events;
}
