/**
 * @file main.cpp
 * @brief Minimalist Speeduino-style firmware for STM32F103C8T6
 * @date 2026-08-13
 */

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>
#include "STM32_specific.h"

int main(void) {
    STM32_Init();  // Inicializa tudo de uma vez
    
    while(1) {
        // Loop principal
    }
}
/* ============================================================================ */
/* DECLARATIONS DE TIMERS E UART                                              */
/* ============================================================================ */

TIM_HandleTypeDef htim1;
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart3;

/* ============================================================================ */
/* CLOCK CONFIGURATION (72 MHz)                                               */
/* ============================================================================ */

static void SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* Enable HSE (External oscillator - 8 MHz) */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;  /* 8 MHz * 9 = 72 MHz */
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /* Configure clocks */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;   /* 36 MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;   /* 72 MHz */
    HAL_RCC_ClkConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    /* Update SystemCoreClock */
    SystemCoreClockUpdate();
}

/* ============================================================================ */
/* GPIO INITIALIZATION                                                        */
/* ============================================================================ */

static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* PA8, PA9 - Injection PWM */
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA10 - RPM Input (capture) */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA11, PA12 - Ignition PWM */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA0-PA4 - ADC Inputs */
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | 
                          GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB0 - Fuel Pump Output */
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET);

    /* PB10, PB11 - USART3 (remapped) */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/* ============================================================================ */
/* TIMER 1 INITIALIZATION (PWM + RPM Capture)                                 */
/* ============================================================================ */

static void MX_TIM1_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};
    TIM_IC_InitTypeDef sConfigIC = {0};

    __HAL_RCC_TIM1_CLK_ENABLE();

    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 0;                  /* 72 MHz / 1 */
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 36000 - 1;            /* 36000 ticks @ 2µs = 72ms */
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_OC_Init(&htim1);
    HAL_TIM_PWM_Init(&htim1);
    HAL_TIM_IC_Init(&htim1);

    /* CH1: INJ1 (PA8) - PWM @ 2kHz */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

    /* CH2: INJ2 (PA9) - PWM @ 2kHz */
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);

    /* CH3: RPM Capture (PA10) */
    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_3);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);

    /* CH4: IGN1 (PA11) - PWM @ 5kHz */
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_4);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
}

/* ============================================================================ */
/* ADC1 INITIALIZATION                                                        */
/* ============================================================================ */

static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
    hadc1.Init.ContinuousConvMode = ADC_CONTINUOUS_DISABLE;
    hadc1.Init.DiscontinuousConvMode = ADC_DISCONTINUOUS_ENABLE;
    hadc1.Init.DiscontinuousConvCount = 1;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 5;
    HAL_ADC_Init(&hadc1);

    /* Configure channels PA0-PA4 */
    for (uint32_t ch = 0; ch < 5; ch++) {
        sConfig.Channel = ADC_CHANNEL_0 + ch;
        sConfig.Rank = (ch + 1);
        sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    }
}

/* ============================================================================ */
/* USART3 INITIALIZATION (SERIAL/TUNER STUDIO)                                */
/* ============================================================================ */

static void MX_USART3_Init(void)
{
    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    /* Remap USART3 to PB10/PB11 */
    __HAL_AFIO_REMAP_USART3_ENABLE();

    huart3.Instance = USART3;
    huart3.Init.BaudRate = 115200;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    HAL_UART_Init(&huart3);
}

/* ============================================================================ */
/* INTERRUPT HANDLERS (stubs here, full definitions in stm32f1xx_it.c)        */
/* ============================================================================ */

volatile uint32_t g_rpm = 0;
volatile uint32_t g_last_pulse_time = 0;
volatile uint32_t g_pulse_count = 0;
volatile uint32_t g_loop_tick = 0;
volatile uint8_t g_loop_ready = 0;

/* Main loop scheduler (from SysTick) */
void HAL_SYSTICK_Callback(void)
{
    static uint16_t tick_counter = 0;
    tick_counter++;
    if (tick_counter >= 10) {  /* 1kHz / 10 = 100Hz */
        tick_counter = 0;
        g_loop_tick++;
        g_loop_ready = 1;
    }
}

/* RPM Capture Callback */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
        uint32_t current_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);
        
        g_pulse_count++;
        if (g_pulse_count >= 2) {  /* 2 pulses per rotation */
            uint32_t period = current_time - g_last_pulse_time;
            if (period > 0) {
                /* RPM = (pulses/rotation) * (1000000 us/s) / period * 60 */
                g_rpm = (2 * 1000000 / period) * 60;
            }
            g_pulse_count = 0;
            g_last_pulse_time = current_time;
        }
    }
}

/* ============================================================================ */
/* MAIN LOOP                                                                  */
/* ============================================================================ */

static void Main_Loop(void)
{
    /* Read ADC channels */
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK) {
        uint32_t tps = HAL_ADC_GetValue(&hadc1);    /* PA0 */
        uint32_t map = HAL_ADC_GetValue(&hadc1);    /* PA1 */
        uint32_t iat = HAL_ADC_GetValue(&hadc1);    /* PA2 */
        uint32_t clt = HAL_ADC_GetValue(&hadc1);    /* PA3 */
        uint32_t o2 = HAL_ADC_GetValue(&hadc1);     /* PA4 */

        (void)tps; (void)map; (void)iat; (void)clt; (void)o2;  /* Suppress warnings */
    }

    /* Check engine state */
    uint8_t is_running = (g_rpm >= 500) ? 1 : 0;
        if (is_running) {
        /* Example: Calculate injection pulse width from load */
        float load = (float)HAL_ADC_GetValue(&hadc1) / 4095.0f * 100.0f;
        float ve = 50.0f;  /* Dummy VE value (replace with map lookup) */
        
        /* Pulse width = dwell_time * (VE/100) * (Load/100) */
        float pulse_width_us = 3000.0f * (ve / 100.0f) * (load / 100.0f);  /* in µs */
        
        /* Convert to timer counts (72MHz = 13.9ns per tick) */
        uint32_t pulse_counts = (uint32_t)(pulse_width_us * 72.0f / 1000.0f);
        
        /* Set PWM pulse width (CH1 and CH2 - simultaneous injection) */
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_counts);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_counts);
        
        /* Set ignition advance (fixed 10° during cranking, dynamic during running) */
        float ign_advance = (g_rpm < 500) ? 10.0f : 15.0f;  /* degrees */
        uint32_t ign_counts = (uint32_t)(ign_advance * 1000.0f);  /* placeholder */
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, ign_counts);
        
    } else {
        /* Engine not running - disable outputs */
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    }

    /* Fuel pump control */
    static uint32_t fuel_pump_timeout = 0;
    if (is_running) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET);   /* Enable fuel pump */
        fuel_pump_timeout = g_loop_tick + 200;  /* Timeout after 2s */
    } else if (g_loop_tick >= fuel_pump_timeout) {
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET); /* Disable fuel pump */
    }
}

/* ============================================================================ */
/* MAIN ENTRY POINT                                                           */
/* ============================================================================ */

int main(void)
{
    /* HAL initialization */
    HAL_Init();

    /* Configure system clock (72MHz) */
    SystemClockConfig();

    /* Initialize all peripherals */
    MX_GPIO_Init();
    MX_TIM1_Init();
    MX_ADC1_Init();
    MX_USART3_Init();

    /* Enable interrupts */
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    HAL_NVIC_SetPriority(USART3_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

    /* Send startup message */
    const char *startup_msg = "\r\n=== Speeduino STM32F103 Started ===\r\n";
    HAL_UART_Transmit(&huart3, (uint8_t *)startup_msg, strlen(startup_msg), 100);

    /* Main control loop (100Hz) */
    while (1) {
        if (g_loop_ready) {
            g_loop_ready = 0;
            Main_Loop();
        }
    }

    return 0;
}

/* ============================================================================ */
/* END OF main.cpp                                                            */
/* ============================================================================ */
