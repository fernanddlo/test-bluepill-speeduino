#ifndef INJECTION_H
#define INJECTION_H

#include <stdint.h>

// ============ ESTRUTURAS ============
typedef struct {
    uint16_t pulse_width_us;   // Largura do pulso em µs
    uint8_t enabled;           // 1=ativo, 0=desabilitado
    uint32_t injections_total; // Contador de injeções
    uint32_t last_fire_ms;     // Último acionamento
} Injector;

typedef struct {
    Injector inj1;             // Cilindros 1 + 3 (PA8, TIM1_CH1)
    Injector inj2;             // Cilindros 2 + 4 (PA9, TIM1_CH2)
    uint8_t mode;              // SIMULTANEOUS_DUAL, SEQUENTIAL, etc
    uint32_t last_update_ms;
    uint8_t state;             // 0=off, 1=pronto, 2=acionando
} InjectionSystem;

// ============ VARIÁVEL GLOBAL ============
extern InjectionSystem inj_sys;

// ============ FUNÇÕES ============

/**
 * Inicializa o sistema de injeção
 * - Configura TIM1_CH1 (PA8) e TIM1_CH2 (PA9) em modo PWM
 * - Frequência: 2 kHz
 * - Duty cycle inicialmente 0%
 */
void Injection_Init(void);

/**
 * Define a largura do pulso de injeção
 * Aplica ao mesmo tempo a INJ1 e INJ2 (modo simultâneo)
 * @param pw_us Largura em microsegundos (0 a 25000)
 */
void Injection_SetPulseWidth(uint16_t pw_us);

/**
 * Calcula e atualiza o pulso baseado em RPM e carga
 * Busca na tabela VE (mapa)
 * @param rpm RPM atual
 * @param load Carga do motor (0.0 a 1.0, ou TPS%)
 */
void Injection_Process(uint32_t rpm, float load);

/**
 * Desabilita todos os injetores
 * Coloca pulso em 0%
 */
void Injection_Disable(void);

/**
 * Habilita injetores (se motor ligado)
 */
void Injection_Enable(void);

/**
 * Retorna se sistema está habilitado
 * @return 1=sim, 0=não
 */
uint8_t Injection_IsEnabled(void);

/**
 * Retorna total de injeções realizadas
 * (para diagnóstico)
 */
uint32_t Injection_GetTotalCount(void);

#endif // INJECTION_H
