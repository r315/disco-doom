//#include "stm32f769i_discovery.h"
//#include "stm32f769i_discovery_audio.h"
//#include "audio.h"
//#include "../Components/wm8994/wm8994.h"
#include "main.h"
//#include "math.h"

#define PI 3.14159265358979323846

#define AUDIO_BUFF_SIZE 4096
#define DEFAULT_VOLUME 20

typedef void (*callback_t)(void *stream, uint32_t len);

SAI_HandleTypeDef SaiHandle;
AUDIO_DrvTypeDef *audio_drv;
uint16_t *buffer_pos;
uint16_t audio_buffer[AUDIO_BUFF_SIZE];
uint32_t audio_buffer_len;
uint32_t sample_rate;
callback_t call_back;

double sampleSquaWare(double f, double t);
void AUD_HW_Init(uint32_t sr);

AUDIO_DrvTypeDef *AUD_Init(uint32_t freq, uint8_t channels, uint32_t samples, callback_t callback)
{
    AUDIO_DrvTypeDef *drv = &wm8994_drv;
    sample_rate = freq;
    AUD_HW_Init(freq);
    /* Initialize audio driver */
    if (WM8994_ID != drv->ReadID(AUDIO_I2C_ADDRESS))
    {
        Error_Handler();
    }

    drv->Reset(AUDIO_I2C_ADDRESS);

    if (0 != drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, DEFAULT_VOLUME, sample_rate) || callback == NULL)
    {
        Error_Handler();
    }

    audio_drv = drv;
    audio_buffer_len = samples;
    call_back = callback;
    buffer_pos = audio_buffer;
    return drv;
}

void AUD_Start()
{

    for (int i = 0; i < AUDIO_BUFF_SIZE; i += 2)
    {
        //audio_buffer[i] = ((i >> DIV) & 1) * 0x1000;     //2kHz on right
        //audio_buffer[i + 1] = ((i >> DIV) & 1) * 0x1000; //2kHz on left
    }

    /* Start the playback */
    if (audio_drv->Play(AUDIO_I2C_ADDRESS, NULL, 0) != 0)
    {
        Error_Handler();
    }

    if (HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)buffer_pos, audio_buffer_len) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  Playback initialization
  * @param  None
  * @retval None
  */
void AUD_HW_Init(uint32_t AudioFreq)
{
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;

    if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K)){
        /* Configure PLLSAI prescalers */
        /* PLLSAI_VCO: VCO_429M 
        SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
        SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */
        RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
        RCC_PeriphCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = 429;
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = 2;
        RCC_PeriphCLKInitStruct.PLLSAIDivQ = 19;
    }else{  /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */  
        /* SAI clock config 
        PLLSAI_VCO: VCO_344M 
        SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 344/7 = 49.142 Mhz 
        SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 49.142/1 = 49.142 Mhz */  
        RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
        RCC_PeriphCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = 344; 
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = 7; 
        RCC_PeriphCLKInitStruct.PLLSAIDivQ = 1;
    }
    
    if (HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /* Initialize SAI */
    __HAL_SAI_RESET_HANDLE_STATE(&SaiHandle);

    SaiHandle.Instance = AUDIO_SAIx;

    __HAL_SAI_DISABLE(&SaiHandle);

    SaiHandle.Init.AudioFrequency = AudioFreq;
    SaiHandle.Init.AudioMode = SAI_MODEMASTER_TX;
    SaiHandle.Init.Synchro = SAI_ASYNCHRONOUS;
    SaiHandle.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLE;
    SaiHandle.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    SaiHandle.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
    SaiHandle.Init.Protocol = SAI_FREE_PROTOCOL;
    SaiHandle.Init.DataSize = SAI_DATASIZE_16;
    SaiHandle.Init.FirstBit = SAI_FIRSTBIT_MSB;
    SaiHandle.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;

    SaiHandle.FrameInit.FrameLength = 32;
    SaiHandle.FrameInit.ActiveFrameLength = 16;
    SaiHandle.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
    SaiHandle.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    SaiHandle.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

    SaiHandle.SlotInit.FirstBitOffset = 0;
    SaiHandle.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
    SaiHandle.SlotInit.SlotNumber = 2;
    SaiHandle.SlotInit.SlotActive = (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1);

    if (HAL_OK != HAL_SAI_Init(&SaiHandle))
    {
        Error_Handler();
    }

    /* Enable SAI to generate clock used by audio driver */
    __HAL_SAI_ENABLE(&SaiHandle);
}

/**
  * @brief Tx Transfer completed callbacks.
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    call_back(&audio_buffer[audio_buffer_len / 2], audio_buffer_len/2);
}

/**
  * @brief Tx Transfer Half completed callbacks
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{    
    call_back(audio_buffer, audio_buffer_len/2);
}
