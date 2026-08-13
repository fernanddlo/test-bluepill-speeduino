#include "injection.h"
#include "config.h"
#include "STM32_specific.h"
#include "maps.h"
#include "eeprom.h"
#include "stm32f1xx_hal.h"

// ============ VARIÁVEL GLOBAL ============
InjectionSystem inj_sys = {
    .inj1 = {.pulse_width_us = 0, .enabled = 0, .injections_total = 0, .last_fire_ms = 0},
    .inj2 = {.pulse_width_us = 0, .enabled = 0, .injections_total = 0, .last_fire_ms = 0},
    .mode = SIMULTANEOUS_DUAL,
    .last_update_ms = 0,
    .state = 0
};

// ============ INICIALIZAÇÃO ============
void Injection_Init(void) {
    // TIM1_CH1 (PA8) e TIM1_CH2 (PA9) já configurados em Timer_Init()
    // Aqui apenas inicializa estrutura
    inj_sys.state = 1;  // Pronto
    Injection_Disable();
}

// ============ CONFIGURA LARGURA DE PULSO ============
void Injection_SetPulseWidth(uint16_t pw_us) {
    // Limita valor máximo
    if (pw_us > (MAX_PULSE_WIDTH_MS * 1000)) {
        pw_us = MAX_PULSE_WIDTH_MS * 1000;
    }
    
    // Converte µs para duty cycle (0 a 999, para 2 kHz)
    // Período PWM = 1 ms / 2 kHz = 500 µs
    // CCR = (pw_us / 500) * 1000
    // Mas usamos ARR=999 e prescaler=35, então:
    // T_período = (1000 * 36) / 72MHz = 500 µs
    // CCR = (pw_us * 2) mas isso é aproximado
    
    uint16_t ccr_value = (pw_us * 2) / 1000;  // Conversão de µs para CCR
    if (ccr_value > 999) ccr_value = 999;
    
    // Aplica ao TIM1_CH1 (INJ1)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr_value);
    
    // Aplica ao TIM1_CH2 (INJ2)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr_value);
    
    // Armazena valor
    inj_sys.inj1.pulse_width_us = pw_us;
    inj_sys.inj2.pulse_width_us = pw_us;
    inj_sys.last_update_ms = HAL_GetTick();
}

// ============ PROCESSAMENTO (BUSCA NA TABELA) ============
void Injection_Process(uint32_t rpm, float load) {
    if (!inj_sys.inj1.enabled || !inj_sys.inj2.enabled) {
        return;
    }
    
    // Busca valor na tabela VE com interpolação
    float ve_value = Maps_GetInterpolated(&ve_map_default, rpm, load);
    
    // Calcula largura de pulso baseado em VE
    // Simplificado: pw = (ve_value / 100.0) * BASE_PW
    // Onde BASE_PW é um valor de referência
    uint16_t base_pw_us = 3000;  // 3 ms base (ajustar conforme necessário)
    uint16_t calculated_pw = (uint16_t)((ve_value / 100.0f) * base_pw_us);
    
    // Aplica mínimo
    if (calculated_pw < (MIN_PULSE_WIDTH_MS * 1000)) {
        calculated_pw = MIN_PULSE_WIDTH_MS * 1000;
    }
    
    // Define pulso
    Injection_SetPulseWidth(calculated_pw);
}

// ============ DESABILITA INJETORES ============
void Injection_Disable(void) {
    Injection_SetPulseWidth(0);
    inj_sys.inj1.enabled = 0;
    inj_sys.inj2.enabled = 0;
}

// ============ HABILITA INJETORES ============
void Injection_Enable(void) {
    inj_sys.inj1.enabled = 1;
    inj_sys.inj2.enabled = 1;
}

// ============ GETTERS ============
uint8_t Injection_IsEnabled(void) {
    return (inj_sys.inj1.enabled && inj_sys.inj2.enabled);
}

uint32_t Injection_GetTotalCount(void) {
    return inj_sys.inj1.injections_total;
}
