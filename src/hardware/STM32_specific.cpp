#include "STM32_specific.h"
#include "config.h"
#include "trigger.h"

#include <stdint.h>


/* ============================================================
 * GLOBAL HAL HANDLES
 * ============================================================ */

TIM_HandleTypeDef htim1;

TIM_HandleTypeDef htim4;

ADC_HandleTypeDef hadc1;

UART_HandleTypeDef huart3;


/* ============================================================
 * GLOBAL ADC BUFFER
 * ============================================================ */

/*
 * PA0 = TPS
 * PA1 = MAP
 * PA2 = IAT
 * PA3 = CLT
 * PA4 = O2
 *
 * IMPORTANTE:
 *
 * Existe somente UMA definição deste buffer.
 *
 * Os demais arquivos utilizam o extern declarado
 * em STM32_specific.h.
 */

volatile uint16_t adc_buffer[ADC_CHANNEL_COUNT] = {0};


/* ============================================================
 * TIM1 SOFTWARE OVERFLOW COUNTER
 * ============================================================ */

volatile uint32_t tim1_overflow_count = 0;


/* ============================================================
 * EXTERNAL LOOP FLAG
 * ============================================================ */

extern volatile uint8_t ecu_10ms_flag;


/* ============================================================
 * FATAL ERROR
 * ============================================================ */

void STM32_FatalError(void)
{
    /*
     * Em qualquer falha crítica:
     *
     * combustível OFF
     * injetores OFF
     * ignição OFF
     */

    /* Fuel pump */
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0,
        GPIO_PIN_RESET
    );


    /* Injection */
    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_RESET
    );


    /*
     * Ignition
     *
     * PB4 / PB5
     *
     * PA11/PA12 NÃO são tocados.
     *
     * Eles permanecem livres para USB.
     */
    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_4 | GPIO_PIN_5,
        GPIO_PIN_RESET
    );


    __disable_irq();


    while (1)
    {
        /*
         * Fatal error.
         */
    }
}


/* ============================================================
 * CLOCK CONFIGURATION
 * ============================================================ */

void SystemClockConfig(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};

    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};


    /*
     * ========================================================
     * HSE = 8 MHz
     *
     * PLL = 8 MHz x 9
     *
     * SYSCLK = 72 MHz
     * ========================================================
     */

    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSE;

    RCC_OscInitStruct.HSEState =
        RCC_HSE_ON;

    RCC_OscInitStruct.HSEPredivValue =
        RCC_HSE_PREDIV_DIV1;


    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_ON;

    RCC_OscInitStruct.PLL.PLLSource =
        RCC_PLLSOURCE_HSE;

    RCC_OscInitStruct.PLL.PLLMUL =
        RCC_PLL_MUL9;


    if (HAL_RCC_OscConfig(
            &RCC_OscInitStruct
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * BUS CLOCKS
     * ========================================================
     */

    RCC_ClkInitStruct.ClockType =
        RCC_CLOCKTYPE_HCLK |
        RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 |
        RCC_CLOCKTYPE_PCLK2;


    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_PLLCLK;


    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;


    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV2;


    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_2
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * SysTick = 1 ms
     * ========================================================
     */

    HAL_SYSTICK_Config(
        HAL_RCC_GetHCLKFreq() / 1000UL
    );


    HAL_SYSTICK_CLKSourceConfig(
        SYSTICK_CLKSOURCE_HCLK
    );
}


/* ============================================================
 * GPIO CONFIGURATION
 * ============================================================ */

void GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};


    /*
     * ========================================================
     * GPIO CLOCKS
     * ========================================================
     */

    __HAL_RCC_GPIOA_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    __HAL_RCC_AFIO_CLK_ENABLE();


    /*
     * ========================================================
     * JTAG / SWD
     * ========================================================
     *
     * Desabilita JTAG.
     *
     * Mantém SWD:
     *
     * PA13 = SWDIO
     * PA14 = SWCLK
     *
     * Libera:
     *
     * PB3
     * PB4
     * PA15
     *
     * PB4 será utilizado futuramente pela IGN1.
     */

    __HAL_AFIO_REMAP_SWJ_NOJTAG();


    /*
     * ========================================================
     * ADC INPUTS
     * ========================================================
     *
     * PA0 = TPS
     * PA1 = MAP
     * PA2 = IAT
     * PA3 = CLT
     * PA4 = O2
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_0 |
        GPIO_PIN_1 |
        GPIO_PIN_2 |
        GPIO_PIN_3 |
        GPIO_PIN_4;


    GPIO_InitStruct.Mode =
        GPIO_MODE_ANALOG;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );


    /*
     * ========================================================
     * INJECTION OUTPUTS
     * ========================================================
     *
     * PA8 = INJ1
     * PA9 = INJ2
     *
     * TIM1_CH1 / TIM1_CH2
     *
     * Nesta etapa os canais não são iniciados.
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_8 |
        GPIO_PIN_9;


    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_PP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );


    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_8 | GPIO_PIN_9,
        GPIO_PIN_RESET
    );


    /*
     * ========================================================
     * RPM / TRIGGER INPUT
     * ========================================================
     *
     * PA10 = TIM1_CH3
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_10;


    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;


    GPIO_InitStruct.Pull =
        GPIO_PULLDOWN;


    HAL_GPIO_Init(
        GPIOA,
        &GPIO_InitStruct
    );


    /*
     * ========================================================
     * PA11 / PA12
     * ========================================================
     *
     * NÃO CONFIGURAR.
     *
     * PA11 = USB D-
     * PA12 = USB D+
     *
     * Mantemos esses pinos livres.
     */


    /*
     * ========================================================
     * IGNITION OUTPUTS
     * ========================================================
     *
     * PB4 = IGN1
     * PB5 = IGN2
     *
     * IMPORTANTE:
     *
     * A ignição está desabilitada.
     *
     * Portanto os pinos ficam como GPIO de saída
     * mantidos LOW.
     *
     * O remapeamento TIM3 será implementado
     * na etapa de ignição.
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_4 |
        GPIO_PIN_5;


    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOB,
        &GPIO_InitStruct
    );


    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_4 | GPIO_PIN_5,
        GPIO_PIN_RESET
    );


    /*
     * ========================================================
     * FUEL PUMP
     * ========================================================
     *
     * PB0
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_0;


    GPIO_InitStruct.Mode =
        GPIO_MODE_OUTPUT_PP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOB,
        &GPIO_InitStruct
    );


    HAL_GPIO_WritePin(
        GPIOB,
        GPIO_PIN_0,
        GPIO_PIN_RESET
    );


    /*
     * ========================================================
     * USART3 REMAP
     * ========================================================
     *
     * PB10 = TX
     * PB11 = RX
     */

    __HAL_AFIO_REMAP_USART3_ENABLE();


    /*
     * TX
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_10;


    GPIO_InitStruct.Mode =
        GPIO_MODE_AF_PP;


    GPIO_InitStruct.Speed =
        GPIO_SPEED_FREQ_HIGH;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOB,
        &GPIO_InitStruct
    );


    /*
     * RX
     */

    GPIO_InitStruct.Pin =
        GPIO_PIN_11;


    GPIO_InitStruct.Mode =
        GPIO_MODE_INPUT;


    GPIO_InitStruct.Pull =
        GPIO_NOPULL;


    HAL_GPIO_Init(
        GPIOB,
        &GPIO_InitStruct
    );
}


/* ============================================================
 * ADC CONFIGURATION
 * ============================================================ */

void ADC_Init(void)
{
    /*
     * ADC1 clock
     */

    __HAL_RCC_ADC1_CLK_ENABLE();


    /*
     * ========================================================
     * ADC CLOCK
     * ========================================================
     *
     * APB2 = 72 MHz
     *
     * ADC clock:
     *
     * 72 / 6 = 12 MHz
     *
     * Dentro do limite do ADC do STM32F103.
     */

    RCC->CFGR &= ~RCC_CFGR_ADCPRE;

    RCC->CFGR |= RCC_CFGR_ADCPRE_DIV6;


    /*
     * ========================================================
     * ADC CONFIG
     * ========================================================
     */

    hadc1.Instance =
        ADC1;


    hadc1.Init.ScanConvMode =
        ENABLE;


    hadc1.Init.ContinuousConvMode =
        ENABLE;


    hadc1.Init.DiscontinuousConvMode =
        DISABLE;


    hadc1.Init.ExternalTrigConv =
        ADC_SOFTWARE_START;


    hadc1.Init.DataAlign =
        ADC_DATAALIGN_RIGHT;


    hadc1.Init.NbrOfConversion =
        ADC_CHANNEL_COUNT;


    if (HAL_ADC_Init(
            &hadc1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    ADC_ChannelConfTypeDef sConfig = {0};


    sConfig.SamplingTime =
        ADC_SAMPLETIME_55CYCLES_5;


    /*
     * ========================================================
     * TPS
     * ========================================================
     */

    sConfig.Channel =
        ADC_CHANNEL_0;

    sConfig.Rank =
        ADC_REGULAR_RANK_1;


    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * MAP
     * ========================================================
     */

    sConfig.Channel =
        ADC_CHANNEL_1;

    sConfig.Rank =
        ADC_REGULAR_RANK_2;


    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * IAT
     * ========================================================
     */

    sConfig.Channel =
        ADC_CHANNEL_2;

    sConfig.Rank =
        ADC_REGULAR_RANK_3;


    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * CLT
     * ========================================================
     */

    sConfig.Channel =
        ADC_CHANNEL_3;

    sConfig.Rank =
        ADC_REGULAR_RANK_4;


    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * O2
     * ========================================================
     */

    sConfig.Channel =
        ADC_CHANNEL_4;

    sConfig.Rank =
        ADC_REGULAR_RANK_5;


    if (HAL_ADC_ConfigChannel(
            &hadc1,
            &sConfig
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * CALIBRATION
     * ========================================================
     */

    if (HAL_ADCEx_Calibration_Start(
            &hadc1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * START ADC
     * ========================================================
     */

    if (HAL_ADC_Start(
            &hadc1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }
}


/* ============================================================
 * TIMER CONFIGURATION
 * ============================================================ */

/*
 * ============================================================
 * TIM1
 * ============================================================
 *
 * Clock:
 *
 * 72 MHz
 *
 * Prescaler:
 *
 * 71
 *
 * Timer:
 *
 * 1 MHz
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
 *   RPM
 *
 *
 * IMPORTANTE:
 *
 * INJ1/INJ2 não são iniciados nesta etapa.
 *
 * A futura implementação deverá transformar
 * esses canais em scheduler de eventos.
 *
 *
 * ============================================================
 *
 * TIM4
 *
 * 100 Hz
 *
 * 10 ms
 */

void Timer_Init(void)
{
    TIM_OC_InitTypeDef sConfigOC = {0};

    TIM_IC_InitTypeDef sConfigIC = {0};


    /*
     * ========================================================
     * TIMER CLOCKS
     * ========================================================
     */

    __HAL_RCC_TIM1_CLK_ENABLE();

    __HAL_RCC_TIM4_CLK_ENABLE();


    /*
     * ========================================================
     * TIM1 BASE
     * ========================================================
     */

    htim1.Instance =
        TIM1;


    htim1.Init.Prescaler =
        71;


    htim1.Init.CounterMode =
        TIM_COUNTERMODE_UP;


    htim1.Init.Period =
        65535;


    htim1.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;


    htim1.Init.RepetitionCounter =
        0;


    htim1.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;


    if (HAL_TIM_Base_Init(
            &htim1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * TIM1 CH1 - INJ1
     * ========================================================
     */

    sConfigOC.OCMode =
        TIM_OCMODE_PWM1;


    sConfigOC.Pulse =
        0;


    sConfigOC.OCPolarity =
        TIM_OCPOLARITY_HIGH;


    sConfigOC.OCFastMode =
        TIM_OCFAST_DISABLE;


    if (HAL_TIM_PWM_ConfigChannel(
            &htim1,
            &sConfigOC,
            TIM_CHANNEL_1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * TIM1 CH2 - INJ2
     * ========================================================
     */

    if (HAL_TIM_PWM_ConfigChannel(
            &htim1,
            &sConfigOC,
            TIM_CHANNEL_2
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * TIM1 CH3 - RPM INPUT CAPTURE
     * ========================================================
     */

    sConfigIC.ICPolarity =
        TIM_ICPOLARITY_RISING;


    sConfigIC.ICSelection =
        TIM_ICSELECTION_DIRECTTI;


    sConfigIC.ICPrescaler =
        TIM_ICPSC_DIV1;


    /*
     * Pequeno filtro digital.
     *
     * Será ajustado conforme o condicionamento
     * do sinal do distribuidor.
     */

    sConfigIC.ICFilter =
        2;


    if (HAL_TIM_IC_ConfigChannel(
            &htim1,
            &sConfigIC,
            TIM_CHANNEL_3
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * START TIM1 BASE
     * ========================================================
     */

    if (HAL_TIM_Base_Start(
            &htim1
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * RPM INPUT CAPTURE INTERRUPT
     * ========================================================
     */

    if (HAL_TIM_IC_Start_IT(
            &htim1,
            TIM_CHANNEL_3
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * ========================================================
     * TIM1 INTERRUPTS
     * ========================================================
     */

    HAL_NVIC_SetPriority(
        TIM1_CC_IRQn,
        1,
        0
    );


    HAL_NVIC_EnableIRQ(
        TIM1_CC_IRQn
    );


    HAL_NVIC_SetPriority(
        TIM1_UP_IRQn,
        1,
        1
    );


    HAL_NVIC_EnableIRQ(
        TIM1_UP_IRQn
    );


    /*
     * ========================================================
     * TIM4 - 100 Hz
     * ========================================================
     *
     * 72 MHz / 72 = 1 MHz
     *
     * 1 MHz / 10000 = 100 Hz
     *
     * Portanto:
     *
     * 1 interrupção a cada 10 ms.
     */

    htim4.Instance =
        TIM4;


    htim4.Init.Prescaler =
        71;


    htim4.Init.CounterMode =
        TIM_COUNTERMODE_UP;


    htim4.Init.Period =
        9999;


    htim4.Init.ClockDivision =
        TIM_CLOCKDIVISION_DIV1;


    htim4.Init.AutoReloadPreload =
        TIM_AUTORELOAD_PRELOAD_DISABLE;


    if (HAL_TIM_Base_Init(
            &htim4
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    if (HAL_TIM_Base_Start_IT(
            &htim4
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    HAL_NVIC_SetPriority(
        TIM4_IRQn,
        2,
        0
    );


    HAL_NVIC_EnableIRQ(
        TIM4_IRQn
    );
}


/* ============================================================
 * USART3 CONFIGURATION
 * ============================================================ */

/*
 * PCLK1 = 36 MHz
 *
 * USART3 utiliza PCLK1.
 *
 * PB10 = TX
 * PB11 = RX
 */

void UART_Init(void)
{
    /*
     * USART3 clock
     */

    __HAL_RCC_USART3_CLK_ENABLE();


    /*
     * ========================================================
     * USART3
     * ========================================================
     */

    huart3.Instance =
        USART3;


    huart3.Init.BaudRate =
        SERIAL_BAUD;


    huart3.Init.WordLength =
        UART_WORDLENGTH_8B;


    huart3.Init.StopBits =
        UART_STOPBITS_1;


    huart3.Init.Parity =
        UART_PARITY_NONE;


    huart3.Init.Mode =
        UART_MODE_TX_RX;


    huart3.Init.HwFlowCtl =
        UART_HWCONTROL_NONE;


    huart3.Init.OverSampling =
        UART_OVERSAMPLING_16;


    if (HAL_UART_Init(
            &huart3
        ) != HAL_OK)
    {
        STM32_FatalError();
    }


    /*
     * USART3 interrupt
     */

    HAL_NVIC_SetPriority(
        USART3_IRQn,
        3,
        0
    );


    HAL_NVIC_EnableIRQ(
        USART3_IRQn
    );
}


/* ============================================================
 * TIM1 INPUT CAPTURE CALLBACK
 * ============================================================ */

void HAL_TIM_IC_CaptureCallback(
    TIM_HandleTypeDef *htim
)
{
    if (
        htim->Instance == TIM1 &&
        htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3
    )
    {
                /*
         * Encaminha a captura do TIM1_CH3 para
         * o módulo trigger/RPM.
         */
        RPM_CAS_ISR();
    }
}


/* ============================================================
 * TIMER PERIOD CALLBACK
 * ============================================================ */

void HAL_TIM_PeriodElapsedCallback(
    TIM_HandleTypeDef *htim
)
{
    /*
     * ========================================================
     * TIM1 OVERFLOW
     * ========================================================
     */

    if (htim->Instance == TIM1)
    {
        tim1_overflow_count++;
    }


    /*
     * ========================================================
     * TIM4 - 100 Hz
     * ========================================================
     *
     * Apenas marca o evento.
     *
     * O processamento permanece fora da ISR.
     */

    if (htim->Instance == TIM4)
    {
        ecu_10ms_flag = 1;
    }
}


/* ============================================================
 * MASTER INITIALIZATION
 * ============================================================ */

void STM32_Init(void)
{
    /*
     * HAL
     */

    HAL_Init();


    /*
     * Clock
     */

    SystemClockConfig();


    /*
     * GPIO
     */

    GPIO_Init();


    /*
     * ADC
     */

    ADC_Init();


    /*
     * Timers
     */

    Timer_Init();


    /*
     * USART3
     */

    UART_Init();
}