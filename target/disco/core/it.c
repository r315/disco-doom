/**
  ******************************************************************************
  * @file    LCD_DSI/LCD_DSI_VideoMode_SingleBuffer/Src/stm32f7xx_it.c 
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f7xx_it.h"


typedef struct {
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
}stackframe_t;

/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup LCD_DSI_VideoMode_SingleBuffer
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern LTDC_HandleTypeDef hltdc_discovery;
extern SAI_HandleTypeDef SaiHandle;
extern SD_HandleTypeDef uSdHandle;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

void EXTI0_IRQHandler (void)
{
    if(__HAL_GPIO_EXTI_GET_IT(GPIO_PIN_0) == SET)
    {
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET){
            //Key_Event(0x1b, false);
        }else{
            //Key_Event(0x1b, true);
        }
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_0);
    }
}

/*
void LTDC_IRQHandler(void)
{
	HAL_LTDC_IRQHandler(&hltdc_discovery);
}
*/ 
void MemManage_Handler(void){
    __asm volatile(
        "bkpt #01 \n"
        "b . \n"
    );   
}

void BusFault_Handler(void){
    __asm volatile(
        "bkpt #01 \n"
        "b . \n"
    );   
}

void UsageFault_Handler(void){
    __asm volatile(
        "bkpt #01 \n"
        "b . \n"
    );   
}

typedef void(*vector_t)(void);

void Fault_Handler(void)
{
    volatile uint8_t isr_number = (SCB->ICSR & 255) - 16;
    // See position number on Table 46 from RM0410
    UNUSED(isr_number);

    __asm volatile(
        "bkpt #01 \n"
        "b . \n"
    );
}

void Stack_Dump(stackframe_t *stack){
    GPIOJ->MODER = (1 << 26);
    HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_13, GPIO_PIN_SET);

    __asm volatile(
        "bkpt #01 \n"
        "b . \n"
    );
}

/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles LTDC interrupt request.
  * @param  None
  * @retval None
  */
void LTDC_IRQHandler(void)
{
    HAL_LTDC_IRQHandler(&hltdc_discovery);
}

void SDMMC2_IRQHandler(void)
{
    HAL_SD_IRQHandler(&uSdHandle);
}

void DMA2_Stream0_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uSdHandle.hdmarx);
}

void DMA2_Stream5_IRQHandler(void)
{
    HAL_DMA_IRQHandler(uSdHandle.hdmatx);
}

void DMA2_Stream6_IRQHandler(void)
{ 
    HAL_DMA_IRQHandler(SaiHandle.hdmatx);  
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
