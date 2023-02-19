
#include <stdlib.h>
#include <math.h>
#include "doomdef.h"
#include "d_event.h"
#include "d_main.h"
#include "i_system.h"
#include "common.h"
#include "board.h"
#include "g_game.h"
#include "m_menu.h"

#define INPUT_DEBUG      0

#define DPAD_POSX       40
#define DPAD_POSY       (LCD_LANDSCAPE_HEIGHT/2 - 64)
#define DPAD_WIDTH      128
#define DPAD_HIGHT      128
#define DPAD_RADIUS     100

#define STICK_POSX      (LCD_LANDSCAPE_WIDTH - 128 - 40)
#define STICK_POSY      (LCD_LANDSCAPE_HEIGHT/2 - 32)
#define STICK_WIDTH     128
#define STICK_HIGHT     128
#define STICK_RADIUS    100

#define A_BTN_POSX      (800 - 256)
#define A_BTN_POSY      (480 - 55)
#define B_BTN_POSX      (800 - 150)
#define B_BTN_POSY      (480 - 80)

#define M_PI			3.141592f

enum key_states_e{
    KEY_UP  = 0,
    KEY_DOWN,
    KEY_PRESSED,
    KEY_RELEASED,
    KEY_HOLD
};

// Bit order is important
enum dpad_e{
    DPAD_LEFT = 1,
    DPAD_UP = 2,
    DPAD_RIGHT = 4,
    DPAD_DOWN = 8
};

typedef struct widget_s{
    uint16_t cx;        // X center position
    uint16_t cy;        // Y Center position
    uint16_t w;         // widget Width
    uint16_t h;         // widget Hight
    bitmap_t *fg;       // Foreground image
    bitmap_t *bg;       // Background image
    void (*update)(struct widget_s*, uint8_t);
    uint8_t (*ispressed)(struct widget_s*, uint16_t, uint16_t);
}widget_t;

typedef struct button_s{
    widget_t base;
    uint8_t *keys;
    uint8_t event;
}button_t;

typedef struct dpad_s {
    widget_t base;
    float dx;
    float dy;
    float angle;
    uint8_t *keys;
}dpad_t;


const uint8_t dpad_mask[] = {
    DPAD_RIGHT,             //     E
    DPAD_RIGHT | DPAD_UP,   //     NE
    DPAD_UP,                //     N
    DPAD_UP | DPAD_LEFT,    //     NO
    DPAD_LEFT,              //     O
    DPAD_LEFT | DPAD_DOWN,  //     SO
    DPAD_DOWN,              //     S
    DPAD_DOWN | DPAD_RIGHT  //     SE
};

const uint8_t direction_keys[] = {
    KEY_RIGHTARROW,
    0,
    KEY_UPARROW,
    0,
    KEY_LEFTARROW,
    0,
    KEY_DOWNARROW
};

static uint8_t pressed_keys[256];
static dpad_t l_dpad, r_dpad;
static button_t a_button, b_button;

static widget_t *widgets[] = {
    (widget_t*)&l_dpad, 
    (widget_t*)&r_dpad,
    (widget_t*)&a_button, 
    (widget_t*)&b_button
};

/**
 * @brief Loads a bitmap file to memory
 * 
 * @param filename  path to file
 * @param argb      Convert image to ARGB8888 format
 * @return bitmap_t* 
 */
static bitmap_t *INPUT_LoadBmp(const char *filename, uint8_t argb)
{
    uint32_t index = 0;
    uint32_t width = 0, height = 0;
    uint16_t bit_pixel = 0;
    bitmap_t *texture;
    FILE *fp;

    fp = fopen(filename, "rb");
    
    if(fp == NULL){
        return NULL;
    }

    // check bmp header
    fread(&index, 1, 2, fp);
    if(index != 0x4D42){
        return NULL;    // BM not found on header
    }    

    /* Get bitmap data address offset */
    fseek(fp, 10, SEEK_SET);
    fread(&index, 1, 4, fp);

    /* Read bitmap width */
    fseek(fp, 18, SEEK_SET);
    fread(&width, 1, 4, fp);

    /* Read bitmap height */
    fseek(fp, 22, SEEK_SET);
    fread(&height, 1, 4, fp);

    /* Read bit/pixel */
    fseek(fp, 28, SEEK_SET);
    fread(&bit_pixel, 1, 2, fp);

    if(argb){
        texture = (bitmap_t*)I_AllocLow(sizeof(bitmap_t) + (width * height * 4));
        texture->bpp = 32;
    }else{
        texture = (bitmap_t*)I_AllocLow(sizeof(bitmap_t) + (width * height * (bit_pixel/8)));
        texture->bpp = bit_pixel;
    }

    texture->w = width;
    texture->h = height;
    
    /* Skip bitmap header */
    fseek(fp, index, SEEK_SET);

    uint32_t row_size = (width * texture->bpp) / 8;
    uint8_t *pdst = &texture->data[row_size * (height - 1)];

    // Currently RGB888 and no padding
    for(index=0; index < height; index++){
        if(argb){
            for(int i = 0; i < width; i++){
                uint32_t color = LCD_COLOR_TRANSPARENT;
                fread(&color, 1, (bit_pixel/8), fp); // Read one RGB888 pixel                
                ((uint32_t*)pdst)[i] = (color == LCD_COLOR_MAGENTA)? 0 : color;                
            }
        }else{        
            fread(pdst, 1, row_size, fp);
        }
        pdst -= row_size;
    }    

    fclose(fp);
    
    return texture;
}

/**
 * @brief Draw dpad bitmap
 * 
 * @param dpad directional pad
 * @param x    x offset from dpad center x position
 * @param y    y offset from dpad center y position
 */
static void INPUT_DrawDpad(widget_t *wi){  
    // Translate top left of fg bitmap
    uint16_t x = (wi->fg->w - wi->w) / 2 - ((dpad_t*)wi)->dx;
    uint16_t y = (wi->fg->h - wi->h) / 2 + ((dpad_t*)wi)->dy;

    LCD_BlendWindow(wi->fg, x + (y * wi->fg->w), wi->bg, 0, wi->cx - (wi->w/2), wi->cy - (wi->h/2), wi->w, wi->h);
}

/**
 * @brief Draw button widget
 * 
 * @param button    button widget
 * @param x         
 * @param y    
 */
static void INPUT_DrawButton(widget_t *wi){
    LCD_BlendWindow(wi->fg, 0, wi->bg, 0, wi->cx - (wi->w/2), wi->cy - (wi->h/2), wi->w, wi->h);
}

/**
 * @brief Checks if directional pad is pressed.
 * If pressed angle is stored on widget
 * 
 * @param wi        dpad widget
 * @param ts_x      X coordinate to test
 * @param ts_y      Y coordinate to test
 * @return uint8_t  0: coordinates not in dpad area
 */
static uint8_t ispressed_dpad(widget_t *wi, uint16_t ts_x, uint16_t ts_y){
    dpad_t *dpad = (dpad_t*)wi;

    // Translate to origin
    float x = ts_x - wi->cx;
    float y = -(ts_y - wi->cy);

    // Get magnitude and check if within dpad radius
    float mag = sqrtf(x * x + y * y);    

    // Normalize vector
    x /= mag;
    y /= mag;

    // Get angle
    float angle = atan2f(y, x);
    if(angle < 0){
        angle = 2 * M_PI + angle;
    }

    dpad->dx = x;
    dpad->dy = y;
    dpad->angle = angle;    

    return mag < wi->w/2;
}

/**
 * @brief Checks if a round button is pressed
 * 
 * @param wi        button widget
 * @param ts_x      X coordinate to test
 * @param ts_y      Y coordinate to test
 * @return uint8_t  0: coordinates not in button area
 */
static uint8_t ispressed_button(widget_t *wi, uint16_t ts_x, uint16_t ts_y){
    // Translate to origin
    float x = ts_x - wi->cx;
    float y = -(ts_y - wi->cy);

    // Get magnitude
    float mag = sqrtf(x * x + y * y);    

    // Normalize vector
    x /= mag;
    y /= mag;

    return mag < (float)wi->w/2;
}

static void update_stick(widget_t *wi, uint8_t ispressed){
    dpad_t *stick = (dpad_t*)wi;
    event_t event = {0};

    if(ispressed){
        #if INPUT_DEBUG
        COM_Print("r_dpad: (%f,%f) %f\n", stick->dx, stick->dy, stick->angle);
        #endif
        stick->dx *= 20;    // thumb stick travel distance in pixels
        stick->dy *= 20;
        event.type = ev_mouse;
        event.data2 = (int)stick->dx << 2;
        event.data3 = 0;
        D_PostEvent(&event);
    }
    else
    {
        stick->dx = 0;
        stick->dy = 0;
    } 

    INPUT_DrawDpad(wi);
}

static void update_dpad(widget_t *wi, uint8_t ispressed){
    dpad_t *dpad = (dpad_t*)wi;

    if(ispressed){
        // Adjust DPAD rotation relative to coordinate system
        dpad->angle += M_PI/8;

        // map direction
        uint8_t idx = dpad->angle / (M_PI/4);
        idx &= sizeof(dpad_mask) - 1;
        if(menuactive){
            uint8_t sym = direction_keys[idx];
            pressed_keys[sym] = (pressed_keys[sym] == KEY_UP) ? KEY_PRESSED : KEY_HOLD;
        }else{
            uint8_t mask = dpad_mask[idx];        
            for(uint8_t i = 0; i < 4; i++, mask >>= 1){
                if(mask & 1){
                    uint8_t sym = dpad->keys[i];
                    pressed_keys[sym] = (pressed_keys[sym] == KEY_UP) ? KEY_PRESSED : KEY_HOLD;
                }
            }
        }

        #if INPUT_DEBUG 
        const char *dir[] = {"E", "NE", "N", "NO", "O", "SO", "S", "SE"};
        COM_Print("DPAD: %s\n", dir[idx]);
        #endif
    }else{

    }
}

static void update_button(widget_t *wi, uint8_t ispressed){
    button_t *btn = (button_t*)wi;

    if(ispressed){        
        uint8_t sym = menuactive ? btn->keys[1] : btn->keys[0];            
        pressed_keys[sym] = (pressed_keys[sym] == KEY_UP) ? KEY_PRESSED : KEY_HOLD;
    #if INPUT_DEBUG
        if(btn->event == TOUCH_EVENT_NO_EVT){
            btn->event = TOUCH_EVENT_PRESS_DOWN;
            COM_Print("button pressed '%c'\n", (char)sym);
        }

    }else{        
        if(btn->event == TOUCH_EVENT_PRESS_DOWN){
            btn->event = TOUCH_EVENT_NO_EVT;       
            COM_Print("button released '%c'\n", (char)btn->keys[0]);
        }
    }
    #endif
    }
}
//
//
//
static void INPUT_GetTouchEvent(void){
    TS_StateTypeDef ts_state;
    uint8_t wi_cnt;
    uint8_t wi_mask = 0;

    BSP_TS_GetState(&ts_state);
    wi_cnt = sizeof(widgets) / sizeof(widget_t*);

    for(uint8_t index = 0; index < ts_state.touchDetected; index++){
        for(int i = 0; i < wi_cnt; i++){
            if(widgets[i]->ispressed(widgets[i], ts_state.touchX[index], ts_state.touchY[index])){
                if(!(wi_mask & (1 << i))){
                    widgets[i]->update(widgets[i], true);
                    wi_mask |= (1<<i);
                }
            }
        }
    }

    for(int i = 0; i < wi_cnt; i++){ 
        if(!(wi_mask & (1 << i)))
            widgets[i]->update(widgets[i], false);        
    }
}

//
// I_GetEvent
//  
void I_GetEvent(void)
{
    uint8_t sym, esc_seq;
    event_t event = {0};

    INPUT_GetTouchEvent();

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
                case KEY_ESCAPE: esc_seq = 1; break;
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

    if(esc_seq == 1 || BSP_PB_GetState(BUTTON_USER)){
        // escape key pressed
        pressed_keys[KEY_ESCAPE] = (pressed_keys[KEY_ESCAPE] == KEY_UP)? KEY_PRESSED : KEY_HOLD;
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


/**
 * @brief 
 * 
 */
void INPUT_Init(void)
{
    char path[64];

    BSP_PB_Init(BUTTON_USER, BUTTON_MODE_GPIO);

    if(BSP_TS_Init(LCD_LANDSCAPE_WIDTH, LCD_LANDSCAPE_HEIGHT) != TS_OK){
        I_Error("Touch not present");
    }else{
        l_dpad.base.cx = DPAD_POSX + (DPAD_WIDTH / 2);
        l_dpad.base.cy = DPAD_POSY + (DPAD_HIGHT / 2);
        l_dpad.base.w = DPAD_WIDTH;
        l_dpad.base.h = DPAD_HIGHT;
        l_dpad.base.update = update_dpad;
        l_dpad.base.ispressed = ispressed_dpad;
        l_dpad.keys = (uint8_t*)"awds";

        r_dpad.base.cx = STICK_POSX + (STICK_WIDTH / 2);
        r_dpad.base.cy = STICK_POSY + (DPAD_HIGHT / 2);
        r_dpad.base.w = STICK_WIDTH;
        r_dpad.base.h = STICK_HIGHT;
        r_dpad.base.update = update_stick;
        r_dpad.base.ispressed = ispressed_dpad;
        
        a_button.base.cx = A_BTN_POSX;
        a_button.base.cy = A_BTN_POSY;
        a_button.base.w = 80;
        a_button.base.h = 80;
        a_button.base.update = update_button;
        a_button.base.ispressed = ispressed_button;
        a_button.event = TOUCH_EVENT_NO_EVT;
        a_button.keys = (uint8_t*)"e\x1b";

        b_button.base.cx = B_BTN_POSX;
        b_button.base.cy = B_BTN_POSY;
        b_button.base.w = 80;
        b_button.base.h = 80;
        b_button.base.update = update_button;
        b_button.base.ispressed = ispressed_button;
        b_button.event = TOUCH_EVENT_NO_EVT;
        b_button.keys = (uint8_t*)" \x0d";
#if 0
        COM_FormPath(path, basedir, "fn.bmp", sizeof(path));
        bitmap_t *tmp = INPUT_LoadBmp(path, 0);

        if(tmp){
            LCD_BlendWindow(tmp, 0, NULL, 0, 0, 0, tmp->w, tmp->h);
            free(tmp);
        }else{
            COM_Print("%s: Unable to load function keys bitmap\n");
        }

        COM_FormPath(path, basedir, "numpad.bmp", sizeof(path));
        tmp = INPUT_LoadBmp(path, 0);

        if(tmp){
            LCD_BlendWindow(tmp, 0, NULL, 0, 32, 68, tmp->w, tmp->h);
            free(tmp);
        }else{
            COM_Print("%s: Unable to load numeric keys bitmap\n");
        }
#endif
        COM_FormPath(path, basedir, "a_btn_bg.bmp", sizeof(path));
        a_button.base.fg = INPUT_LoadBmp(path, 0);
        a_button.base.bg = NULL;        
        if(a_button.base.fg){
            INPUT_DrawButton((widget_t*)&a_button);
            free(a_button.base.fg); // has no animation so can be freed
        }else{
            COM_Print("%s: Unable to load A button bitmap\n");
        }

        COM_FormPath(path, basedir, "b_btn_bg.bmp", sizeof(path));
        b_button.base.fg = INPUT_LoadBmp(path, 0);
        b_button.base.bg = NULL;
        if(b_button.base.fg){
            INPUT_DrawButton((widget_t*)&b_button);
            free(b_button.base.fg);
        }else{
            COM_Print("%s: Unable to load B button bitmap\n");
        }

        COM_FormPath(path, basedir, "dpad_bg.bmp", sizeof(path));
        l_dpad.base.fg = INPUT_LoadBmp(path, 0);
        l_dpad.base.bg = NULL;
        if(a_button.base.fg){
            INPUT_DrawDpad((widget_t*)&l_dpad);
            free(l_dpad.base.fg);
        }

        COM_FormPath(path, basedir, "stick_bg.bmp", sizeof(path));
        r_dpad.base.bg = INPUT_LoadBmp(path, 0);
        COM_FormPath(path, basedir, "stick_fg.bmp", sizeof(path));
        r_dpad.base.fg = INPUT_LoadBmp(path, 1);

        if(r_dpad.base.fg == NULL || r_dpad.base.bg == NULL){
           I_Error("Unable to load dpad bitmaps\n");
        }else{
            INPUT_DrawDpad((widget_t*)&r_dpad);
        }
    }
    memset(pressed_keys, KEY_UP, sizeof(pressed_keys));
}

void INPUT_Shutdown (void)
{
    if(l_dpad.base.fg){
        free(l_dpad.base.fg);
    }

    if(r_dpad.base.bg){
        free(r_dpad.base.bg);
    }

    if(r_dpad.base.fg){
        free(r_dpad.base.fg);
    }

    if(a_button.base.fg){
        free(a_button.base.fg);
    }

    if(b_button.base.fg){
        free(b_button.base.fg);
    }
}