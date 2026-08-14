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
 * Timestamp estendido da última captura.
 *
 * TIM1:
 *
 * 16 bits
 * 1 MHz
 *
 * Portanto:
 *
 * 0xFFFF = 65.535 ms
 *
 * O contador tim1_overflow_count transforma
 * o timestamp em aproximadamente 32 bits.
 */
static volatile uint32_t last_capture_extended = 0;


/*
 * Indica que ainda não existe uma captura anterior
 * para calcular o período.
 */
static volatile uint8_t first_capture = 1;


// ============================================================
// EXTENDED TIMESTAMP
// ============================================================

static uint32_t RPM_GetExtendedCapture(void)
{
    uint32_t overflow_count;
    uint32_t capture_value;
    uint32_t counter_value;

    /*
     * CCR3 contém o valor congelado no instante
     * da captura.
     */
    capture_value =
        HAL_TIM_ReadCapturedValue(
            &htim1,
            TIM_CHANNEL_3
        );

    /*
     * Lê o contador atual.
     *
     * Ele será usado para resolver o caso em que
     * o overflow aconteceu praticamente junto
     * com a captura.
     */
    counter_value =
        __HAL_TIM_GET_COUNTER(&htim1);

    /*
     * Copia o contador de overflow.
     */
    overflow_count =
        tim1_overflow_count;

    /*
     * ========================================================
     * OVERFLOW PENDENTE
     * ========================================================
     *
     * É possível que:
     *
     * 1. o timer tenha dado overflow;
     * 2. a captura tenha ocorrido;
     * 3. o callback de UPDATE ainda não tenha sido executado.
     *
     * Neste caso o flag UPDATE permanece ativo.
     *
     * Se:
     *
     * capture < counter
     *
     * a captura ocorreu depois do overflow.
     *
     * Portanto precisamos adicionar um overflow.
     */
    if (
        __HAL_TIM_GET_FLAG(
            &htim1,
            TIM_FLAG_UPDATE
        ) != RESET
    )
    {
        if (capture_value < counter_value)
        {
            overflow_count++;
        }
    }

    /*
     * Timestamp estendido:
     *
     * overflow_count * 65536
     * +
     * capture_value
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
     * Limpa dados públicos.
     */
    rpm_data.rpm = 0;
    rpm_data.raw_frequency = 0;
    rpm_data.last_pulse_time = 0;

    rpm_data.state = 0;

    rpm_data.pulse_count = 0;
    rpm_data.error_count = 0;

    /*
     * Limpa estado interno.
     */
    last_capture_extended = 0;
    first_capture = 1;

    /*
     * Reinicia extensão de overflow.
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

    uint64_t frequency_hz;
    uint64_t rpm_calculated;


    // ========================================================
    // READ CAPTURE
    // ========================================================

    capture_extended =
        RPM_GetExtendedCapture();


    // ========================================================
    // FIRST CAPTURE
    // ========================================================

    if (first_capture)
    {
        last_capture_extended =
            capture_extended;

        first_capture = 0;

        rpm_data.state = 0;

        return;
    }


    // ========================================================
    // PERIOD
    // ========================================================

    period_us =
        capture_extended -
        last_capture_extended;

    last_capture_extended =
        capture_extended;


    // ========================================================
    // VALIDATION
    // ========================================================

    /*
     * Pulso rápido demais.
     *
     * Isso normalmente indica ruído ou uma configuração
     * incorreta do trigger.
     */
    if (period_us < RPM_MIN_PERIOD_US)
    {
        rpm_data.error_count++;

        rpm_data.state = 2;

        return;
    }


    /*
     * Período extremamente longo.
     *
     * O RPM será tratado como perdido pelo timeout
     * de RPM_Process().
     */
    if (period_us > RPM_MAX_PERIOD_US)
    {
        rpm_data.error_count++;

        rpm_data.rpm = 0;
        rpm_data.raw_frequency = 0;

        rpm_data.state = 2;

        return;
    }


    // ========================================================
    // FREQUENCY
    // ========================================================

    /*
     * Frequência:
     *
     * F = timer_frequency / period
     *
     * Com:
     *
     * timer = 1 MHz
     * period = us
     */
    frequency_hz =
        (
            (uint64_t)RPM_TIMER_TICK_HZ
        )
        /
        period_us;


    rpm_data.raw_frequency =
        (uint32_t)frequency_hz;


    // ========================================================
    // RPM
    // ========================================================

    /*
     * RPM:
     *
     * RPM =
     *
     * timer_tick_hz * 60
     * -------------------------
     * period_us * pulses_per_rev
     *
     *
     * Exemplo:
     *
     * timer = 1.000.000 Hz
     * período = 10.000 us
     * 2 pulsos/rotação
     *
     * RPM =
     *
     * 1.000.000 * 60
     * ----------------
     * 10.000 * 2
     *
     * = 3.000 RPM
     */
    rpm_calculated =
        (
            (uint64_t)RPM_TIMER_TICK_HZ *
            60ULL
        )
        /
        (
            (uint64_t)period_us *
            (uint64_t)CAS_PULSES_PER_ROTATION
        );


    // ========================================================
    // SAFETY LIMIT
    // ========================================================

    if (
        rpm_calculated >
        RPM_MAX_VALID
    )
    {
        rpm_calculated =
            RPM_MAX_VALID;
    }


    rpm_data.rpm =
        (uint32_t)rpm_calculated;


    // ========================================================
    // TIMESTAMP
    // ========================================================

    rpm_data.last_pulse_time =
        HAL_GetTick();


    // ========================================================
    // STATE
    // ========================================================

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


    // ========================================================
    // NO PULSE YET
    // ========================================================

    if (
        rpm_data.last_pulse_time == 0
    )
    {
        rpm_data.rpm = 0;
        rpm_data.raw_frequency = 0;

        rpm_data.state = 0;

        return;
    }


    // ========================================================
    // SIGNAL TIMEOUT
    // ========================================================

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