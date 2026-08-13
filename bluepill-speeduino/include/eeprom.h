#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

// ============ ESTRUTURA ============
typedef struct {
    uint8_t ve_table[8][8];      // VE map (8x8)
    uint8_t ign_table[8][8];     // Ignition timing (8x8) - futuro
    uint16_t checksum;           // Checksum de validação
    uint16_t reserved;           // Para futuro
} EEPROMData;

// ============ VARIÁVEL GLOBAL ============
extern EEPROMData eeprom_data;

// ============ FUNÇÕES ============

/**
 * Inicializa sistema de EEPROM (Flash)
 * - Carrega tabelas da Flash
 * - Valida checksum
 * - Se inválido, carrega defaults
 */
void EEPROM_Init(void);

/**
 * Lê um valor da tabela VE
 * @param rpm_idx Índice RPM (0-7)
 * @param load_idx Índice load/TPS (0-7)
 * @return Valor VE (0-255, onde 100 = stoich)
 */
uint8_t EEPROM_ReadVE(uint8_t rpm_idx, uint8_t load_idx);

/**
 * Escreve um valor na tabela VE
 * Atualiza RAM e Flash
 * @param rpm_idx Índice RPM (0-7)
 * @param load_idx Índice load/TPS (0-7)
 * @param value Valor VE (0-255)
 * @return 1=sucesso, 0=erro
 */
uint8_t EEPROM_WriteVE(uint8_t rpm_idx, uint8_t load_idx, uint8_t value);

/**
 * Lê um valor da tabela de avanço (ignição)
 * @param rpm_idx Índice RPM (0-7)
 * @param load_idx Índice load/TPS (0-7)
 * @return Valor em graus (0-90)
 */
uint8_t EEPROM_ReadIGN(uint8_t rpm_idx, uint8_t load_idx);

/**
 * Escreve um valor na tabela de avanço
 * @param rpm_idx Índice RPM (0-7)
 * @param load_idx Índice load/TPS (0-7)
 * @param value Valor em graus (0-90)
 * @return 1=sucesso, 0=erro
 */
uint8_t EEPROM_WriteIGN(uint8_t rpm_idx, uint8_t load_idx, uint8_t value);

/**
 * Salva tabelas completas na Flash
 * Atualiza checksum
 * @return 1=sucesso, 0=erro
 */
uint8_t EEPROM_Commit(void);

/**
 * Carrega defaults nas tabelas
 * Cria mapas "segros" padrão para startup
 */
void EEPROM_LoadDefaults(void);

/**
 * Valida checksum das tabelas
 * @return 1=válido, 0=inválido
 */
uint8_t EEPROM_ValidateChecksum(void);

/**
 * Recalcula checksum (após modificações)
 */
void EEPROM_UpdateChecksum(void);

#endif // EEPROM_H
