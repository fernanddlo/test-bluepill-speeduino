#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"
#include "STM32_specific.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart3;

// ============================================================
// Cortex-M3 CORE INTERRUPT HANDLERS
// ============================================================

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

// ============================================================
// SYSTICK
// ============================================================

void SysTick_Handler(void)
{
    HAL_IncTick();
}

// ============================================================
// TIM1 CAPTURE / UPDATE
// ============================================================

void TIM1_CC_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}

void TIM1_UP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim1);
}

// ============================================================
// TIM4 - 100 Hz
// ============================================================

void TIM4_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim4);
}

// ============================================================
// USART3
// ============================================================

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}