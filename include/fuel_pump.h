#ifndef FUEL_PUMP_H
#define FUEL_PUMP_H

#include <stdint.h>

// ============ ESTRUTURA ============
typedef struct {
    uint8_t state;             // 0=off, 1=priming, 2=running
    uint32_t priming_ms_left;  // Tempo restante de priming
    uint32_t no_rpm_timeout_ms;// Timeout sem RPM
    uint32_t last_enable_ms;   // Último acionamento
    uint32_t pump_on_time;     // Tempo total ligada (debug)
} FuelPump;

// ============ VARIÁVEL GLOBAL ============
extern FuelPump fuel_pump;

// ============ FUNÇÕES ============

/**
 * Inicializa o controle da bomba de combustível
 * - Configura PB0 como saída (GPIO)
 * - Priming desabilitado até primeira ignição
 */
void FuelPump_Init(void);

/**
 * Processa lógica da bomba (deve ser chamado no loop principal)
 * - Ligada: RPM > MIN_RPM_FOR_RUNNING
 * - Priming: 500 ms ao ligar
 * - Timeout: Desliga após 2s sem RPM
 * @param rpm RPM atual
 */
void FuelPump_Process(uint32_t rpm);

/**
 * Liga a bomba manualmente
 */
void FuelPump_Enable(void);

/**
 * Desliga a bomba manualmente
 */
void FuelPump_Disable(void);

/**
 * Retorna estado atual
 * @return 0=desligada, 1=priming, 2=rodando
 */
uint8_t FuelPump_GetState(void);

/**
 * Inicia sequência de priming
 * Chamado ao ligar o carro
 */
void FuelPump_Prime(void);

#endif // FUEL_PUMP_H
