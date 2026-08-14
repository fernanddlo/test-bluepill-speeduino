#include "trigger.h"
#include "config.h"
#include "STM32_specific.h"
#include "stm32f1xx_hal.h"

// ============ VARIÁVEL GLOBAL ============
RPM_Data rpm_data = {
    .rpm = 0,
    .raw_frequency = 0,
    .last_pulse_time = 0,
    .state = 0,
    .pulse_count = 0,
    .error_count = 0
};

// ============ VARIÁVEIS INTERNAS ============
static uint32_t last_capture_time = 0;
static uint32_t capture_period_us = 0;

// ============ INICIALIZAÇÃO ============
void RPM_Init(void) {
    // TIM1_CH3 já configurado em Timer_Init() (STM32_specific.c)
    // Aqui apenas inicializa estrutura
    rpm_data.state = 0;
    rpm_data.rpm = 0;
}

// ============ ISR - CHAMADO A CADA PULSO ============
void RPM_CAS_ISR(void) {
    // Lê valor capturado do timer
    uint32_t capture_value = HAL_TIM_ReadCapturedValue(&htim1, TIM_CHANNEL_3);
    
    // Primeira captura - apenas armazena
    if (last_capture_time == 0) {
        last_capture_time = capture_value;
        return;
    }
    
    // Calcula diferença (período entre pulsos em µs)
    // Prescaler = 71 (SYSCLK 72 MHz / 72 = 1 µs por tick)
    if (capture_value >= last_capture_time) {
        capture_period_us = capture_value - last_capture_time;
    } else {
        // Overflow do timer
        capture_period_us = (0xFFFF - last_capture_time) + capture_value;
    }
    
    last_capture_time = capture_value;
    
    // Validação básica (período entre 1 ms e 200 ms = 300 a 60000 RPM)
    if (capture_period_us < 1000 || capture_period_us > 200000) {
        rpm_data.error_count++;
        rpm_data.state = 2;  // Erro
        return;
    }
    
    // Calcula frequência (pulsos por segundo)
    // Um período = 2 pulsos por rotação
    // RPM = (freq * 60) / CAS_PULSES_PER_ROTATION
    // Freq em Hz = 1,000,000 µs / período µs
    uint32_t freq_hz = 1000000 / capture_period_us;
    rpm_data.raw_frequency = freq_hz;
    
    // Calcula RPM
    rpm_data.rpm = (freq_hz * 60) / CAS_PULSES_PER_ROTATION;
    
    // Atualiza timestamp
    rpm_data.last_pulse_time = HAL_GetTick();
    
    // Estado OK
    rpm_data.state = 1;
    rpm_data.pulse_count++;
}

// ============ PROCESSAMENTO EM BACKGROUND ============
void RPM_Process(void) {
    uint32_t now_ms = HAL_GetTick();
    
    // Detecta timeout (sem pulso por mais de RPM_NO_SIGNAL_TIMEOUT_MS)
    if (rpm_data.last_pulse_time > 0) {
        uint32_t elapsed_ms = now_ms - rpm_data.last_pulse_time;
        if (elapsed_ms > RPM_NO_SIGNAL_TIMEOUT_MS) {
            rpm_data.rpm = 0;
            rpm_data.state = 0;  // Sem sinal
        }
    }
}

// ============ GETTERS ============
uint32_t RPM_Get(void) {
    return rpm_data.rpm;
}

uint8_t RPM_GetState(void) {
    return rpm_data.state;  // 0=offline, 1=ok, 2=erro
}

// ============ RESET ============
void RPM_Reset(void) {
    rpm_data.pulse_count = 0;
    rpm_data.error_count = 0;
    last_capture_time = 0;
    rpm_data.rpm = 0;
}