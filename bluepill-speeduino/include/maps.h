#ifndef MAPS_H
#define MAPS_H

#include <stdint.h>

// ============ ESTRUTURAS ============
typedef struct {
    uint16_t rpm_bins[8];      // RPM: 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000
    uint8_t load_bins[8];      // Load%: 0, 12.5, 25, 37.5, 50, 62.5, 75, 87.5, 100
    uint8_t table[8][8];       // Valores interpoláveis
} MapTable;

// ============ VARIÁVEIS GLOBAIS ============
extern const MapTable ve_map_default;      // Mapa VE padrão
extern const MapTable ign_map_default;     // Mapa IGN padrão (futuro)

// ============ FUNÇÕES DE INTERPOLAÇÃO ============

/**
 * Busca valor na tabela com interpolação bilinear
 * Encontra o índice correto para RPM e Load
 * Interpola entre pontos adjacentes
 * 
 * @param map Ponteiro para MapTable
 * @param rpm RPM atual
 * @param load Carga (0.0 a 100.0 %)
 * @return Valor interpolado
 */
float Maps_GetInterpolated(const MapTable *map, uint32_t rpm, float load);

/**
 * Busca índices de RPM
 * Retorna os dois índices para interpolação
 * @param rpm RPM atual
 * @param idx_low Ponteiro para índice inferior
 * @param idx_high Ponteiro para índice superior
 * @return Fração de interpolação (0.0 a 1.0)
 */
float Maps_FindRPMIndex(uint32_t rpm, uint8_t *idx_low, uint8_t *idx_high);

/**
 * Busca índices de Load
 * Retorna os dois índices para interpolação
 * @param load Load em % (0.0 a 100.0)
 * @param idx_low Ponteiro para índice inferior
 * @param idx_high Ponteiro para índice superior
 * @return Fração de interpolação (0.0 a 1.0)
 */
float Maps_FindLoadIndex(float load, uint8_t *idx_low, uint8_t *idx_high);

/**
 * Interpolação linear simples
 * Usada internamente
 * @param y1 Valor 1
 * @param y2 Valor 2
 * @param t Fração (0.0 a 1.0)
 * @return Valor interpolado
 */
float Maps_Lerp(float y1, float y2, float t);

/**
 * Imprime mapa na serial (debug)
 * @param map Ponteiro para MapTable
 */
void Maps_DebugPrint(const MapTable *map);

#endif // MAPS_H
