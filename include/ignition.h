#ifndef IGNITION_H
#define IGNITION_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// ============================================================
// IGNITION CHANNEL
// ============================================================

typedef struct
{
    uint8_t enabled;

    uint32_t dwell_us;

    uint32_t spark_count;

    uint32_t last_fire_us;

} IgnitionChannel;


// ============================================================
// IGNITION SYSTEM
// ============================================================

typedef struct
{
    IgnitionChannel ign1;
    IgnitionChannel ign2;

    uint8_t enabled;

    uint8_t mode;

    uint8_t state;

} IgnitionSystem;


// ============================================================
// GLOBAL
// ============================================================

extern IgnitionSystem ignition_system;


// ============================================================
// INITIALIZATION
// ============================================================

void Ignition_Init(void);


// ============================================================
// ENABLE / DISABLE
// ============================================================

void Ignition_Enable(void);

void Ignition_Disable(void);

uint8_t Ignition_IsEnabled(void);


// ============================================================
// PROCESSING
// ============================================================

void Ignition_Process(uint32_t rpm, float load);


// ============================================================
// MANUAL CONTROL
// ============================================================

void Ignition_SetDwell(uint32_t dwell_us);


// ============================================================
// DIAGNOSTICS
// ============================================================

uint32_t Ignition_GetSparkCount(void);


#ifdef __cplusplus
}
#endif

#endif // IGNITION_H