/*
 * eeprom_storage.h
 *
 * Created: 07-09-2026 05:46:37:PM
 *  Author: pranj
 */ 

#ifndef EEPROM_STORAGE_H
#define EEPROM_STORAGE_H

#include <stdint.h>

typedef struct
{
	uint32_t sequence;
	uint32_t value;
	uint16_t crc;
} EEPROM_Record;

void EEPROM_Save_imp(uint32_t value,uint32_t sequence, uint8_t slot);
uint8_t EEPROM_Read(uint8_t slot, EEPROM_Record *record);
uint8_t eeprom_get_slot_number();
#endif