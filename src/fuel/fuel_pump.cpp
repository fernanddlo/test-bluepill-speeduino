#include "fuel_pump.h"
#include "config.h"
#include "STM32_specific.h"
#include "stm32f1xx_hal.h"

// ============ VARIÁVEL GLOBAL ============
FuelPump fuel_pump = {
    .state = 0,
    .priming_ms_left = 0,
    .no_rpm_timeout_ms = FUEL_PUMP_DISABLE_TIME_MS,
    .last_enable_ms = 0,
    .pump_on_time = 0
};

// ============ INICIALIZAÇÃO ============
void FuelPump_Init(void) {
    // PB0 configurado como saída em GPIO_Init()
    CLEAR_PIN(GPIOB, GPIO_PIN_0);
    fuel_pump.state = 0;
}

// ============ PROCESSAMENTO (LÓGICA PRINCIPAL) ============
void FuelPump_Process(uint32_t rpm) {
    static uint32_t last_process_ms = 0;
    uint32_t now_ms = HAL_GetTick();
    
    // Atualiza timeout sem RPM
    if (rpm > 0) {
        fuel_pump.no_rpm_timeout_ms = 0;
        fuel_pump.last_enable_ms = now_ms;
    } else {
        fuel_pump.no_rpm_timeout_ms = now_ms - fuel_pump.last_enable_ms;
    }
    
    // ===== ESTADO: OFF =====
    if (fuel_pump.state == 0) {
        // Espera primeiro comando de RPM
        if (rpm > MIN_RPM_FOR_RUNNING) {
            FuelPump_Prime();  // Inicia priming
        }
        return;
    }
    
    // ===== ESTADO: PRIMING =====
    if (fuel_pump.state == 1) {
        // Bomba ligada
        SET_PIN(GPIOB, GPIO_PIN_0);
        
        // Decrementa timer
        if ((now_ms - last_process_ms) >= 10) {
            last_process_ms = now_ms;
            if (fuel_pump.priming_ms_left > 0) {
                fuel_pump.priming_ms_left -= 10;
            }
        }
        
        // Fim do priming
        if (fuel_pump.priming_ms_left == 0) {
            fuel_pump.state = 2;  // Passa para RUNNING
        }
        return;
    }
    
    // ===== ESTADO: RUNNING =====
    if (fuel_pump.state == 2) {
        // Mantém bomba ligada
        SET_PIN(GPIOB, GPIO_PIN_0);
        fuel_pump.pump_on_time++;
        
        // Se timeout sem RPM, desliga
        if (fuel_pump.no_rpm_timeout_ms > FUEL_PUMP_DISABLE_TIME_MS) {
            FuelPump_Disable();
        }
        return;
    }
}

// ============ LIGA BOMBA MANUALMENTE ============
void FuelPump_Enable(void) {
    SET_PIN(GPIOB, GPIO_PIN_0);
    fuel_pump.state = 2;
    fuel_pump.last_enable_ms = HAL_GetTick();
}

// ============ DESLIGA BOMBA ============
void FuelPump_Disable(void) {
    CLEAR_PIN(GPIOB, GPIO_PIN_0);
    fuel_pump.state = 0;
    fuel_pump.priming_ms_left = 0;
    fuel_pump.pump_on_time = 0;
}

// ============ INICIA PRIMING ============
void FuelPump_Prime(void) {
    fuel_pump.state = 1;
    fuel_pump.priming_ms_left = FUEL_PUMP_PRIME_TIME_MS;
    fuel_pump.last_enable_ms = HAL_GetTick();
}

// ============ GETTERS ============
uint8_t FuelPump_GetState(void) {
    return fuel_pump.state;
}
