#ifndef RPM_H
#define RPM_H

#include <stdint.h>

// ============ ESTRUTURA DE DADOS ============
typedef struct {
    uint32_t rpm;              // RPM atual
    uint32_t raw_frequency;    // Frequência em Hz
    uint32_t last_pulse_time;  // Timestamp último pulso (µs)
    uint8_t state;             // 0=sem sinal, 1=ok, 2=erro
    uint32_t pulse_count;      // Contador de pulsos
    uint32_t error_count;      // Erros detectados
} RPM_Data;

// ============ VARIÁVEL GLOBAL ============
extern RPM_Data rpm_data;

// ============ FUNÇÕES ============

/**
 * Inicializa o timer de captura do CAS (RPM)
 * - Configura TIM1_CH3 (PA10) para captura de borda ascendente
 * - Prescaler para resolução de 1 µs
 * - Ativa interrupção
 */
void RPM_Init(void);

/**
 * Handler da interrupção do CAS
 * Chamado a cada pulso detectado em PA10
 * Calcula RPM baseado no tempo entre pulsos
 */
void RPM_CAS_ISR(void);

/**
 * Retorna o RPM atual
 * @return RPM (0 se sem sinal)
 */
uint32_t RPM_Get(void);

/**
 * Retorna o estado do sinal CAS
 * @return 0=offline, 1=ok, 2=erro
 */
uint8_t RPM_GetState(void);

/**
 * Processamento em background
 * - Detecção de timeout (sem sinal > 1s)
 * - Validação de sinal
 * - Log de erros
 */
void RPM_Process(void);

/**
 * Reset do contador (para testes)
 */
void RPM_Reset(void);

#endif // RPM_H
