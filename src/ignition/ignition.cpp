#include "ignition.h"
#include "config.h"
#include "STM32_specific.h"

#include "stm32f1xx_hal.h"


// ============================================================
// GLOBAL
// ============================================================

IgnitionSystem ignition_system =
{
    .ign1 =
    {
        .enabled = 0,
        .dwell_us = 0,
        .spark_count = 0,
        .last_fire_us = 0
    },

    .ign2 =
    {
        .enabled = 0,
        .dwell_us = 0,
        .spark_count = 0,
        .last_fire_us = 0
    },

    .enabled = 0,

    .mode = IGN_DISABLED,

    .state = 0
};


// ============================================================
// INITIALIZATION
// ============================================================

void Ignition_Init(void)
{
    ignition_system.enabled = 0;
    ignition_system.mode = IGN_DISABLED;
    ignition_system.state = 1;

    ignition_system.ign1.enabled = 0;
    ignition_system.ign2.enabled = 0;

    ignition_system.ign1.dwell_us = 0;
    ignition_system.ign2.dwell_us = 0;

    Ignition_Disable();
}


// ============================================================
// ENABLE
// ============================================================

void Ignition_Enable(void)
{
#if IGN_ENABLED

    ignition_system.enabled = 1;

    ignition_system.ign1.enabled = 1;
    ignition_system.ign2.enabled = 1;

    ignition_system.mode = IGN_MODE;

#else

    ignition_system.enabled = 0;

    ignition_system.ign1.enabled = 0;
    ignition_system.ign2.enabled = 0;

    ignition_system.mode = IGN_DISABLED;

#endif
}


// ============================================================
// DISABLE
// ============================================================

void Ignition_Disable(void)
{
    ignition_system.enabled = 0;

    ignition_system.ign1.enabled = 0;
    ignition_system.ign2.enabled = 0;

    ignition_system.mode = IGN_DISABLED;

    ignition_system.ign1.dwell_us = 0;
    ignition_system.ign2.dwell_us = 0;

    // Segurança:
    // mantém as saídas de ignição desligadas.

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_11,
        GPIO_PIN_RESET
    );

    HAL_GPIO_WritePin(
        GPIOA,
        GPIO_PIN_12,
        GPIO_PIN_RESET
    );
}


// ============================================================
// STATUS
// ============================================================

uint8_t Ignition_IsEnabled(void)
{
    return ignition_system.enabled;
}


// ============================================================
// DWELL
// ============================================================

void Ignition_SetDwell(uint32_t dwell_us)
{
    if (dwell_us > 10000)
    {
        dwell_us = 10000;
    }

    ignition_system.ign1.dwell_us = dwell_us;
    ignition_system.ign2.dwell_us = dwell_us;
}


// ============================================================
// PROCESS
// ============================================================

void Ignition_Process(uint32_t rpm, float load)
{
    (void)load;

#if IGN_ENABLED

    if (!ignition_system.enabled)
    {
        return;
    }

    if (rpm == 0)
    {
        Ignition_Disable();
        return;
    }

    /*
     * Ainda não estamos disparando a bobina fisicamente.
     *
     * Esta etapa somente mantém a estrutura do módulo.
     *
     * O disparo real será implementado junto com:
     *
     * 1. trigger position
     * 2. crank angle
     * 3. dwell timing
     * 4. spark timing
     * 5. wasted spark
     *
     * Isso evita gerar faísca em posição incorreta
     * enquanto o decoder de trigger ainda está sendo desenvolvido.
     */

    (void)rpm;

#else

    (void)rpm;

#endif
}


// ============================================================
// DIAGNOSTICS
// ============================================================

uint32_t Ignition_GetSparkCount(void)
{
    return
        ignition_system.ign1.spark_count +
        ignition_system.ign2.spark_count;
}