#include "eeprom.h"
#include "config.h"
#include "STM32_specific.h"
#include "stm32f1xx_hal.h"
#include "string.h"

// ============ VARIÁVEL GLOBAL ============
EEPROMData eeprom_data = {0};

// ============ DEFAULTS PARA TABELAS ============
static const uint8_t ve_default[8][8] = {
    {80,  85,  90,  95,  100, 105, 110, 115},
    {82,  87,  92,  97,  102, 107, 112, 117},
    {84,  89,  94,  99,  104, 109, 114, 119},
    {86,  91,  96,  101, 106, 111, 116, 121},
    {88,  93,  98,  103, 108, 113, 118, 123},
    {90,  95,  100, 105, 110, 115, 120, 125},
    {92,  97,  102, 107, 112, 117, 122, 127},
    {94,  99,  104, 109, 114, 119, 124, 129}
};

static const uint8_t ign_default[8][8] = {
    {5,   6,   8,   10,  12,  14,  16,  18},
    {6,   7,   9,   11,  13,  15,  17,  19},
    {7,   8,   10,  12,  14,  16,  18,  20},
    {8,   9,   11,  13,  15,  17,  19,  21},
    {9,   10,  12,  14,  16,  18,  20,  22},
    {10,  11,  13,  15,  17,  19,  21,  23},
    {11,  12,  14,  16,  18,  20,  22,  24},
    {12,  13,  15,  17,  19,  21,  23,  25}
};

// ============ INICIALIZAÇÃO ============
void EEPROM_Init(void) {
    // Tenta carregar da Flash
    uint8_t *flash_ptr = (uint8_t *)EEPROM_VE_MAP_ADDR;
    
    // Copia dados da Flash para RAM
    memcpy((uint8_t *)&eeprom_data, flash_ptr, sizeof(EEPROMData));
    
    // Valida checksum
    if (!EEPROM_ValidateChecksum()) {
        // Se inválido, carrega defaults
        EEPROM_LoadDefaults();
    }
}

// ============ CARREGA DEFAULTS ============
void EEPROM_LoadDefaults(void) {
    // Copia tabelas default
    memcpy(eeprom_data.ve_table, (uint8_t *)ve_default, sizeof(ve_default));
    memcpy(eeprom_data.ign_table, (uint8_t *)ign_default, sizeof(ign_default));
    
    // Calcula checksum
    EEPROM_UpdateChecksum();
}

// ============ CALCULA CHECKSUM ============
void EEPROM_UpdateChecksum(void) {
    uint16_t sum = 0;
    uint8_t *ptr = (uint8_t *)&eeprom_data.ve_table;
    
    // Soma todos os bytes das tabelas
    for (int i = 0; i < 8 * 8 * 2; i++) {
        sum += ptr[i];
    }
    
    eeprom_data.checksum = sum;
}

// ============ VALIDA CHECKSUM ============
uint8_t EEPROM_ValidateChecksum(void) {
    uint16_t sum = 0;
    uint8_t *ptr = (uint8_t *)&eeprom_data.ve_table;
    
    for (int i = 0; i < 8 * 8 * 2; i++) {
        sum += ptr[i];
    }
    
    return (sum == eeprom_data.checksum) ? 1 : 0;
}

// ============ LÊ VE ============
uint8_t EEPROM_ReadVE(uint8_t rpm_idx, uint8_t load_idx) {
    if (rpm_idx >= 8 || load_idx >= 8) return 100;
    return eeprom_data.ve_table[rpm_idx][load_idx];
}

// ============ ESCREVE VE ============
uint8_t EEPROM_WriteVE(uint8_t rpm_idx, uint8_t load_idx, uint8_t value) {
    if (rpm_idx >= 8 || load_idx >= 8) return 0;
    
    eeprom_data.ve_table[rpm_idx][load_idx] = value;
    EEPROM_UpdateChecksum();
    
    return 1;
}

// ============ LÊ IGN ============
uint8_t EEPROM_ReadIGN(uint8_t rpm_idx, uint8_t load_idx) {
    if (rpm_idx >= 8 || load_idx >= 8) return 10;
    return eeprom_data.ign_table[rpm_idx][load_idx];
}

// ============ ESCREVE IGN ============
uint8_t EEPROM_WriteIGN(uint8_t rpm_idx, uint8_t load_idx, uint8_t value) {
    if (rpm_idx >= 8 || load_idx >= 8) return 0;
    
    eeprom_data.ign_table[rpm_idx][load_idx] = value;
    EEPROM_UpdateChecksum();
    
    return 1;
}

// ============ SALVA NA FLASH (FUTURO) ============
uint8_t EEPROM_Commit(void) {
    // TODO: Implementar escrita em Flash
    // Por enquanto, apenas em RAM
    EEPROM_UpdateChecksum();
    return 1;
}
