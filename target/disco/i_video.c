#include <stdlib.h>

#include "doomdef.h"
#include "i_system.h"
#include "v_video.h"
#include "d_main.h"
#include "board.h"

//#define NO_VIDEO
#define DOUBLE_SCREEN 1
//#define ROTATION_180 1

#ifndef NO_VIDEO
typedef struct __attribute__((__packed__)){
    byte b;    
    byte g;
    byte r;
    byte alpha;
}palette_t;

static DMA2D_HandleTypeDef  hdma2d;
static uint32_t forground_clut[256];
#ifdef DOUBLE_SCREEN
#define FRAME_OFFSET ( ((400 - SCREENWIDTH) + ((240 - SCREENHEIGHT) * 800)) * 4)
uint8_t *dfb;
#endif
//
//  LoadPalette
//
void LoadPalette(uint32_t *clut)
{   
    DMA2D_CLUTCfgTypeDef CLUTCfg;

    hdma2d.Instance = DMA2D;   
    
    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/ 
    hdma2d.Init.Mode          = DMA2D_M2M_PFC;
    hdma2d.Init.ColorMode     = DMA2D_OUTPUT_ARGB8888;
#ifdef DOUBLE_SCREEN
    hdma2d.Init.OutputOffset  = BSP_LCD_GetXSize() - (SCREENWIDTH<<1);
#else
    hdma2d.Init.OutputOffset  = BSP_LCD_GetXSize() - SCREENWIDTH; // Offset added to the end of each line
#endif
    hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;  /* No Output Alpha Inversion*/  
    hdma2d.Init.RedBlueSwap   = DMA2D_RB_REGULAR;     /* No Output Red & Blue swap */  

    /*##-2- DMA2D Callbacks Configuration ######################################*/
    hdma2d.XferCpltCallback  = NULL;

    /*##-3- Foreground Configuration ###########################################*/
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaMode      = DMA2D_REPLACE_ALPHA;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputAlpha     = 0xFF; /* Opaque */
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputColorMode = DMA2D_INPUT_L8;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].InputOffset    = 0;
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].AlphaInverted  = DMA2D_REGULAR_ALPHA; /* No ForeGround Alpha inversion */  
    hdma2d.LayerCfg[DMA2D_FOREGROUND_LAYER].RedBlueSwap    = DMA2D_RB_REGULAR;    /* No ForeGround Red/Blue swap */ 
   
    /* DMA2D Initialization */
    if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
    {
        if(HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_FOREGROUND_LAYER) == HAL_OK) 
        {
            /* Load DMA2D Foreground CLUT */
            CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;     
            CLUTCfg.pCLUT = clut;
            CLUTCfg.Size = 255;

            if(HAL_DMA2D_CLUTLoad(&hdma2d, CLUTCfg, DMA2D_FOREGROUND_LAYER) == HAL_OK){
                HAL_DMA2D_PollForTransfer(&hdma2d, 100);  
            }            
        }
    }   
}
#endif

/**
 * Game API
 */
//
//  I_ShutdownGraphics
//
void I_ShutdownGraphics(void)
{ 
    if(dfb != NULL){
        free(dfb);
    }

    if(screens[0] != NULL){
        free(screens[0]);
    }   
}

//
// I_StartTic
//
void I_StartTic(void)
{
    event_t event;
    uint8_t evt;

    event.data1 = 0;
    event.data2 = 0;
    event.data3 = 0;
    if(BUTTON_Read() != 0){
        while((evt = BUTTON_GetEvent(&event.data1)) != 0){        
            switch(evt){
                case BTN_PRESSED:
                    event.type = ev_keydown;
                    //printf("Key %d, Event: %s\n", event.data1, "key down");
                    D_PostEvent(&event);
                    break;

                case BTN_RELEASED:
                    event.type = ev_keyup;
                    //printf("Key %d, Event: %s\n", event.data1, "key up");
                    D_PostEvent(&event);
                    break;
            }        
        }     
    }  
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
#ifndef NO_VIDEO
static uint32_t fps_tick = 0;
static int fps = 0;
char tmp[5];

    if(fps_tick < HAL_GetTick()){
        sprintf(tmp, "%d", fps);
        BSP_LCD_DisplayStringAtLine(0, (uint8_t*)tmp); 
        fps = 0;
        fps_tick = HAL_GetTick() + 1000;
    }else{
        fps++;
    }

#ifdef DOUBLE_SCREEN   
    uint8_t *fb = screens[0];
    // Double the size
#ifndef ROTATION_180
    for(int i = 0; i < SCREENHEIGHT<<1; i+=2){
        for(int j = 0; j < SCREENWIDTH<<1; j+=2, fb++){
#else
    for(int i = (SCREENHEIGHT<<1) - 2; i >= 0 ; i -= 2){
        for(int j = (SCREENWIDTH<<1) - 2; j >= 0 ; j -= 2, fb++){
#endif
            uint8_t idx = *fb;
            dfb[(i*SCREENWIDTH<<1) + j] = idx;
            dfb[(i*SCREENWIDTH<<1) + j + 1] = idx;
            dfb[((i+1)*SCREENWIDTH<<1) + j] = idx;
            dfb[((i+1)*SCREENWIDTH<<1) + j + 1] = idx;
        }
    }
    

    //CopyBuffer((uint32_t *)LCD_FB_START_ADDRESS, (uint32_t *)screens[0], 400 - 160, 240 - 100, 320, 200);
    if (HAL_DMA2D_Start(&hdma2d, (uint32_t)dfb, LCD_FB_START_ADDRESS + FRAME_OFFSET, SCREENWIDTH<<1, SCREENHEIGHT<<1) == HAL_OK)
    {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);               
    }
#else
    #define FRAME_OFFSET ( ((800 - SCREENWIDTH)/2 + ((480 - SCREENHEIGHT) * 400)) * 4)
    if (HAL_DMA2D_Start(&hdma2d, (uint32_t)screens[0], LCD_FB_START_ADDRESS + FRAME_OFFSET, SCREENWIDTH, SCREENHEIGHT) == HAL_OK)
    {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 100);               
    }
#endif /* DOUBLE_SCREEN */
#endif /* NO_VIDEO */
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr)
{
    memcpy(scr, screens[0], SCREENWIDTH * SCREENHEIGHT);
}

//
// I_SetPalette
//
void I_SetPalette(byte *palette)
{
#ifndef NO_VIDEO
  palette_t *pClut = (palette_t*)forground_clut;
  byte brightness = 50;

    for (uint32_t i = 0; i < 256; ++i, ++pClut)
    {
        pClut->r = gammatable[usegamma][*palette++];
        pClut->g = gammatable[usegamma][*palette++];
        pClut->b = gammatable[usegamma][*palette++];
        pClut->alpha = 0xFF;

        pClut->r = (pClut->r + brightness) < 256 ? pClut->r + brightness : 255;
        pClut->g = (pClut->g + brightness) < 256 ? pClut->g + brightness : 255;
        pClut->b = (pClut->b + brightness) < 256 ? pClut->b + brightness : 255;
    }
    
    LoadPalette(forground_clut);
#endif
}

//
// I_InitGraphics
//
void I_InitGraphics(void)
{
#ifndef NO_VIDEO    
    OnError_Handler(BSP_LCD_Init() != LCD_OK);

    BSP_LCD_LayerDefaultInit(DMA2D_FOREGROUND_LAYER, LCD_FB_START_ADDRESS);     
    BSP_LCD_SelectLayer(DMA2D_FOREGROUND_LAYER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);

    BSP_LED_On(LED2);

    BSP_LCD_SetFont(&Font16);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);

#endif
    screens[0] = (unsigned char *)malloc(SCREENWIDTH * SCREENHEIGHT);

    if (screens[0] == NULL)
        I_Error("Couldn't allocate screen memory");

    #ifdef DOUBLE_SCREEN
        dfb = malloc(SCREENWIDTH * SCREENHEIGHT * 4);
    #endif
}


