/*
 * eeprom_storage.h
 *
 * Created: 07-09-2026 05:46:37:PM
 * Author: pranj
 */

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <stdint.h>

// Prevents C++ name mangling when compiled inside an Arduino project
#ifdef __cplusplus
extern "C" {
#endif

#define EEPROM_SIZE 1024
#define RECORD_SIZE sizeof(EEPROM_Record)
#define NUM_RECORDS (EEPROM_SIZE / RECORD_SIZE)

typedef struct
{
    uint32_t sequence; // used to find the latest saved eeprom value
    uint32_t imp_kWh;// any 4 byte value , usually imp_kWh
    uint16_t token_serial; // newly addeded
    uint8_t  status; // newly addeded
    uint16_t crc;
} EEPROM_Record;

uint8_t EEPROM_get_slot_number(void);
uint8_t EEPROM_Read_Record(uint8_t slot, EEPROM_Record *record);
void EEPROM_Save_Record(EEPROM_Record *record, uint8_t slot);

#ifdef __cplusplus
}
#endif

#endif // EEPROM_STORAGE_H