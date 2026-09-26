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

#define EEPROM_SIZE       1010
#define RECORD_SIZE       10
#define NUM_RECORDS      (EEPROM_SIZE / RECORD_SIZE) // 0-100 (101) records/slots/sectors
/*
             eeprom Add
Record 0     → 0–9
Record 1     → 10–19
Record 2     → 20–29
...
Record 100   → 1000–1009

Reserved     → 1010–1023
               ↑
             14 bytes
*/


typedef struct
{
    uint32_t token_serial;
    uint16_t crc;
} TokenRecord;
#define TOKEN_SERIAL_ADDR 1010
#define ALARM_STATUS_ADDR (TOKEN_SERIAL_ADDR + sizeof(TokenRecord))

typedef struct
{
    uint32_t sequence; // used to find the latest saved eeprom value
    uint32_t value;// any 4 byte value , usually imp_kWh
    uint16_t crc;
} EEPROM_Record;

// EEPROM functions
void EEPROM_Save_imp(uint32_t value,
                     uint32_t sequence,
                     uint8_t slot);

uint8_t EEPROM_Read(uint8_t slot,
                    EEPROM_Record *record);

uint8_t EEPROM_get_slot_number(void);
bool EEPROM_LoadTokenSerial(uint32_t *serial);
void EEPROM_saveTokenSerial(uint32_t serial);

#ifdef __cplusplus
}
#endif

#endif // EEPROM_STORAGE_H