#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============ RPM / CAS ============
#define CAS_PULSES_PER_ROTATION 2          // 2 pulsos por 360°
#define CAS_INPUT_PIN PA10                 // TIM1_CH3
#define MIN_RPM_FOR_RUNNING 500            // RPM mínimo para sair de cranking
#define RPM_NO_SIGNAL_TIMEOUT_MS 1000      // Sem sinal = parada

// ============ INJECTION ============
#define INJ1_OUTPUT_PIN PA8                // TIM1_CH1 - Cyl 1+3
#define INJ2_OUTPUT_PIN PA9                // TIM1_CH2 - Cyl 2+4
#define INJECTION_MODE SIMULTANEOUS_DUAL   // Modo injeção
#define DWELL_TIME_MS 3.0f                 // Tempo carga bobina (ignição futura)
#define MAX_PULSE_WIDTH_MS 25.0f           // Proteção máxima
#define MIN_PULSE_WIDTH_MS 0.8f            // Mínimo deadband

// ============ IGNITION (WASTED SPARK - 2 OUTPUTS) ============
#define IGN_ENABLED 0                      // 1=ativo, 0=desabilitado
#define IGN_MODE WASTED_SPARK              // Modo: WASTED_SPARK

// IGN1: Cilindros 1 + 4 (180° fora de fase)
// IGN2: Cilindros 2 + 3 (180° fora de fase)
// Firing order: 1-2-4-3
#define IGN1_OUTPUT_PIN PA11               
#define IGN2_OUTPUT_PIN PA12               

#define IGN_DWELL_TIME_MS 3.0f             // Tempo de dwell (carga bobina)
#define IGN_MAX_ADVANCE 45.0f              // Avanço máximo em graus ATDC
#define IGN_MIN_ADVANCE 0.0f               // Avanço mínimo
#define IGN_FIXED_TIMING_CRANKING 10.0f    // Avanço fixo durante cranking

// ============ FUEL PUMP ============
#define FUEL_PUMP_PIN PB0
#define FUEL_PUMP_PRIME_TIME_MS 500        // Priming ao ligar
#define FUEL_PUMP_DISABLE_TIME_MS 2000     // Desliga sem RPM

// ============ SENSORS (ADC) ============
#define TPS_PIN PA0                        // Throttle Position Sensor
#define MAP_PIN PA1                        // Manifold Absolute Pressure
#define IAT_PIN PA2                        // Intake Air Temperature
#define CLT_PIN PA3                        // Coolant Temperature
#define O2_PIN PA4                         // Oxygen Sensor

// ============ SERIAL / TUNER STUDIO ============
#define SERIAL_TX_PIN PB10                 // USART3_TX
#define SERIAL_RX_PIN PB11                 // USART3_RX
#define SERIAL_BAUD 115200
#define TS_SIGNATURE 0x5F544531            // "_TE1" (TunerStudio Free identifier)

// ============ TIMERS ============
#define PWM_FREQUENCY_HZ 2000              // 2 kHz para PWM injeção
#define IGN_PWM_FREQUENCY_HZ 5000          // 5 kHz para PWM ignição (mais responsivo)
#define LOOP_FREQUENCY_HZ 100              // Loop principal 100 Hz (10 ms)

// ============ STORAGE / EEPROM ============
#define EEPROM_VE_MAP_ADDR 0x08007000     // VE table na Flash
#define EEPROM_IGN_MAP_ADDR 0x08007800    // Ign map
#define EEPROM_SIZE 2048                   // 2KB para mapas

// ============ ADC LIMITS ============
#define TPS_MIN_ADC 0
#define TPS_MAX_ADC 4095
#define MAP_MIN_KPA 10.0f
#define MAP_MAX_KPA 100.0f
#define CLT_MIN_TEMP -40.0f
#define CLT_MAX_TEMP 120.0f

// ============ MAPAS / LOOKUP TABLES ============
#define ROWS_RPM 8                         // RPM bins (500, 1000, 1500...)
#define COLS_LOAD 8                        // Load/TPS bins (0, 12.5, 25...)

// ============ ENUMS ============
enum InjectionMode {
    SEQUENTIAL = 0,
    BATCH_FIRE = 1,
    SIMULTANEOUS_DUAL = 2
};

enum IgnitionMode {
    IGN_DISABLED = 0,
    WASTED_SPARK = 1,          // Wasted spark (2 outputs)
    DIRECT_COIL = 2             // Futuro: bobina individual por cilindro
};

enum EngineState {
    ENGINE_OFF = 0,
    ENGINE_CRANKING = 1,
    ENGINE_RUNNING = 2,
    ENGINE_ERROR = 3
};

#endif // CONFIG_H
