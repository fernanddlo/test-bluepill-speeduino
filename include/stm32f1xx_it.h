#ifndef STM32F1XX_IT_H
#define STM32F1XX_IT_H

#ifdef __cplusplus
extern "C" {
#endif

// System tick handler (100Hz main loop trigger)
void SysTick_Handler(void);

// Timer 1 capture/compare interrupt (RPM input on PA10)
void TIM1_CC_IRQHandler(void);

// Timer 1 update interrupt
void TIM1_UP_IRQHandler(void);

// USART3 interrupt (serial communication)
void USART3_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif // STM32F1XX_IT_H
