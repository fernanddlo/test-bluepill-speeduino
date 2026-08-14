#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// RPM / TRIGGER STATE
// ============================================================

typedef struct
{
    // RPM calculado
    volatile uint32_t rpm;

    // Frequência bruta dos pulsos
    volatile uint32_t raw_frequency;

    // Timestamp do último pulso em ms
    volatile uint32_t last_pulse_time;

    // Estado:
    // 0 = sem sinal
    // 1 = sinal válido
    // 2 = erro
    volatile uint8_t state;

    // Contadores de diagnóstico
    volatile uint32_t pulse_count;
    volatile uint32_t error_count;

} RPM_Data;


// ============================================================
// GLOBAL
// ============================================================

extern RPM_Data rpm_data;


// ============================================================
// INITIALIZATION
// ============================================================

void RPM_Init(void);


// ============================================================
// INTERRUPT SERVICE
// ============================================================

/*
 * Chamado pelo HAL quando ocorre uma captura
 * no TIM1_CH3.
 */
void RPM_CAS_ISR(void);


// ============================================================
// BACKGROUND PROCESSING
// ============================================================

/*
 * Deve ser chamado no loop principal.
 *
 * Responsável principalmente por:
 * - timeout de sinal
 * - estado do trigger
 */
void RPM_Process(void);


// ============================================================
// GETTERS
// ============================================================

uint32_t RPM_Get(void);

uint8_t RPM_GetState(void);


// ============================================================
// RESET
// ============================================================

void RPM_Reset(void);


#ifdef __cplusplus
}
#endif

#endif // TRIGGER_H