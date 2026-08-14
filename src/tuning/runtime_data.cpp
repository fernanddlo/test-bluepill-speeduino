#include "runtime_data.h"

#include "trigger.h"
#include "sensors.h"
#include "injection.h"
#include "ignition.h"

// ============================================================
// GLOBAL RUNTIME DATA
// ============================================================

volatile RuntimeData runtime_data = {
    .rpm = 0,

    .map_kpa = 0.0f,

    .load = 0.0f,

    .tps = 0.0f,

    .clt = 0.0f,

    .iat = 0.0f,

    .o2 = 0.0f,

    .engine_state = ENGINE_OFF,

    .injection_enabled = 0,

    .fuel_pump_enabled = 0,

    .ignition_enabled = 0,

    .ignition_advance = 0,

    .rpm_state = 0
};


// ============================================================
// INITIALIZATION
// ============================================================

void RuntimeData_Init(void)
{
    runtime_data.rpm = 0;

    runtime_data.map_kpa = 0.0f;

    runtime_data.load = 0.0f;

    runtime_data.tps = 0.0f;

    runtime_data.clt = 0.0f;

    runtime_data.iat = 0.0f;

    runtime_data.o2 = 0.0f;

    runtime_data.engine_state = ENGINE_OFF;

    runtime_data.injection_enabled = 0;

    runtime_data.fuel_pump_enabled = 0;

    runtime_data.ignition_enabled = 0;

    runtime_data.ignition_advance = 0;

    runtime_data.rpm_state = 0;
}


// ============================================================
// UPDATE
// ============================================================

void RuntimeData_Update(
    uint32_t rpm,
    float map_kpa,
    float load,
    float tps,
    EngineState state
)
{
    runtime_data.rpm = rpm;

    runtime_data.map_kpa = map_kpa;

    runtime_data.load = load;

    runtime_data.tps = tps;

    runtime_data.clt = Sensors_GetCLT();

    runtime_data.iat = Sensors_GetIAT();

    runtime_data.o2 = Sensors_GetO2();

    runtime_data.engine_state = state;

    runtime_data.injection_enabled = Injection_IsEnabled();

    runtime_data.ignition_enabled = Ignition_IsEnabled();

    runtime_data.rpm_state = RPM_GetState();
}


// ============================================================
// GETTERS
// ============================================================

uint32_t RuntimeData_GetRPM(void)
{
    return runtime_data.rpm;
}


float RuntimeData_GetMAP(void)
{
    return runtime_data.map_kpa;
}


float RuntimeData_GetLoad(void)
{
    return runtime_data.load;
}


float RuntimeData_GetTPS(void)
{
    return runtime_data.tps;
}


EngineState RuntimeData_GetEngineState(void)
{
    return runtime_data.engine_state;
}
