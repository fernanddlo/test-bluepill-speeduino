#include "map.h"
#include "config.h"


// ============================================================
// MAP → ENGINE LOAD
// ============================================================
//
// Converte pressão absoluta do coletor (kPa)
// para carga percentual.
//
// MAP_MIN_KPA = carga mínima
// MAP_MAX_KPA = carga máxima
//
// Exemplo:
// 30 kPa → carga baixa
// 100 kPa → carga máxima
//
// ============================================================

float MAP_ToLoad(float map_kpa)
{
    if (map_kpa <= MAP_MIN_KPA)
    {
        return 0.0f;
    }

    if (map_kpa >= MAP_MAX_KPA)
    {
        return 100.0f;
    }

    float load =
        ((map_kpa - MAP_MIN_KPA) /
        (MAP_MAX_KPA - MAP_MIN_KPA)) * 100.0f;

    return load;
}