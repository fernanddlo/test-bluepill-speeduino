#include "serial_protocol.h"
#include "config.h"
#include "STM32_specific.h"
#include "rpm.h"
#include "injection.h"
#include "sensors.h"
#include "eeprom.h"
#include "stm32f1xx_hal.h"
#include "string.h"

// ============ VARIÁVEL GLOBAL ============
SerialPort serial_port = {0};

// ============ INICIALIZAÇÃO ============
void Serial_Init(void) {
    // USART3 já inicializado em UART_Init() (STM32_specific.c)
    serial_port.state = 0;  // Idle
    Serial_ClearBuffers();
}

// ============ ENVIA UM BYTE ============
void Serial_SendByte(uint8_t byte) {
    HAL_UART_Transmit(&huart3, &byte, 1, 100);
}

// ============ ENVIA STRING ============
void Serial_SendString(const char *str) {
    uint16_t len = strlen(str);
    HAL_UART_Transmit(&huart3, (uint8_t *)str, len, 100);
}

// ============ ENVIA DADOS ============
void Serial_SendData(const uint8_t *data, uint16_t len) {
    HAL_UART_Transmit(&huart3, (uint8_t *)data, len, 100);
}

// ============ HANDLER RX ============
void Serial_RX_ISR(void) {
    // Callback de UART_RxCpltCallback é chamado aqui
    // Implementação simplificada: apenas marca como recebido
    serial_port.state = 1;  // Recebendo
}

// ============ PROCESSA COMANDOS TUNER STUDIO ============
void Serial_ProcessCommands(void) {
    static uint8_t command_buffer[MAX_COMMAND_LEN];
    static uint16_t buffer_pos = 0;
    
    // Verifica se há dados no buffer RX
    if (HAL_UART_Receive(&huart3, command_buffer + buffer_pos, 1, 0) == HAL_OK) {
        uint8_t byte = command_buffer[buffer_pos];
        
        // Protocolo TunerStudio simplificado
        // Comandos:
        // 'S' = Signature (compatibilidade)
        // 'Q' = Quit
        // 'R' = Read table
        // 'W' = Write table
        
        switch (byte) {
            case 'S':  // SIGNATURE
                // TunerStudio Free: responde com assinatura
                {
                    uint8_t sig[4];
                    sig[0] = (TS_SIGNATURE >> 0) & 0xFF;
                    sig[1] = (TS_SIGNATURE >> 8) & 0xFF;
                    sig[2] = (TS_SIGNATURE >> 16) & 0xFF;
                    sig[3] = (TS_SIGNATURE >> 24) & 0xFF;
                    Serial_SendData(sig, 4);
                }
                break;
            
            case 'Q':  // QUIT
                Serial_SendByte('O');
                break;
            
            case 'R':  // READ (próximos bytes são endereço)
                {
                    buffer_pos++;
                    if (buffer_pos >= 3) {
                        uint16_t addr = (command_buffer[1] << 8) | command_buffer[2];
                        uint8_t value = *((uint8_t *)(addr));
                        Serial_SendByte(value);
                        buffer_pos = 0;
                    }
                }
                break;
            
            case 'W':  // WRITE
                {
                    buffer_pos++;
                    if (buffer_pos >= 4) {
                        uint16_t addr = (command_buffer[1] << 8) | command_buffer[2];
                        uint8_t value = command_buffer[3];
                        *((uint8_t *)(addr)) = value;
                        Serial_SendByte('O');
                        buffer_pos = 0;
                    }
                }
                break;
            
            default:
                buffer_pos++;
                if (buffer_pos >= MAX_COMMAND_LEN) {
                    buffer_pos = 0;
                }
                break;
        }
    }
}

// ============ RETORNA ESTADO ============
uint8_t Serial_GetState(void) {
    return serial_port.state;
}

// ============ LIMPA BUFFERS ============
void Serial_ClearBuffers(void) {
    memset(serial_port.tx_buffer, 0, BUFFER_SIZE);
    memset(serial_port.rx_buffer, 0, BUFFER_SIZE);
    serial_port.tx_index = 0;
    serial_port.rx_index = 0;
}
