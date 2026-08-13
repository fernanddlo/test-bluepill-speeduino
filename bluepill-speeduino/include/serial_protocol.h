#ifndef SERIAL_PROTOCOL_H
#define SERIAL_PROTOCOL_H

#include <stdint.h>

// ============ CONSTANTES ============
#define BUFFER_SIZE 256
#define MAX_COMMAND_LEN 64

// ============ ESTRUTURA ============
typedef struct {
    uint8_t tx_buffer[BUFFER_SIZE];
    uint8_t rx_buffer[BUFFER_SIZE];
    uint16_t tx_index;
    uint16_t rx_index;
    uint8_t state;  // 0=idle, 1=receiving, 2=error
    uint32_t last_msg_ms;
} SerialPort;

// ============ VARIÁVEL GLOBAL ============
extern SerialPort serial_port;

// ============ FUNÇÕES ============

/**
 * Inicializa USART3 para TunerStudio
 * - Baud rate: 115200
 * - Pinos: PB10 (TX), PB11 (RX)
 * - Ativa interrupção RX
 */
void Serial_Init(void);

/**
 * Envia um byte pela serial
 * @param byte Byte a enviar
 */
void Serial_SendByte(uint8_t byte);

/**
 * Envia string pela serial
 * @param str Ponteiro para string
 */
void Serial_SendString(const char *str);

/**
 * Envia n bytes
 * @param data Ponteiro para dados
 * @param len Quantidade de bytes
 */
void Serial_SendData(const uint8_t *data, uint16_t len);

/**
 * Handler de interrupção RX
 * Processa comandos TunerStudio
 */
void Serial_RX_ISR(void);

/**
 * Processa comandos recebidos
 * Implementa protocolo TunerStudio (signature, ping, read/write)
 */
void Serial_ProcessCommands(void);

/**
 * Retorna estado da porta serial
 * @return 0=idle, 1=recebendo, 2=erro
 */
uint8_t Serial_GetState(void);

/**
 * Limpa buffers RX/TX
 */
void Serial_ClearBuffers(void);

#endif // SERIAL_PROTOCOL_H
