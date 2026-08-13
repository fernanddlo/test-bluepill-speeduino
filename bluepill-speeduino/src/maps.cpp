#include "maps.h"
#include "config.h"
#include <math.h>

// ============ MAPAS DEFAULT ============
const MapTable ve_map_default = {
    .rpm_bins = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000},
    .load_bins = {0, 12, 25, 37, 50, 62, 75, 87},  // Percentuais aprox
    .table = {
        {80,  85,  90,  95,  100, 105, 110, 115},
        {82,  87,  92,  97,  102, 107, 112, 117},
        {84,  89,  94,  99,  104, 109, 114, 119},
        {86,  91,  96,  101, 106, 111, 116, 121},
        {88,  93,  98,  103, 108, 113, 118, 123},
        {90,  95,  100, 105, 110, 115, 120, 125},
        {92,  97,  102, 107, 112, 117, 122, 127},
        {94,  99,  104, 109, 114, 119, 124, 129}
    }
};

const MapTable ign_map_default = {
    .rpm_bins = {500, 1000, 1500, 2000, 2500, 3000, 3500, 4000},
    .load_bins = {0, 12, 25, 37, 50, 62, 75, 87},
    .table = {
        {5,   6,   8,   10,  12,  14,  16,  18},
        {6,   7,   9,   11,  13,  15,  17,  19},
        {7,   8,   10,  12,  14,  16,  18,  20},
        {8,   9,   11,  13,  15,  17,  19,  21},
        {9,   10,  12,  14,  16,  18,  20,  22},
        {10,  11,  13,  15,  17,  19,  21,  23},
        {11,  12,  14,  16,  18,  20,  22,  24},
        {12,  13,  15,  17,  19,  21,  23,  25}
    }
};

// ============ INTERPOLAÇÃO LINEAR ============
float Maps_Lerp(float y1, float y2, float t) {
    // t deve estar entre 0.0 e 1.0
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return y1 + (y2 - y1) * t;
}

// ============ BUSCA ÍNDICE RPM ============
float Maps_FindRPMIndex(uint32_t rpm, uint8_t *idx_low, uint8_t *idx_high) {
    const uint16_t *bins = ve_map_default.rpm_bins;
    
    // Limita RPM aos bins
    if (rpm <= bins[0]) {
        *idx_low = 0;
        *idx_high = 0;
        return 0.0f;
    }
    if (rpm >= bins[7]) {
        *idx_low = 7;
        *idx_high = 7;
        return 1.0f;
    }
    
    // Procura o intervalo
    for (int i = 0; i < 7; i++) {
        if (rpm >= bins[i] && rpm < bins[i + 1]) {
            *idx_low = i;
            *idx_high = i + 1;
            
            // Fração de interpolação
            float fraction = (float)(rpm - bins[i]) / (float)(bins[i + 1] - bins[i]);
            return fraction;
        }
    }
    
    *idx_low = 7;
    *idx_high = 7;
    return 1.0f;
}

// ============ BUSCA ÍNDICE LOAD ============
float Maps_FindLoadIndex(float load, uint8_t *idx_low, uint8_t *idx_high) {
    const uint8_t *bins = ve_map_default.load_bins;
    
    // Limita load aos bins
    if (load <= bins[0]) {
        *idx_low = 0;
        *idx_high = 0;
        return 0.0f;
    }
    if (load >= bins[7]) {
        *idx_low = 7;
        *idx_high = 7;
        return 1.0f;
    }
    
    // Procura o intervalo
    for (int i = 0; i < 7; i++) {
        if (load >= bins[i] && load < bins[i + 1]) {
            *idx_low = i;
            *idx_high = i + 1;
            
            // Fração de interpolação
            float fraction = (load - bins[i]) / (float)(bins[i + 1] - bins[i]);
            return fraction;
        }
    }
    
    *idx_low = 7;
    *idx_high = 7;
    return 1.0f;
}

// ============ LEITURA COM INTERPOLAÇÃO ============
uint8_t Maps_ReadVE(uint32_t rpm, float load) {
    uint8_t rpm_idx_low, rpm_idx_high;
    uint8_t load_idx_low, load_idx_high;
    float rpm_frac, load_frac;
    
    // Encontra índices RPM
    rpm_frac = Maps_FindRPMIndex(rpm, &rpm_idx_low, &rpm_idx_high);
    
    // Encontra índices Load
    load_frac = Maps_FindLoadIndex(load, &load_idx_low, &load_idx_high);
    
    // Interpolação bilinear
    float v00 = ve_map_default.table[rpm_idx_low][load_idx_low];
    float v01 = ve_map_default.table[rpm_idx_low][load_idx_high];
    float v10 = ve_map_default.table[rpm_idx_high][load_idx_low];
    float v11 = ve_map_default.table[rpm_idx_high][load_idx_high];
    
    float v0 = Maps_Lerp(v00, v01, load_frac);
    float v1 = Maps_Lerp(v10, v11, load_frac);
    float result = Maps_Lerp(v0, v1, rpm_frac);
    
    return (uint8_t)result;
}

// ============ LEITURA AVANÇO IGNIÇÃO ============
uint8_t Maps_ReadIGN(uint32_t rpm, float load) {
    uint8_t rpm_idx_low, rpm_idx_high;
    uint8_t load_idx_low, load_idx_high;
    float rpm_frac, load_frac;
    
    // Encontra índices RPM
    rpm_frac = Maps_FindRPMIndex(rpm, &rpm_idx_low, &rpm_idx_high);
    
    // Encontra índices Load
    load_frac = Maps_FindLoadIndex(load, &load_idx_low, &load_idx_high);
    
    // Interpolação bilinear
    float v00 = ign_map_default.table[rpm_idx_low][load_idx_low];
    float v01 = ign_map_default.table[rpm_idx_low][load_idx_high];
    float v10 = ign_map_default.table[rpm_idx_high][load_idx_low];
    float v11 = ign_map_default.table[rpm_idx_high][load_idx_high];
    
    float v0 = Maps_Lerp(v00, v01, load_frac);
    float v1 = Maps_Lerp(v10, v11, load_frac);
    float result = Maps_Lerp(v0, v1, rpm_frac);
    
    return (uint8_t)result;
}

// ============ ESCRITA VE ============
void Maps_WriteVE(uint8_t rpm_idx, uint8_t load_idx, uint8_t value) {
    if (rpm_idx < 8 && load_idx < 8) {
        ((uint8_t *)ve_map_default.table)[rpm_idx * 8 + load_idx] = value;
    }
}

// ============ ESCRITA IGN ============
void Maps_WriteIGN(uint8_t rpm_idx, uint8_t load_idx, uint8_t value) {
    if (rpm_idx < 8 && load_idx < 8) {
        ((uint8_t *)ign_map_default.table)[rpm_idx * 8 + load_idx] = value;
    }
}
