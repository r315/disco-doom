
#ifndef _audio_h_
#define _audio_h_

#include "board.h"

#define DEFAULT_VOLUME          20

/* SAI peripheral configuration defines */
#define AUDIO_SAIx                           SAI1_Block_A

#define AUDIO_SAIx_CLK_ENABLE()              __HAL_RCC_SAI1_CLK_ENABLE()

#define AUDIO_SAIx_FS_GPIO_PORT              GPIOE
#define AUDIO_SAIx_FS_AF                     GPIO_AF6_SAI1
#define AUDIO_SAIx_FS_PIN                    GPIO_PIN_4

#define AUDIO_SAIx_SCK_GPIO_PORT             GPIOE
#define AUDIO_SAIx_SCK_AF                    GPIO_AF6_SAI1
#define AUDIO_SAIx_SCK_PIN                   GPIO_PIN_5

#define AUDIO_SAIx_SD_GPIO_PORT              GPIOE
#define AUDIO_SAIx_SD_AF                     GPIO_AF6_SAI1
#define AUDIO_SAIx_SD_PIN                    GPIO_PIN_6

#define AUDIO_SAIx_MCLK_GPIO_PORT            GPIOG
#define AUDIO_SAIx_MCLK_AF                   GPIO_AF6_SAI1
#define AUDIO_SAIx_MCLK_PIN                  GPIO_PIN_7
   
#define AUDIO_SAIx_MCLK_ENABLE()             __HAL_RCC_GPIOG_CLK_ENABLE()
#define AUDIO_SAIx_SCK_ENABLE()              __HAL_RCC_GPIOE_CLK_ENABLE()
#define AUDIO_SAIx_FS_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()
#define AUDIO_SAIx_SD_ENABLE()               __HAL_RCC_GPIOE_CLK_ENABLE()


typedef struct audiospec{
    uint32_t freq;          // freq in Hz
    uint32_t channels;      // 1: Mono, 2: Stereo
    uint32_t size;          // Number of samples of one channel in a single buffer
    uint32_t volume;        // Start volume
    uint16_t *buf;          // samples buffer
    void (*callback)(void *stream, int len);
}audiospec_t;


uint8_t AUDIO_Init(audiospec_t *spec);
void AUDIO_Shutdown(void);
void AUDIO_Stop(void);
void AUDIO_SetVolume(int vol);
#endif