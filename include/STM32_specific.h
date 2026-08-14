#ifndef STM32_SPECIFIC_H
#define STM32_SPECIFIC_H


#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "config.h"
#include "trigger.h"


#ifdef __cplusplus
extern "C" {
#endif


/* ============================================================
 * INITIALIZATION
 * ============================================================ */

void SystemClockConfig(void);

void GPIO_Init(void);

void ADC_Init(void);

void Timer_Init(void);

void UART_Init(void);

void STM32_Init(void);


/* ============================================================
 * HAL CALLBACKS
 * ============================================================ */

void HAL_TIM_IC_CaptureCallback(
    TIM_HandleTypeDef *htim
);

void HAL_TIM_PeriodElapsedCallback(
    TIM_HandleTypeDef *htim
);


/* ============================================================
 * ERROR HANDLER
 * ============================================================ */

void STM32_FatalError(void);


/* ============================================================
 * GLOBAL HAL HANDLES
 * ============================================================ */

extern TIM_HandleTypeDef htim1;

extern TIM_HandleTypeDef htim4;

extern ADC_HandleTypeDef hadc1;

extern UART_HandleTypeDef huart3;


/* ============================================================
 * TIMER DEFINITIONS
 * ============================================================ */


/*
 * ============================================================
 * TIM1
 * ============================================================
 *
 * Timer clock:
 *
 * 72 MHz
 *
 * Prescaler:
 *
 * 71
 *
 * Resultado:
 *
 * 72 MHz / 72 = 1 MHz
 *
 * Portanto:
 *
 * 1 tick = 1 us
 *
 *
 * CH1:
 *   INJ1
 *
 * CH2:
 *   INJ2
 *
 * CH3:
 *   RPM / Trigger
 */

#define TIM_RPM                 TIM1

#define TIM_INJECTION           TIM1


#define TIM_INJ1_CHANNEL        TIM_CHANNEL_1

#define TIM_INJ2_CHANNEL        TIM_CHANNEL_2

#define TIM_RPM_CHANNEL         TIM_CHANNEL_3


/*
 * ============================================================
 * TIM4
 * ============================================================
 *
 * Scheduler do loop principal.
 *
 * 100 Hz = 10 ms
 */

#define TIM_LOOP                TIM4


/*
 * ============================================================
 * TIM3
 * ============================================================
 *
 * Reservado para ignição.
 *
 * Futuramente:
 *
 * CH1 = IGN1
 * CH2 = IGN2
 *
 * PB4/PB5 com remapeamento TIM3.
 *
 * NÃO é inicializado nesta etapa.
 */

#define TIM_IGNITION            TIM3

#define TIM_IGN1_CHANNEL        TIM_CHANNEL_1

#define TIM_IGN2_CHANNEL        TIM_CHANNEL_2


/* ============================================================
 * ADC
 * ============================================================ */

#define ADC_INSTANCE            ADC1

#define ADC_CHANNEL_COUNT       5


/*
 * Buffer global do ADC.
 *
 * IMPORTANTE:
 *
 * A variável é definida apenas uma vez em
 * STM32_specific.cpp.
 */
extern volatile uint16_t adc_buffer[ADC_CHANNEL_COUNT];


/*
 * Índices do ADC.
 */

#define ADC_IDX_TPS             0

#define ADC_IDX_MAP             1

#define ADC_IDX_IAT             2

#define ADC_IDX_CLT             3

#define ADC_IDX_O2              4


/* ============================================================
 * UART
 * ============================================================ */

#define UART_TUNER              USART3


/* ============================================================
 * CLOCKS
 * ============================================================ */

#define SYSCLK_FREQ             72000000UL

#define HCLK_FREQ              72000000UL

#define APB1_FREQ              36000000UL

#define APB2_FREQ              72000000UL


/*
 * STM32F103:
 *
 * APB1 = 36 MHz
 *
 * Como o prescaler APB1 é diferente de 1,
 * os timers do APB1 recebem clock de 72 MHz.
 */

#define TIM1_CLOCK_FREQ         72000000UL

#define TIM3_CLOCK_FREQ         72000000UL

#define TIM4_CLOCK_FREQ         72000000UL


/*
 * Clock usado pelo cálculo de RPM.
 */

#define RPM_TIMER_TICK_HZ       1000000UL

#define RPM_TIMER_TICK_US       1UL


/* ============================================================
 * GPIO HELPERS
 * ============================================================ */

#define SET_PIN(port, pin) \
    HAL_GPIO_WritePin( \
        (port), \
        (pin), \
        GPIO_PIN_SET \
    )


#define CLEAR_PIN(port, pin) \
    HAL_GPIO_WritePin( \
        (port), \
        (pin), \
        GPIO_PIN_RESET \
    )


#define READ_PIN(port, pin) \
    HAL_GPIO_ReadPin( \
        (port), \
        (pin) \
    )


/* ============================================================
 * RPM TIMER EXTENSION
 * ============================================================ */

/*
 * TIM1 possui contador de 16 bits.
 *
 * Este contador de software permite acompanhar
 * os overflow do timer.
 */

extern volatile uint32_t tim1_overflow_count;


#ifdef __cplusplus
}
#endif


#endif /* STM32_SPECIFIC_H */