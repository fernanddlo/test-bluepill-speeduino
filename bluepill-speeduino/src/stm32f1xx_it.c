#include "stm32f1xx_it.h"
#include "stm32f1xx_hal.h"
#include "config.h"
#include "rpm.h"
#include "injection.h"
#include "sensors.h"
#include "serial_protocol.h"

extern TIM_HandleTypeDef htim1;
extern UART_HandleTypeDef huart3;

volatile uint32_t systick_count = 0;

void SysTick_Handler(void) {
    HAL_IncTick();
    systick_count++;
    
    // Trigger 100Hz main loop every 10ms (LOOP_FREQUENCY_HZ = 100)
    if (systick_count % 10 == 0) {
        // Main loop logic will be in main.cpp
    }
}

void TIM1_CC_IRQHandler(void) {
    // Handle input capture on PA10 (RPM signal)
    if (__HAL_TIM_GET_FLAG(&htim1, TIM_FLAG_CC3)) {
        __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_CC3);
        RPM_CaptureISR();
    }
}

void TIM1_UP_IRQHandler(void) {
    // Handle timer overflow if needed
    __HAL_TIM_CLEAR_FLAG(&htim1, TIM_FLAG_UPDATE);
}

void USART3_IRQHandler(void) {
    // Handle USART3 RX/TX interrupts
    HAL_UART_IRQHandler(&huart3);
}

// UART RX callback (called by HAL_UART_IRQHandler)
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        Serial_RxCallback();
    }
}
