#include "STM32_specific.h"
#include "config.h"

// ============ PERIFÉRICOS GLOBAIS ============
TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim4;
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart3;

// ============ CLOCK CONFIGURATION ============
void SystemClockConfig(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    // Habilita HSE (8 MHz externo) e PLL (8 * 9 = 72 MHz)
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        while(1);
    }

    // Configura divisores de clock
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | 
                                   RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    // HCLK = 72 MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     // APB1 = 36 MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     // APB2 = 72 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        while(1);
    }

    // SysTick: 1ms
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
}

// ============ GPIO CONFIGURATION ============
void GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Habilita clocks dos GPIO
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    // ========== PORTA A ==========
    
    // PA8, PA9: PWM Injeção (TIM1_CH1, TIM1_CH2)
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA10: RPM Input (TIM1_CH3, captura)
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA11, PA12: Ignição saídas (IGN1, IGN2)
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA13: Sensor de comando (cam sensor, entrada - não usado por enquanto)
    GPIO_InitStruct.Pin = GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // PA0-PA4: ADC (TPS, MAP, IAT, CLT, O2)
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ========== PORTA B ==========
    
    // PB0: Bomba de combustível (saída)
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // PB10, PB11: USART3 remapeado (TX, RX)
    __HAL_AFIO_REMAP_USART3_ENABLE();
    
    GPIO_InitStruct.Pin = GPIO_PIN_10;  // TX
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;  // RX
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

// ============ TIMER CONFIGURATION ============
void Timer_Init(void) {
    __HAL_RCC_TIM1_CLK_ENABLE();
    __HAL_RCC_TIM4_CLK_ENABLE();

    // ========== TIM1: PWM + RPM Input ==========
    htim1.Instance = TIM1;
    htim1.Init.Prescaler = 71;  // 72MHz / 72 = 1 MHz (1µs per tick)
    htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim1.Init.Period = 65535;  // Auto-reload
    htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim1.Init.RepetitionCounter = 0;
    htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim1);

    // Configura PWM para CH1 (INJ1) e CH2 (INJ2)
    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;

    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2);

    // Configura captura para CH3 (RPM input)
    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = TIM_ICPOLARITY_RISING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_3);

    // Inicia PWM e captura
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);

    // Habilita interrupções
    HAL_NVIC_SetPriority(TIM1_CC_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_CC_IRQn);
    HAL_NVIC_SetPriority(TIM1_UP_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM1_UP_IRQn);

    // ========== TIM4: Loop timer (opcional) ==========
    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 71;  // 1 MHz
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.Period = 10000;  // 10ms @ 1MHz = 100Hz
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    HAL_TIM_Base_Init(&htim4);

    HAL_TIM_Base_Start_IT(&htim4);
    HAL_NVIC_SetPriority(TIM4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);
}

// ============ ADC CONFIGURATION ============
void ADC_Init(void) {
    __HAL_RCC_ADC1_CLK_ENABLE();

    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = ENABLE;             // Modo varredura
    hadc1.Init.ContinuousConvMode = ENABLE;       // Contínuo
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 5;
    HAL_ADC_Init(&hadc1);

    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.SamplingTime = ADC_SAMPLETIME_55CYCLES_5;

    // CH0: PA0 - TPS
    sConfig.Rank = ADC_REGULAR_RANK_1;
    sConfig.Channel = ADC_CHANNEL_0;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // CH1: PA1 - MAP
    sConfig.Rank = ADC_REGULAR_RANK_2;
    sConfig.Channel = ADC_CHANNEL_1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // CH2: PA2 - IAT
    sConfig.Rank = ADC_REGULAR_RANK_3;
    sConfig.Channel = ADC_CHANNEL_2;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // CH3: PA3 - CLT
    sConfig.Rank = ADC_REGULAR_RANK_4;
    sConfig.Channel = ADC_CHANNEL_3;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    // CH4: PA4 - O2
    sConfig.Rank = ADC_REGULAR_RANK_5;
    sConfig.Channel = ADC_CHANNEL_4;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
}

// ============ UART CONFIGURATION ============
// ============ UART CONFIGURATION ============
void UART_Init(void) {
    __HAL_RCC_USART3_CLK_ENABLE();

    huart3.Instance = USART3;
    huart3.Init.BaudRate = SERIAL_BAUD;              // 115200
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart3) != HAL_OK) {
        while(1);
    }

    // Habilita interrupção RX
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

// ============ MASTER INIT FUNCTION ============
void STM32_Init(void) {
    HAL_Init();
    SystemClockConfig();
    GPIO_Init();
    ADC_Init();
    Timer_Init();
    UART_Init();
}
