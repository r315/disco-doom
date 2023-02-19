#ifndef _board_h_
#define _board_h_

#include "main.h"
#include "input.h"
#include "lis302.h"
#include "pcf8574.h"
#include "serial.h"
#include "stm32f769i_discovery_ts.h"



/**
 * Heap grows up
 * 
 * +------------------+
 * |      LCD FB      |
 * +------------------+  SDRAM BASE + HEAP_SIZE (0xC0D12000)
 * |                  |
 * |                  | <- heap_end 
 * |                  |
 * +------------------+  SDRAM BASE
 */

#define SDRAM_BASE_ADDR   0xC0000000UL
#define SDRAM_SIZE        0x01000000UL  /* 16 MBytes */
#define MAX_HEAP_SIZE     (SDRAM_SIZE - LCD_FB_SIZE)    /* 16 - 3 MBytes */
#define LCD_FB_SIZE		  (800 * 480 * 4 * 2)  // 0x002EE000 3 MB for frame buffer
#define LCD_FB_BASE_ADDR  ((SDRAM_BASE_ADDR + SDRAM_SIZE) - LCD_FB_SIZE)

#define LCD_LANDSCAPE_WIDTH     800
#define LCD_LANDSCAPE_HEIGHT    480

#define LCD_BG_BASE_ADDR	LCD_FB_BASE_ADDR
#define LCD_FG_BASE_ADDR	(LCD_FB_BASE_ADDR + (LCD_FB_SIZE / 2))

//#define IO_EXPANDER
//#define ACCELEROMETER

#define HW_DBG_PIN_HIGH		HAL_GPIO_WritePin(DBG_PIN_PORT, DBG_PIN, GPIO_PIN_SET)
#define HW_DBG_PIN_LOW		HAL_GPIO_WritePin(DBG_PIN_PORT, DBG_PIN, GPIO_PIN_RESET)
#define HW_DBG_PIN_TOGGLE	HAL_GPIO_TogglePin(DBG_PIN_PORT, DBG_PIN)

typedef struct _i2cbus_t{
    uint32_t   (*read)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
    uint32_t   (*write)(uint8_t Addr, uint8_t *Buffer, uint16_t Length);
}i2cbus_t;

typedef struct bitmap_s {
    uint16_t w;
    uint16_t h;
    uint32_t bpp;
    uint8_t data[]; // Must be 32-bit aligned for DMA
}bitmap_t;


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

void LCD_BlendWindow(bitmap_t *fg, uint32_t fg_offset, bitmap_t *bg, uint32_t bg_offset, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void INPUT_Init(void);

void __debugbreak(void);
int __getsize(int fd);
#endif