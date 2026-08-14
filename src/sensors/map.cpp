static float MAP_ToLoad(float map_kpa)
{
    if (map_kpa <= MAP_MIN_KPA)
        return 0.0f;

    if (map_kpa >= MAP_MAX_KPA)
        return 100.0f;

    return
        ((map_kpa - MAP_MIN_KPA) /
        (MAP_MAX_KPA - MAP_MIN_KPA)) * 100.0f;
}
float map_kpa = Sensors_GetMAP();

float load = MAP_ToLoad(map_kpa);

Injection_Process(rpm, load);