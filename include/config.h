#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/*
 * ============================================================
 * BLUEPILL SPEEDUINO ECU
 * STM32F103C8T6
 *
 * Opala 4 cilindros
 * 4 tempos
 *
 * Combustível:
 *   2 canais de injeção
 *   INJ1 = cilindros 1 + 3
 *   INJ2 = cilindros 2 + 4
 *
 * Ignição:
 *   2 canais reservados
 *   ainda desabilitada
 *
 * Trigger:
 *   distribuidor básico inicialmente
 *   Hall/VR futuramente
 * ============================================================
 */


/* ============================================================
 * ENGINE
 * ============================================================ */

#define ENGINE_CYLINDERS                 4
#define ENGINE_STROKE_CYCLE_DEG         720.0f

/*
 * Ordem de ignição do Opala 4 cilindros:
 *
 * 1 - 2 - 4 - 3
 */
#define ENGINE_FIRING_ORDER_1            1
#define ENGINE_FIRING_ORDER_2            2
#define ENGINE_FIRING_ORDER_3            4
#define ENGINE_FIRING_ORDER_4            3


/* ============================================================
 * TRIGGER / RPM
 * ============================================================ */

/*
 * Distribuidor fornece dois pulsos por volta do virabrequim
 * nesta primeira implementação.
 */
#define CAS_PULSES_PER_ROTATION          2
//#define TRIGGER_PULSES_PER_CRANK_REV 2

#define CAS_INPUT_PIN                    PA10

#define MIN_RPM_FOR_RUNNING              500
#define RPM_NO_SIGNAL_TIMEOUT_MS         1000


/*
 * Trigger inicial
 */
#define TRIGGER_BASIC_DISTRIBUTOR        0
#define TRIGGER_HALL                     1
#define TRIGGER_VR                       2

#define TRIGGER_TYPE                     TRIGGER_BASIC_DISTRIBUTOR


/* ============================================================
 * INJECTION
 * ============================================================ */

#define INJ1_OUTPUT_PIN                  PA8
#define INJ2_OUTPUT_PIN                  PA9

/*
 * INJ1:
 *   cilindros 1 + 3
 *
 * INJ2:
 *   cilindros 2 + 4
 */
#define INJ1_CYLINDER_MASK               0x05
#define INJ2_CYLINDER_MASK               0x0A


/*
 * Estratégia inicial:
 *
 * dois canais simultâneos.
 *
 * Posteriormente podemos implementar:
 *   BATCH
 *   ALTERNATING
 *   SEMI-SEQUENTIAL
 */
#define INJECTION_MODE_SIMULTANEOUS      0
#define INJECTION_MODE_ALTERNATING       1

#define INJECTION_MODE                   INJECTION_MODE_SIMULTANEOUS


/*
 * Limites de segurança
 */
#define MAX_PULSE_WIDTH_MS               25.0f
#define MIN_PULSE_WIDTH_MS               0.8f

#define INJECTION_BASE_PW_MS             3.0f


/* ============================================================
 * IGNITION
 * ============================================================ */

/*
 * IMPORTANTE:
 *
 * PA11/PA12 NÃO são usados.
 *
 * Eles ficam reservados para USB.
 *
 * IGN1 = PB4
 * IGN2 = PB5
 */

#define IGN_ENABLED                      0

#define IGN_MODE_DISABLED                0
#define IGN_MODE_WASTED_SPARK            1

#define IGN_MODE                         IGN_MODE_WASTED_SPARK


#define IGN1_OUTPUT_PIN                  PB4
#define IGN2_OUTPUT_PIN                  PB5


#define IGN1_CYLINDER_MASK               0x09
#define IGN2_CYLINDER_MASK               0x06


#define IGN_DWELL_TIME_MS                3.0f

#define IGN_MAX_ADVANCE                  45.0f
#define IGN_MIN_ADVANCE                  0.0f

#define IGN_FIXED_TIMING_CRANKING        10.0f


/* ============================================================
 * FUEL PUMP
 * ============================================================ */

#define FUEL_PUMP_PIN                    PB0

#define FUEL_PUMP_PRIME_TIME_MS          500
#define FUEL_PUMP_DISABLE_TIME_MS        2000


/* ============================================================
 * ANALOG SENSORS
 * ============================================================ */

#define TPS_PIN                           PA0
#define MAP_PIN                           PA1
#define IAT_PIN                           PA2
#define CLT_PIN                           PA3
#define O2_PIN                            PA4


/* ============================================================
 * SENSOR LIMITS
 * ============================================================ */

#define TPS_MIN_ADC                       0
#define TPS_MAX_ADC                       4095

#define MAP_MIN_KPA                       10.0f
#define MAP_MAX_KPA                       100.0f

#define CLT_MIN_TEMP                      -40.0f
#define CLT_MAX_TEMP                      120.0f


/* ============================================================
 * TUNERSTUDIO / SERIAL
 * ============================================================ */

/*
 * USART3
 *
 * PB10 = TX
 * PB11 = RX
 *
 * Não interfere com:
 *
 * ADC      PA0..PA4
 * INJ      PA8/PA9
 * RPM      PA10
 * USB      PA11/PA12
 * SWD      PA13/PA14
 * IGN      PB4/PB5
 * Pump     PB0
 */

#define SERIAL_TX_PIN                    PB10
#define SERIAL_RX_PIN                    PB11

#define SERIAL_BAUD                      115200

#define TS_SIGNATURE                     0x5F544531


/* ============================================================
 * TIMERS
 * ============================================================ */

/*
 * TIM1
 *
 * CH1 = INJ1
 * CH2 = INJ2
 * CH3 = RPM
 */
#define TIM1_TIMER_TICK_HZ                1000000UL

#define TIM1_TIMER_TICK_US                1UL

/*
 * TIM4
 *
 * scheduler principal
 *
 * 100 Hz = 10 ms
 */
#define LOOP_FREQUENCY_HZ                100


/*
 * TIM3
 *
 * reservado para ignição
 *
 * CH1 = PB4
 * CH2 = PB5
 */
#define IGNITION_TIMER                   TIM3


/* ============================================================
 * MAP TABLE
 * ============================================================ */

#define ROWS_RPM                         8
#define COLS_LOAD                        8

/*
 * Load bins:
 *
 * 0
 * 12
 * 25
 * 40
 * 55
 * 70
 * 85
 * 100 %
 */


/* ============================================================
 * FLASH STORAGE
 * ============================================================ */

#define EEPROM_VE_MAP_ADDR               0x08007000
#define EEPROM_IGN_MAP_ADDR              0x08007800

#define EEPROM_SIZE                      2048


/* ============================================================
 * ENUMS
 * ============================================================ */

enum InjectionMode
{
    SEQUENTIAL = 0,
    BATCH_FIRE = 1,
    SIMULTANEOUS_DUAL = 2
};


enum IgnitionMode
{
    IGN_DISABLED = 0,
    WASTED_SPARK = 1,
    DIRECT_COIL = 2
};


enum EngineState
{
    ENGINE_OFF = 0,
    ENGINE_CRANKING = 1,
    ENGINE_RUNNING = 2,
    ENGINE_ERROR = 3
};


#endif