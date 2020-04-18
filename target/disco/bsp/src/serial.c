
#include <stdint.h>
#include <stdio.h>

#include "fifo.h"
#include "main.h"

#define UART_BAUDRATE 115200
#define VCOM_QUEUE_LENGTH 128
#define VCOM_QUEUE_ITEM_SIZE 1

static fifo_t rx_fifo;

UART_HandleTypeDef huart1;

/**
  * @brief USART1 Initialization Function
  *   DO NOT FORGET to implement
  *   void HAL_UART_MspInit(UART_HandleTypeDef* huart);
  *   on xxxxxxx_hal_msp.c
  * 
  * @param None
  * @retval None
  */
void Serial_Init(void){
  huart1.Instance = USART1;
  huart1.Init.BaudRate = UART_BAUDRATE;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX | USART_CR1_RXNEIE;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  HAL_UART_Init(&huart1);
  fifo_init(&rx_fifo);  
  NVIC_SetPriority(USART1_IRQn, NVIC_PRIORITYGROUP_0);
  NVIC_EnableIRQ(USART1_IRQn);    
}

void vc_putchar(char c){
  HAL_UART_Transmit(&huart1, (uint8_t *)&c, 1, 0xFFFF);
}

void vc_puts(const char* str){
  int size = 0;
  while(*(str + size))
    size++;
   HAL_UART_Transmit(&huart1, (uint8_t *)str, size, 0xFFFF);
}

/**
 * Blocking call
 * */
char vc_getchar(void){
    uint8_t c;
    while(fifo_get(&rx_fifo, &c) == 0);
    return (char)c;
}

uint8_t vc_getCharNonBlocking(char *c){
   return fifo_get(&rx_fifo, (uint8_t*)c);
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
  uint8_t data = USART1->RDR;
  fifo_put(&rx_fifo, data);  
  BSP_LED_Toggle(LED_RED);
}
