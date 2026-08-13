#ifndef STM32_SPECIFIC_H
#define STM32_SPECIFIC_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "config.h"

// ============ FORWARD DECLARATIONS ============
void SystemClockConfig(void);
void GPIO_Init(void);
void ADC_Init(void);
void Timer_Init(void);
void UART_Init(void);
void STM32_Init(void);  // Master init function

// ============ PERIFÉRICOS HAL ============
extern TIM_HandleTypeDef htim1;   // PWM + RPM
extern TIM_HandleTypeDef htim4;   // Loop timer
extern ADC_HandleTypeDef hadc1;   // ADC para sensores
extern UART_HandleTypeDef huart3; // USART3 (TunerStudio)

// ============ TIMERS INSTANCES ============
#define TIM_PWM TIM1          // PWM injeção (PA8, PA9)
#define TIM_RPM TIM1          // Captura RPM (PA10)
#define TIM_LOOP TIM4         // Timer para loop principal

// ============ ADC INSTANCE ============
#define ADC_INSTANCE ADC1

// ============ UART INSTANCE ============
#define UART_TUNER USART3

// ============ CLOCK SPEED ============
#define SYSCLK_FREQ 72000000  // 72 MHz (STM32F1)
#define APB1_FREQ 36000000    // 36 MHz (timers APB1)
#define APB2_FREQ 72000000    // 72 MHz (timers APB2)

// ============ HELPER MACROS ============
#define SET_PIN(port, pin)   HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define CLEAR_PIN(port, pin) HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define READ_PIN(port, pin)  HAL_GPIO_ReadPin(port, pin)

#endif // STM32_SPECIFIC_H
