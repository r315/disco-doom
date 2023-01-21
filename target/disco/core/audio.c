#include "stm32f769i_discovery.h"
#include "stm32f769i_discovery_audio.h"
#include "audio.h"
#include "wm8994.h"

#define AUDIO_DYNAMIC_BUFFER 1

#define AUDIO_BUFFER_DOUBLE 2 // 1: single buffer, 2: double buffer

#if !AUDIO_DYNAMIC_BUFFER
#define AUDIO_SAMPLES       512
#define AUDIO_CHANNELS      2
#define AUDIO_BUFFER_SIZE   (AUDIO_SAMPLES * AUDIO_CHANNELS * AUDIO_BUFFER_DOUBLE)

static uint16_t          audio_buffer[AUDIO_BUFFER_SIZE];
#endif

static audiospec_t       *audio_specs;
static AUDIO_DrvTypeDef  *codec;
static DMA_HandleTypeDef hSaiDma;
static SAI_HandleTypeDef hSai;


void AUDIO_Start(audiospec_t *spec)
{
    if(spec->buf != NULL && spec->callback != NULL){
        audio_specs = spec;
        HAL_SAI_Transmit_DMA(&hSai, (uint8_t *)spec->buf, spec->size * spec->channels * AUDIO_BUFFER_DOUBLE);
    }
}

void AUDIO_Stop(audiospec_t *spec)
{
    HAL_SAI_DMAStop(&hSai);
#if AUDIO_DYNAMIC_BUFFER
    if(spec->buf){
        free(spec->buf);
    }
#endif
}

void AUDIO_SetVolume(int vol)
{
    //codec->SetVolume(AUDIO_I2C_ADDRESS, vol);
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

    /* Configure GPIOs
	 * PE3    SAI1_B SD		DI
	 * PE4    SAI1_A FS		LRCLK
	 * PE5	  DAI1_A SCK	BCLK
	 * PE6	  SAI1_A SD		DO
	 * PG7	  SAI1_A MCLK	MCLK
	 * PJ12	  Codec INT
	 * */
   	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

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
        OnError_Handler(HAL_DMA_Init(&hSaiDma) != HAL_OK);
    	
		HAL_NVIC_SetPriority(DMA2_Stream6_IRQn, 0x01, 0);
    	HAL_NVIC_EnableIRQ(DMA2_Stream6_IRQn);
    }
}

static void AUDIO_Init_LL(audiospec_t *spec, SAI_HandleTypeDef *handle, DMA_HandleTypeDef *hdma)
{
    /* Configure I2S PLL */
    RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct = {0};

	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
	RCC_PeriphCLKInitStruct.Sai1ClockSelection   = RCC_SAI1CLKSOURCE_PLLI2S;

    if ((spec->freq == AUDIO_FREQUENCY_11K) || (spec->freq == AUDIO_FREQUENCY_22K) || (spec->freq == AUDIO_FREQUENCY_44K)){
        /* PLLI2S_VCO: VCO_429M 
        SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 429/2 = 214.5 Mhz
        SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 214.5/19 = 11.289 Mhz */
        RCC_PeriphCLKInitStruct.PLLI2S.PLLI2SN = 429;
        RCC_PeriphCLKInitStruct.PLLI2S.PLLI2SQ = 2;
        RCC_PeriphCLKInitStruct.PLLI2SDivQ = 19;
    } else { /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K, AUDIO_FREQUENCY_96K */
        /* PLLSAI_VCO: VCO_344M 
        SAI_CLK(first level) = PLLSAI_VCO/PLLSAIQ = 344/7 = 49.142 Mhz 
        SAI_CLK_x = SAI_CLK(first level)/PLLSAIDIVQ = 49.142/1 = 49.142 Mhz */
        RCC_PeriphCLKInitStruct.PLLI2S.PLLI2SN = 344;
        RCC_PeriphCLKInitStruct.PLLI2S.PLLI2SQ = 7;
        RCC_PeriphCLKInitStruct.PLLI2SDivQ = 1;
    }

    OnError_Handler(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK);

    /* Initialize SAI for I2S mode */
    __HAL_SAI_RESET_HANDLE_STATE(handle);

    handle->Instance = AUDIO_SAIx;

    __HAL_SAI_DISABLE(handle);

    handle->Init.AudioFrequency = spec->freq;
    handle->Init.AudioMode      = SAI_MODEMASTER_TX;
    handle->Init.Synchro        = SAI_ASYNCHRONOUS;
    handle->Init.OutputDrive    = SAI_OUTPUTDRIVE_ENABLE;
    handle->Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
    handle->Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
    handle->Init.Protocol       = SAI_FREE_PROTOCOL;
    handle->Init.DataSize       = SAI_DATASIZE_16;            // only 16 bits inside slot are data
    handle->Init.FirstBit       = SAI_FIRSTBIT_MSB;
    handle->Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;

    handle->FrameInit.FrameLength       = 32;                 // Frame size = 32 BCLK
    handle->FrameInit.ActiveFrameLength = 16;                 // FS is active half time
    handle->FrameInit.FSDefinition      = SAI_FS_CHANNEL_IDENTIFICATION;
    handle->FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
    handle->FrameInit.FSOffset          = SAI_FS_BEFOREFIRSTBIT;

    handle->SlotInit.FirstBitOffset = 0;                      // First bit on second rising edge BLCK following FS transition
    handle->SlotInit.SlotSize       = SAI_SLOTSIZE_DATASIZE;  // Size of each slot
    handle->SlotInit.SlotNumber     = spec->channels;         // One slot per channel
    handle->SlotInit.SlotActive     = (spec->channels == 2) ? (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1) : SAI_SLOTACTIVE_0;

    OnError_Handler(HAL_SAI_Init(handle) != HAL_OK);

	/* Enable SAI to generate clock used by audio driver */
   	__HAL_SAI_ENABLE(handle);

	/* Initialize audio codec */
	codec = AUDIO_Get_Driver();

    OnError_Handler(codec->ReadID(AUDIO_I2C_ADDRESS) != WM8994_ID);

    codec->Reset(AUDIO_I2C_ADDRESS);

    OnError_Handler(codec->Init(AUDIO_I2C_ADDRESS, OUTPUT_DEVICE_AUTO, spec->volume, spec->freq) != 0);
    /* Start the playback */
    OnError_Handler(codec->Play(AUDIO_I2C_ADDRESS, NULL, 0) != 0);
}

/**
 * @brief 
 * 
 * @param spec 
 */
void AUDIO_Init(audiospec_t *spec)
{

#if AUDIO_DYNAMIC_BUFFER
    spec->buf = (uint16_t*)calloc(AUDIO_BUFFER_DOUBLE, spec->size * spec->channels * sizeof(uint16_t));
    OnError_Handler(spec->buf == NULL);
#else
    spec->size = AUDIO_SAMPLES;
    spec->channels = AUDIO_CHANNELS;
    spec->buf = audio_buffer;
#endif

    AUDIO_Init_LL(spec, &hSai, &hSaiDma);
    AUDIO_Start(spec);

#if AUDIO_DEBUG_MODE
    GPIOC->MODER = (5 << 12);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
#endif
}

void DMA2_Stream6_IRQHandler(void)
{ 
    DMA_HandleTypeDef *hdma = &hSaiDma;
    uint32_t tmpisr = DMA2->HISR;
    int nsamples = audio_specs->size * audio_specs->channels;

    if (tmpisr & ((DMA_FLAG_TEIF0_4 | DMA_FLAG_FEIF0_4 | DMA_FLAG_DMEIF0_4) << hdma->StreamIndex)){
        //regs->LIFCR = (tmpisr & (DMA_FLAG_TEIF0_4 | DMA_FLAG_FEIF0_4 | DMA_FLAG_DMEIF0_4)) << hdma->StreamIndex;
    }

    if (tmpisr & (DMA_FLAG_HTIF0_4 << hdma->StreamIndex)){
        DMA2->HIFCR = DMA_FLAG_HTIF0_4 << hdma->StreamIndex;
        audio_specs->callback(audio_specs->buf, nsamples);
        #if AUDIO_DEBUG_MODE
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_7);
        #endif
    }

    if (tmpisr & (DMA_FLAG_TCIF0_4 << hdma->StreamIndex)){
        DMA2->HIFCR = DMA_FLAG_TCIF0_4 << hdma->StreamIndex;
        audio_specs->callback(audio_specs->buf + nsamples, nsamples);
        #if AUDIO_DEBUG_MODE
		HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_6);
        #endif
    }
}
