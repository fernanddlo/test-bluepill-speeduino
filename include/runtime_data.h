#ifndef RUNTIME_DATA_H
#define RUNTIME_DATA_H

#include <stdint.h>

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================
// RUNTIME DATA
// ============================================================
//
// Dados instantâneos da ECU.
//
// Este módulo funciona como uma interface entre a lógica da ECU
// e módulos externos, principalmente o protocolo TunerStudio.
//
// As variáveis internas dos módulos não devem ser acessadas
// diretamente pelo protocolo serial.
// ============================================================

typedef struct
{
    // --------------------------------------------------------
    // Engine
    // --------------------------------------------------------

    uint32_t rpm;

    float map_kpa;

    float load;

    float tps;

    float clt;

    float iat;

    float o2;


    // --------------------------------------------------------
    // Engine state
    // --------------------------------------------------------

    EngineState engine_state;


    // --------------------------------------------------------
    // Fuel
    // --------------------------------------------------------

    uint8_t injection_enabled;

    uint8_t fuel_pump_enabled;


    // --------------------------------------------------------
    // Ignition
    // --------------------------------------------------------

    uint8_t ignition_enabled;

    uint8_t ignition_advance;


    // --------------------------------------------------------
    // Trigger
    // --------------------------------------------------------

    uint8_t rpm_state;


} RuntimeData;


// ============================================================
// GLOBAL INSTANCE
// ============================================================

extern volatile RuntimeData runtime_data;


// ============================================================
// INITIALIZATION
// ============================================================

void RuntimeData_Init(void);


// ============================================================
// UPDATE
// ============================================================
//
// Atualiza os valores de runtime usados pela ECU.
//

void RuntimeData_Update(
    uint32_t rpm,
    float map_kpa,
    float load,
    float tps,
    EngineState engine_state
);


// ============================================================
// GETTERS
// ============================================================

uint32_t RuntimeData_GetRPM(void);

float RuntimeData_GetMAP(void);

float RuntimeData_GetLoad(void);

float RuntimeData_GetTPS(void);

EngineState RuntimeData_GetEngineState(void);


#ifdef __cplusplus
}
#endif

#endif
