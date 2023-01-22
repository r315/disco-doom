#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "v_video.h"
#include "d_main.h"
#include "board.h"
#include "hu_lib.h"
#include "hu_stuff.h"
#include "i_input.h"

#define VIDEO_ENABLED       1

#if VIDEO_ENABLED

#define DOUBLE_SCREEN       1
#define VIDEO_PALETTE_DMA   0
#define VIDEO_DMA           1
#define VIDEO_DRAW_PALETTE  1
#define VIDEO_LAYER_WINDOW	0

#define VIDEO_LAYER			DMA2D_FOREGROUND_LAYER
#define VIDEO_LAYER_BASE    LCD_FG_BASE_ADDR
#define VIDEO_LAYER_SIZE    (LCD_FB_SIZE / 2)

#if DOUBLE_SCREEN
#define CANVAS_WIDTH        (SCREENWIDTH * 2)
#define CANVAS_HEIGHT       (SCREENHEIGHT * 2)
#else
#define CANVAS_WIDTH        SCREENWIDTH
#define CANVAS_HEIGHT       SCREENHEIGHT
#endif

#if VIDEO_LAYER_WINDOW
#define VIDEO_WINDOW		VIDEO_LAYER_BASE
#else
#define VIDEO_WINDOW		(VIDEO_LAYER_BASE + ( (((800 - CANVAS_WIDTH)/2) + ((480 - CANVAS_HEIGHT) * 400)) * 4))
#endif

#define DMA2D_CR_M2M (0 << 16)
#define DMA2D_CR_M2M_PFC (1 << 16)
#define DMA2D_CR_M2M_BLEND (2 << 16)
#define DMA2D_CR_R2M (3 << 16)
#define DMA2D_FGPFCCR_SET_ALPHA(a) ((a << 24) | (1 << 16))
#define DMA2D_FGPFCCR_SET_CS(cs) ((cs) << 8)	// CLUT size
#define DMA2D_FGPFCCR_SET_CM(cm) ((cm) << 0)  // Input Color mode
#define DMA2D_OPFCCR_SET_CM(cm) ((cm) << 0)	// Output Color mode
#define DMA2D_NLR_PLNL(pl, nl) (((pl) << 16) | nl)

#define LCD_BACKGROUND_COLOR    0xFF484848

#if DOUBLE_SCREEN
static uint8_t *dfb;
#endif

#endif

static hu_textline_t	h_fps;
static byte	*screen_buffer;

#if VIDEO_ENABLED
static void LCD_ConfigVideoDma(uint32_t src, uint16_t w, uint16_t h)
{
    DMA2D->CR = DMA2D_CR_M2M_PFC;
    DMA2D->FGMAR = src;
    DMA2D->FGOR = 0;
    DMA2D->FGPFCCR = DMA2D_FGPFCCR_SET_ALPHA(0xFF) |        // Replace alpha 
                     DMA2D_FGPFCCR_SET_CS(256 - 1) |        // CLUT Size
                     DMA2D_FGPFCCR_SET_CM(DMA2D_INPUT_L8) | // Input color format
                     DMA2D_FGPFCCR_CCM;                     // RGB CLUT Mode
    DMA2D->OPFCCR = DMA2D_OPFCCR_SET_CM(DMA2D_OUTPUT_ARGB8888) |
                    //DMA2D_OPFCCR_RBS |                    // Swap Red Blue
                    0;
    DMA2D->OMAR = (uint32_t)VIDEO_WINDOW;                   // Absolute memory address 
    DMA2D->OOR = BSP_LCD_GetXSize() - w;                    // Add offset to start of next line
    DMA2D->NLR = DMA2D_NLR_PLNL(w, h);

    LTDC->BCCR = LCD_BACKGROUND_COLOR;
}
#endif
/**
 * Public API
 */

//
//  I_ShutdownGraphics
//
void I_ShutdownGraphics(void)
{ 
#if DOUBLE_SCREEN
    if(dfb != NULL){
        free(dfb);
    }
#endif
    // FIX: free only if was allocated by this module
    //if(screen_buffer != NULL){
    //    free(screen_buffer);
    //}   
}

//
// I_StartTic
//
void I_StartTic(void)
{
    I_GetEvent();
}

//
// I_UpdateNoBlit
//
void I_UpdateNoBlit(void)
{

}

//
// I_FinishUpdate
//
void I_FinishUpdate(void)
{
#if VIDEO_ENABLED

    if(d_devparm){
        static uint32_t fps_tick = 0;
        static int fps = 0;
        static char txt[5];

        if(fps_tick < HAL_GetTick()){
            sprintf(txt, "%d", fps);
            //BSP_LCD_DisplayStringAtLine(0, (uint8_t*)txt); 
            fps = 0;
            fps_tick = HAL_GetTick() + 1000;
        }else{
            fps++;
        }

        HUlib_clearTextLine(&h_fps);
        char *ptxt = txt;

        while (*ptxt) {
            HUlib_addCharToTextLine(&h_fps, *ptxt++);
        }
        HUlib_drawTextLine(&h_fps, false);
    }

#if DOUBLE_SCREEN   
    uint8_t *fb = screen_buffer;
    // Double the size
    for(int i = 0; i < CANVAS_HEIGHT; i+=2){
        for(int j = 0; j < CANVAS_WIDTH; j+=2, fb++){
            uint8_t idx = *fb;
            dfb[(i*CANVAS_WIDTH) + j] = idx;
            dfb[(i*CANVAS_WIDTH) + j + 1] = idx;
            dfb[((i+1)*CANVAS_WIDTH) + j] = idx;
            dfb[((i+1)*CANVAS_WIDTH) + j + 1] = idx;
        }
    }
#endif /* DOUBLE_SCREEN */

    DMA2D->CR |= DMA2D_CR_START;
    do{

    }while(DMA2D->CR & DMA2D_CR_START);
#endif /* VIDEO_ENABLED */
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *dst)
{
    memcpy(dst, screen_buffer, SCREENWIDTH * SCREENHEIGHT);
}

byte* I_GetScreen(void) 
{
    return screen_buffer;
}

//
// I_SetPalette
//
void I_SetPalette(byte *palette)
{
#if VIDEO_ENABLED
    #if VIDEO_PALETTE_DMA

    #else
    //byte brightness = 1;
    // Write palette directly to DMA2D foreground CLUT
    for (uint32_t i = 0; i < 256; ++i){
        #if 0
        uint8_t r = *palette++;
        uint8_t g = *palette++;
        uint8_t b = *palette++;
        #else
        uint8_t r = gammatable[usegamma][*palette++];
        uint8_t g = gammatable[usegamma][*palette++];
        uint8_t b = gammatable[usegamma][*palette++];
        
        //r = (r + brightness) < 256 ? r + brightness : 255;
        //g = (g + brightness) < 256 ? g + brightness : 255;
        //b = (b + brightness) < 256 ? b + brightness : 255;
        #endif
        ((uint32_t*)DMA2D->FGCLUT)[i] = (r << 16) | (g << 8) | (b << 0);		
    }
    #endif /* VIDEO_PALETTE_DMA */

    #if VIDEO_DRAW_PALETTE
    // Draw 3x3 px squares on bottom of display
    for(int l = 0; l < 3; l++){
        uint32_t *pdst = (uint32_t*)(VIDEO_LAYER_BASE + ((479 - l) * 800 * 4));
        for(int i = 0; i < 256 * 3; i++){
            pdst[i] = ((uint32_t*)DMA2D->FGCLUT)[i/3] | 0xFF000000;
        }
    }
    #endif /* VIDEO_DRAW_PALETTE */
#endif /* VIDEO_DISABLED */
}

//
// I_InitGraphics
//
void I_InitGraphics(void)
{
    screen_buffer = (byte*)calloc(1, SCREENWIDTH * SCREENHEIGHT);

    if (screen_buffer == NULL)
        I_Error("Couldn't allocate screen memory");

    #if DOUBLE_SCREEN
        dfb = malloc(CANVAS_WIDTH * CANVAS_HEIGHT);
        if (dfb == NULL)
            I_Error("Couldn't allocate double screen buffer");
    #endif

    HUlib_initTextLine(&h_fps, 10, 10, hu_font, HU_FONTSTART);

#if VIDEO_ENABLED    
    OnError_Handler(BSP_LCD_Init() != LCD_OK);

    BSP_LCD_LayerDefaultInit(VIDEO_LAYER, VIDEO_LAYER_BASE);
    BSP_LCD_SetColorKeying(VIDEO_LAYER, LCD_COLOR_MAGENTA);  // For controls transparencies
    // Clear LCD
    memset((uint8_t*)VIDEO_LAYER_BASE, 0, VIDEO_LAYER_SIZE);
    // Clear palette
    memset((void*)DMA2D->FGCLUT, 0, 256 * 4);

    BSP_LCD_SelectLayer(VIDEO_LAYER);

    #if DOUBLE_SCREEN
    LCD_ConfigVideoDma((uint32_t)dfb, CANVAS_WIDTH, CANVAS_HEIGHT);
    #else
    LCD_ConfigVideoDma((uint32_t)screen_buffer, CANVAS_WIDTH, CANVAS_HEIGHT);
    #endif

    I_FinishUpdate();
    BSP_LED_On(LED2);
#endif
}


