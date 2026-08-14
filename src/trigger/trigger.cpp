#include "trigger.h"
#include "config.h"
#include "STM32_specific.h"
#include "stm32f1xx_hal.h"


// ============================================================
// GLOBAL RPM DATA
// ============================================================

RPM_Data rpm_data =
{
    .rpm = 0,
    .raw_frequency = 0,
    .last_pulse_time = 0,
    .state = 0,
    .pulse_count = 0,
    .error_count = 0
};


// ============================================================
// INTERNAL STATE
// ============================================================

/*
 * Timestamp estendido do último pulso.
 *
 * TIM1 é 16-bit e trabalha a 1 MHz.
 *
 * Portanto:
 *
 * 65536 ticks = 65.536 ms
 *
 * O contador de overflow do TIM1 transforma isso
 * em um timestamp efetivamente de 32 bits.
 */
static volatile uint32_t last_capture_extended = 0;


/*
 * Indica que ainda não recebemos duas capturas
 * consecutivas suficientes para calcular RPM.
 */
static volatile uint8_t first_capture = 1;


// ============================================================
// HELPER - GET EXTENDED CAPTURE TIME
// ============================================================

static uint32_t RPM_GetExtendedCapture(void)
{
    uint32_t overflow_count;
    uint32_t capture_value;

    /*
     * CCR3 contém o valor capturado pelo TIM1_CH3.
     */
    capture_value =
        HAL_TIM_ReadCapturedValue(
            &htim1,
            TIM_CHANNEL_3
        );

    /*
     * Contador de overflow mantido pelo callback
     * HAL_TIM_PeriodElapsedCallback().
     */
    overflow_count = tim1_overflow_count;

    /*
     * Condição especial:
     *
     * Pode ocorrer um overflow entre o momento da captura
     * e a execução da ISR de captura.
     *
     * Se o flag de update estiver pendente e o valor capturado
     * estiver próximo do início do contador, consideramos que
     * essa captura pertence ao próximo ciclo.
     */
    if (
        __HAL_TIM_GET_FLAG(
            &htim1,
            TIM_FLAG_UPDATE
        ) != RESET
        &&
        capture_value < 0x8000UL
    )
    {
        overflow_count++;
    }

    /*
     * Junta:
     *
     * overflow << 16
     *
     * +
     *
     * valor de captura
     */
    return
        (overflow_count << 16)
        |
        (capture_value & 0xFFFFUL);
}


// ============================================================
// INITIALIZATION
// ============================================================

void RPM_Init(void)
{
    /*
     * TIM1_CH3 já é configurado em Timer_Init().
     */

    rpm_data.rpm = 0;
    rpm_data.raw_frequency = 0;
    rpm_data.last_pulse_time = 0;

    rpm_data.state = 0;

    rpm_data.pulse_count = 0;
    rpm_data.error_count = 0;

    last_capture_extended = 0;
    first_capture = 1;

    /*
     * Garante que o contador de overflow comece zerado.
     */
    tim1_overflow_count = 0;
}


// ============================================================
// RPM CAPTURE ISR
// ============================================================

void RPM_CAS_ISR(void)
{
    uint32_t capture_extended;
    uint32_t period_us;
    uint32_t frequency_hz;
    uint64_t rpm_calculated;


    // --------------------------------------------------------
    // Lê timestamp estendido
    // --------------------------------------------------------

    capture_extended =
        RPM_GetExtendedCapture();


    // --------------------------------------------------------
    // Primeira captura
    // --------------------------------------------------------

    if (first_capture)
    {
        last_capture_extended =
            capture_extended;

        first_capture = 0;

        rpm_data.state = 0;

        return;
    }


    // --------------------------------------------------------
    // Calcula período
    // --------------------------------------------------------

    period_us =
        capture_extended -
        last_capture_extended;


    last_capture_extended =
        capture_extended;


    // --------------------------------------------------------
    // Validação
    // --------------------------------------------------------

    /*
     * Ignora pulsos impossivelmente rápidos.
     *
     * 100 us = 600.000 pulsos/min.
     *
     * Muito acima do necessário para o motor.
     */
    if (period_us < 100)
    {
        rpm_data.error_count++;

        rpm_data.state = 2;

        return;
    }


    /*
     * Timeout máximo físico utilizado para validar
     * um intervalo individual.
     *
     * 1 segundo permite velocidades extremamente baixas,
     * mas ainda evita aceitar sinal travado.
     */
    if (period_us > 1000000UL)
    {
        rpm_data.error_count++;

        rpm_data.rpm = 0;

        rpm_data.raw_frequency = 0;

        rpm_data.state = 2;

        return;
    }


    // --------------------------------------------------------
    // Frequência
    // --------------------------------------------------------

    /*
     * Frequência em Hz:
     *
     * 1.000.000 / período_us
     *
     * Aqui usamos uint64 para evitar overflow
     * e manter precisão.
     */
    frequency_hz =
        (uint32_t)(
            1000000ULL /
            period_us
        );


    rpm_data.raw_frequency =
        frequency_hz;


    // --------------------------------------------------------
    // RPM
    // --------------------------------------------------------

    /*
     * RPM:
     *
     * RPM =
     *
     * 60.000.000
     * -----------------------------
     * período_us * pulsos_por_rotação
     *
     *
     * Usamos diretamente o período em vez de:
     *
     * frequência -> divisão inteira -> RPM
     *
     * Isso preserva mais precisão.
     */

    rpm_calculated =
        60000000ULL /
        (
            (uint64_t)period_us *
            (uint64_t)CAS_PULSES_PER_ROTATION
        );


    if (rpm_calculated > 100000UL)
    {
        rpm_calculated = 100000UL;
    }


    rpm_data.rpm =
        (uint32_t)rpm_calculated;


    // --------------------------------------------------------
    // Timestamp
    // --------------------------------------------------------

    rpm_data.last_pulse_time =
        HAL_GetTick();


    // --------------------------------------------------------
    // Estado
    // --------------------------------------------------------

    rpm_data.state = 1;

    rpm_data.pulse_count++;
}


// ============================================================
// BACKGROUND PROCESSING
// ============================================================

void RPM_Process(void)
{
    uint32_t now_ms;
    uint32_t elapsed_ms;


    now_ms =
        HAL_GetTick();


    /*
     * Nunca recebemos pulso ainda.
     */
    if (rpm_data.last_pulse_time == 0)
    {
        rpm_data.rpm = 0;

        rpm_data.raw_frequency = 0;

        rpm_data.state = 0;

        return;
    }


    /*
     * Detecta perda de sinal.
     */
    elapsed_ms =
        now_ms -
        rpm_data.last_pulse_time;


    if (
        elapsed_ms >
        RPM_NO_SIGNAL_TIMEOUT_MS
    )
    {
        rpm_data.rpm = 0;

        rpm_data.raw_frequency = 0;

        rpm_data.state = 0;
    }
}


// ============================================================
// GET RPM
// ============================================================

uint32_t RPM_Get(void)
{
    return rpm_data.rpm;
}


// ============================================================
// GET STATE
// ============================================================

uint8_t RPM_GetState(void)
{
    return rpm_data.state;
}


// ============================================================
// RESET
// ============================================================

void RPM_Reset(void)
{
    rpm_data.rpm = 0;

    rpm_data.raw_frequency = 0;

    rpm_data.last_pulse_time = 0;

    rpm_data.state = 0;

    rpm_data.pulse_count = 0;

    rpm_data.error_count = 0;

    last_capture_extended = 0;

    first_capture = 1;

    tim1_overflow_count = 0;
}