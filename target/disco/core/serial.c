
#include <stdint.h>
#include <stdio.h>

#include "stm32f7xx_hal.h"
#include "fifo.h"
#include "serial.h"

#define UART_BAUDRATE   115200
#define UART_FIFO_SIZE  128

#ifdef __GNUC__
/* With GCC, small printf (option LD Linker->Libraries->Small printf
   set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */


static fifo_t rxfifo, txfifo;
static uint8_t rxbuf[UART_FIFO_SIZE];
static uint8_t txbuf[UART_FIFO_SIZE];

UART_HandleTypeDef huart;

/**
  * @brief USART1 Initialization Function
  *   DO NOT FORGET to implement
  *   void HAL_UART_MspInit(UART_HandleTypeDef* huart);
  *   on xxxxxxx_hal_msp.c
  * 
  * @param None
  * @retval None
  */
void SERIAL_Init(void){
    huart.Instance = USART1;
    huart.Init.BaudRate = UART_BAUDRATE;
    huart.Init.WordLength = UART_WORDLENGTH_8B;
    huart.Init.StopBits = UART_STOPBITS_1;
    huart.Init.Parity = UART_PARITY_NONE;
    huart.Init.Mode = UART_MODE_TX_RX | USART_CR1_RXNEIE;
    huart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart.Init.OverSampling = UART_OVERSAMPLING_16;
    huart.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    HAL_UART_Init(&huart);

    fifo_init(&rxfifo, rxbuf, UART_FIFO_SIZE);
    fifo_init(&txfifo, txbuf, UART_FIFO_SIZE);

    NVIC_SetPriority(USART1_IRQn, NVIC_PRIORITYGROUP_0);
    NVIC_EnableIRQ(USART1_IRQn);    
}

uint8_t SERIAL_GetChar(char *c){
   return fifo_get(&rxfifo, (uint8_t*)c);
}

uint32_t SERIAL_Write(char *data, uint32_t len) {
	uint32_t sent = 0;
	while(len--){
		if(fifo_free(&txfifo) == 0){
			SET_BIT(huart.Instance->CR1, USART_CR1_TXEIE);
			while(fifo_free(&txfifo) == 0);
		}
		fifo_put(&txfifo, *(uint8_t*)data++);
		sent++;
	}

	SET_BIT(huart.Instance->CR1, USART_CR1_TXEIE);

	return sent;
}

PUTCHAR_PROTOTYPE {
    SERIAL_Write((char*)&ch, 1);
    return ch;
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  //BSP_LED_Toggle(LED_RED);
    uint32_t isrflags = huart.Instance->ISR;
	uint32_t cr1its = huart.Instance->CR1;
    uint8_t sym;

	/* If no error occurs */
	uint32_t errorflags = isrflags & (uint32_t)(
			USART_ISR_PE | USART_ISR_FE | USART_ISR_NE | USART_ISR_ORE | USART_ISR_RTOF);

	if (errorflags == 0U) {
		if (((isrflags & USART_ISR_RXNE) != 0U)
				&& ((cr1its & USART_CR1_RXNEIE) != 0U)) {
                    sym = READ_REG(huart.Instance->RDR);
			fifo_put(&rxfifo, (uint8_t) sym);

            //huart.Instance->TDR = hex_tbl[sym >> 4];
            //while(!( huart.Instance->ISR & USART_ISR_TXE));
            //huart.Instance->TDR = hex_tbl[sym & 15];
            //huart.Instance->TDR = sym;
            //while(!( huart.Instance->ISR & USART_ISR_TXE));
		}

		if (((isrflags & USART_ISR_TXE) != 0U)
				&& ((cr1its & USART_CR1_TXEIE) != 0U)) {
			if (fifo_get(&txfifo, (uint8_t*) &huart.Instance->TDR) == 0U) {
				/* No data transmitted, disable TXE interrupt */
				CLEAR_BIT(huart.Instance->CR1, USART_CR1_TXEIE);
			}
		}
	} else {
		fifo_flush(&rxfifo);
        fifo_flush(&txfifo);
        huart.Instance->ICR = errorflags;
	}
}
