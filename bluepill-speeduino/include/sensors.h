#ifndef SENSORS_H
#define SENSORS_H

#include <stdint.h>

// ============ ESTRUTURA AGREGADA ============
typedef struct {
    float tps;     // Throttle Position (0.0 a 100.0 %)
    float map;     // Manifold Absolute Pressure (kPa)
    float iat;     // Intake Air Temperature (°C)
    float clt;     // Coolant Temperature (°C)
    float o2;      // Oxygen sensor / Lambda (0.0 a 1.0)
    uint32_t last_update_ms;
} SensorData;

// ============ VARIÁVEL GLOBAL ============
extern SensorData sensor_data;

// ============ FUNÇÕES ============

/**
 * Inicializa ADC para leitura de sensores
 * - Configura 5 canais: PA0, PA1, PA2, PA3, PA4
 * - DMA ou polling (conforme necessidade)
 * - Calibração ADC
 */
void Sensors_Init(void);

/**
 * Lê todos os sensores (conversão ADC)
 * Atualiza sensor_data com valores brutos convertidos
 * Deve ser chamado ~100 Hz (loop principal)
 */
void Sensors_ReadAll(void);

/**
 * Lê TPS especificamente
 * @return Valor 0.0 a 100.0 (%)
 */
float Sensors_GetTPS(void);

/**
 * Lê MAP
 * @return Valor em kPa
 */
float Sensors_GetMAP(void);

/**
 * Lê temperatura do ar admitido
 * @return Valor em °C
 */
float Sensors_GetIAT(void);

/**
 * Lê temperatura do líquido de arrefecimento
 * @return Valor em °C
 */
float Sensors_GetCLT(void);

/**
 * Lê sensor O2
 * @return Valor 0.0 a 1.0 (lambda)
 */
float Sensors_GetO2(void);

/**
 * Valida sensor (checking range, error conditions)
 * @param sensor_id Qual sensor (TPS=0, MAP=1, etc)
 * @return 1=ok, 0=erro
 */
uint8_t Sensors_Validate(uint8_t sensor_id);

#endif // SENSORS_H
