#ifndef TRIGGER_H
#define TRIGGER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// RPM DATA
// ============================================================

typedef struct
{
    volatile uint32_t rpm;
    volatile uint32_t raw_frequency;

    volatile uint32_t last_pulse_time;

    volatile uint8_t state;

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
// INTERRUPT
// ============================================================

void RPM_CAS_ISR(void);


// ============================================================
// BACKGROUND PROCESSING
// ============================================================

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