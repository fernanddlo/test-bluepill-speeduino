#ifndef IGNITION_H
#define IGNITION_H

#include <stdint.h>

void Ignition_Init(void)
{
    Ignition_Disable();
};

void Ignition_Enable(void)
{

};

void Ignition_Disable(void)
{

};

void Ignition_Process(
    uint32_t rpm,
    float load
);

void Ignition_SetAdvance(
    float advance_deg
);

void Ignition_SetDwell(
    float dwell_ms
);

uint8_t Ignition_IsEnabled(void);

#endif