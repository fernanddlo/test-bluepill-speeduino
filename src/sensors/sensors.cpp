#include "sensors.h"
#include "config.h"
#include "STM32_specific.h"
#include "stm32f1xx_hal.h"

// ============ VARIÁVEL GLOBAL ============
SensorData sensor_data = {
    .tps = 0.0f,
    .map = 0.0f,
    .iat = 0.0f,
    .clt = 0.0f,
    .o2 = 0.5f,
    .last_update_ms = 0
};

// ============ INICIALIZAÇÃO ============
void Sensors_Init(void) {
    // ADC1 já inicializado em ADC_Init() (STM32_specific.c)
    // Aqui apenas inicializa valores
    sensor_data.tps = 0.0f;
    sensor_data.map = 50.0f;
    sensor_data.iat = 25.0f;
    sensor_data.clt = 25.0f;
}

// ============ LEITURA DE TODOS OS SENSORES ============
void Sensors_ReadAll(void) {
    // Lê ADC para os 5 canais
    // Canais: PA0=0, PA1=1, PA2=2, PA3=3, PA4=4
    
    // Simplificado: usar DMA ou polling conforme configuração
    // Aqui usamos polling simples para cada canal
    
    // TPS (PA0 = ADC_CHANNEL_0)
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_buffer[0] = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // MAP (PA1 = ADC_CHANNEL_1)
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_buffer[1] = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // IAT (PA2 = ADC_CHANNEL_2)
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_buffer[2] = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // CLT (PA3 = ADC_CHANNEL_3)
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_buffer[3] = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // O2 (PA4 = ADC_CHANNEL_4)
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 100);
    adc_buffer[4] = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    
    // Converte valores ADC para físicos
    sensor_data.tps = (float)adc_buffer[0] / 4095.0f * 100.0f;
    sensor_data.map = MAP_MIN_KPA + (float)adc_buffer[1] / 4095.0f * (MAP_MAX_KPA - MAP_MIN_KPA);
    sensor_data.iat = (float)adc_buffer[2] / 4095.0f * 100.0f - 40.0f;  // Simplificado
    sensor_data.clt = (float)adc_buffer[3] / 4095.0f * 100.0f - 40.0f;  // Simplificado
    sensor_data.o2 = (float)adc_buffer[4] / 4095.0f;
    
    sensor_data.last_update_ms = HAL_GetTick();
}

// ============ GETTERS ============
float Sensors_GetTPS(void) {
    return sensor_data.tps;
}

float Sensors_GetMAP(void) {
    return sensor_data.map;
}

float Sensors_GetIAT(void) {
    return sensor_data.iat;
}

float Sensors_GetCLT(void) {
    return sensor_data.clt;
}

float Sensors_GetO2(void) {
    return sensor_data.o2;
}

// ============ VALIDAÇÃO ============
uint8_t Sensors_Validate(uint8_t sensor_id) {
    switch (sensor_id) {
        case 0:  // TPS
            return (sensor_data.tps >= 0.0f && sensor_data.tps <= 100.0f) ? 1 : 0;
        case 1:  // MAP
            return (sensor_data.map >= MAP_MIN_KPA && sensor_data.map <= MAP_MAX_KPA) ? 1 : 0;
        case 2:  // IAT
            return (sensor_data.iat >= CLT_MIN_TEMP && sensor_data.iat <= CLT_MAX_TEMP) ? 1 : 0;
        case 3:  // CLT
            return (sensor_data.clt >= CLT_MIN_TEMP && sensor_data.clt <= CLT_MAX_TEMP) ? 1 : 0;
        case 4:  // O2
            return (sensor_data.o2 >= 0.0f && sensor_data.o2 <= 1.0f) ? 1 : 0;
        default:
            return 0;
    }
}
