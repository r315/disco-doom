
#include "doomdef.h"
#include "d_event.h"
#include "serial.h"
#include "d_main.h"

enum key_states_e{
    KEY_UP  = 0,
    KEY_DOWN,
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_HOLD
};

static uint8_t pressed_keys[256];

//
// I_GetEvent
//  
void I_GetEvent(void)
{
    uint8_t sym, esc_seq;
    event_t event = {0};

    esc_seq = 0;

    while(SERIAL_GetChar((char*)&sym)){
        if(esc_seq == 2){
            switch(sym){
                case 0x41: sym = KEY_UPARROW; break;
                case 0x42: sym = KEY_DOWNARROW; break;
                case 0x43: sym = KEY_RIGHTARROW; break;
                case 0x44: sym = KEY_LEFTARROW; break;
            }
            esc_seq = 0;
        }else{
            switch(sym){
                case 0x1b: esc_seq = 1; break;
                case '\n': sym = '\r'; break;
                case '[':
                    if(esc_seq == 1)
                        esc_seq = 2;                   
                    break;

                case '\b':
                    sym = KEY_BACKSPACE;
                    break;

                default:
                    if(esc_seq == 1)
                        esc_seq = 0;

                break;
            }
        }        

        if(esc_seq == 0){
            pressed_keys[sym] = (pressed_keys[sym] == KEY_UP)? KEY_PRESSED : KEY_HOLD;
        }
    }

    if(esc_seq == 1){
        // escape key pressed
        pressed_keys[sym] = KEY_PRESSED;
    }

    sym = 255;
    do{
        switch(pressed_keys[sym]){
            case KEY_UP: break;

            case KEY_PRESSED:
                pressed_keys[sym] = KEY_DOWN;
                event.type = ev_keydown;
                event.data1 = sym;
                D_PostEvent(&event);
                break;

            case KEY_HOLD:
                pressed_keys[sym] = KEY_DOWN;
                break;

            default:
                pressed_keys[sym] = KEY_UP;
                event.type = ev_keyup;
                event.data1 = sym;
                D_PostEvent(&event);
                break;
        }
    }while(sym--);    
}

