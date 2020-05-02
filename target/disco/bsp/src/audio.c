#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_audio.h"
#include "audio.h"
#include "wm8994.h"

static audiospec_t *audio;
static AUDIO_DrvTypeDef *drv;
static DMA_HandleTypeDef hSaiDma;

SAI_HandleTypeDef SaiHandle;

uint16_t audio_buffer[AUDIO_MAX_BUFF_SIZE];

void AUDIO_Init(audiospec_t *spec)
{
    audio = spec;
    drv = &wm8994_drv;
    spec->buf = audio_buffer;

    AUD_HW_Init(audio);
    /* Initialize audio driver */
    if (WM8994_ID != drv->ReadID(AUDIO_I2C_ADDRESS))
    {
        Error_Handler();
    }
    drv->Reset(AUDIO_I2C_ADDRESS);

    if (drv->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_HEADPHONE, DEFAULT_VOLUME, audio->freq) != 0 || audio->callback == NULL)
    {
        Error_Handler();
    }

    spec->playing = 0;

    /* Start the playback */
    if (drv->Play(AUDIO_I2C_ADDRESS, NULL, 0) != 0)
    {
        Error_Handler();
    }
}

void AUDIO_Play(audiospec_t *spec)
{
    if (spec->playing)
    {
        return;
    }

    if (HAL_SAI_Transmit_DMA(&SaiHandle, (uint8_t *)spec->buf, spec->size) != HAL_OK)
    {
        Error_Handler();
    }

    spec->playing = 1;
}

void AUDIO_Stop(audiospec_t *spec)
{
    HAL_SAI_DMAStop(&SaiHandle);
    spec->playing = 0;
}

void AUDIO_SetVolume(int vol)
{
    //drv->SetVolume(AUDIO_I2C_ADDRESS, vol);
}

/**
  * @brief  Playback initialization
  * @param  None
  * @retval None
  */
void AUD_HW_Init(audiospec_t *spec)
{
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;

    if ((spec->freq == AUDIO_FREQUENCY_11K) || (spec->freq == AUDIO_FREQUENCY_22K) || (spec->freq == AUDIO_FREQUENCY_44K))
    {
        /* Configure PLLSAI prescalers */
        /* PLLSAI_VCO: VCO_429M 
        SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
        SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */
        RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI2;
        RCC_PeriphCLKInitStruct.Sai2ClockSelection = RCC_SAI2CLKSOURCE_PLLSAI;
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = 429;
        RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = 2;
        RCC_PeriphCLKInitStruct.PLLSAIDivQ = 19;
    }
    else
    { /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
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

    SaiHandle.Init.AudioFrequency = spec->freq;
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
  * @brief  SAI MSP Init.
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
    GPIO_InitTypeDef GPIO_Init;

    /* Enable SAI1 clock */
    __HAL_RCC_SAI1_CLK_ENABLE();

    /* Configure GPIOs used for SAI2 */
    AUDIO_SAIx_MCLK_ENABLE();
    AUDIO_SAIx_SCK_ENABLE();
    AUDIO_SAIx_FS_ENABLE();
    AUDIO_SAIx_SD_ENABLE();

    GPIO_Init.Mode = GPIO_MODE_AF_PP;
    GPIO_Init.Pull = GPIO_NOPULL;
    GPIO_Init.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_Init.Alternate = AUDIO_SAIx_FS_AF;
    GPIO_Init.Pin = AUDIO_SAIx_FS_PIN;
    HAL_GPIO_Init(AUDIO_SAIx_FS_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Alternate = AUDIO_SAIx_SCK_AF;
    GPIO_Init.Pin = AUDIO_SAIx_SCK_PIN;
    HAL_GPIO_Init(AUDIO_SAIx_SCK_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Alternate = AUDIO_SAIx_SD_AF;
    GPIO_Init.Pin = AUDIO_SAIx_SD_PIN;
    HAL_GPIO_Init(AUDIO_SAIx_SD_GPIO_PORT, &GPIO_Init);
    GPIO_Init.Alternate = AUDIO_SAIx_MCLK_AF;
    GPIO_Init.Pin = AUDIO_SAIx_MCLK_PIN;
    HAL_GPIO_Init(AUDIO_SAIx_MCLK_GPIO_PORT, &GPIO_Init);

    /* Configure DMA used for SAI2 */
    __HAL_RCC_DMA2_CLK_ENABLE();

    if (hsai->Instance == AUDIO_SAIx)
    {
        hSaiDma.Init.Channel = DMA_CHANNEL_10;
        hSaiDma.Init.Direction = DMA_MEMORY_TO_PERIPH;
        hSaiDma.Init.PeriphInc = DMA_PINC_DISABLE;
        hSaiDma.Init.MemInc = DMA_MINC_ENABLE;
        hSaiDma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hSaiDma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hSaiDma.Init.Mode = DMA_CIRCULAR;
        hSaiDma.Init.Priority = DMA_PRIORITY_HIGH;
        hSaiDma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
        hSaiDma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
        hSaiDma.Init.MemBurst = DMA_MBURST_SINGLE;
        hSaiDma.Init.PeriphBurst = DMA_PBURST_SINGLE;

        /* Select the DMA instance to be used for the transfer : DMA2_Stream6 */
        hSaiDma.Instance = DMA2_Stream6;

        /* Associate the DMA handle */
        __HAL_LINKDMA(hsai, hdmatx, hSaiDma);

        /* Deinitialize the Stream for new transfer */
        HAL_DMA_DeInit(&hSaiDma);

        /* Configure the DMA Stream */
        if (HAL_OK != HAL_DMA_Init(&hSaiDma))
        {
            Error_Handler();
        }
    }

    HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 10, 9);
    HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
}

/**
  * @brief Tx Transfer completed callbacks.
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    audio->callback(audio->buf + (audio->size >> 1), audio->size >> 1);
}

/**
  * @brief Tx Transfer Half completed callbacks
  * @param  hsai : pointer to a SAI_HandleTypeDef structure that contains
  *                the configuration information for SAI module.
  * @retval None
  */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    audio->callback(audio->buf, audio->size >> 1);
}
