/*
 * eeprom_storage.c
 *
 * Created: 07-09-2026 05:45:06:PM
 *  Author: pranj
 */ 
#include <avr/eeprom.h>
#include <stdint.h>
#include <stdbool.h>
#include "eeprom_storage.h"



/* ---------------------------------------------------------
   CRC16
   --------------------------------------------------------- */

static uint16_t CRC16(const uint8_t *data, uint8_t length)
{
    uint16_t crc = 0xFFFF;

    uint8_t i;
    uint8_t j;

    for (i = 0; i < length; i++)
    {
        crc ^= data[i];

        for (j = 0; j < 8; j++)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}


/* ---------------------------------------------------------
   Check whether EEPROM record is valid
   --------------------------------------------------------- */

static uint8_t RecordValid(EEPROM_Record *record)
{
    uint16_t crc;
	
	//calculate the CRC over the first 8 bytes of the record structure
	/*
	typedef struct
	{
		uint32_t sequence;  // 4 bytes
		uint32_t value;     // 4 bytes
		uint16_t crc;       // 2 bytes
	} EEPROM_Record;
	*/
    crc = CRC16(
        (uint8_t *)record,
        8
    );

    if (crc == record->crc)
        return 1;

    return 0;
}


/* ---------------------------------------------------------
   Read record
   eeprom_read_block(RAM_address, EEPROM_address, number_of_bytes);
   // reads multiple bytes from the eeprom and stores in the record variable
   --------------------------------------------------------- */
static void ReadRecord(uint8_t slot,
                       EEPROM_Record *record)
{
    uint16_t address;

    address = (uint16_t)slot * RECORD_SIZE;

    eeprom_read_block(
        record,
        (const void *)address,
        RECORD_SIZE
    );
}


void EEPROM_Save_imp(uint32_t value,uint32_t sequence, uint8_t slot)
{
    EEPROM_Record record;
	record.sequence = sequence;
	record.value = value;

	record.crc = CRC16(
	(uint8_t *)&record,
	8
	);
	
	 eeprom_update_block(
	 &record,
	 (void *)((uint8_t)slot * RECORD_SIZE),
	 RECORD_SIZE
	 );
}

uint8_t EEPROM_Read(uint8_t slot, EEPROM_Record *record)
{
	ReadRecord(slot, record);

	if (RecordValid(record))
	{
		return 1;   // valid record
	}

	return 0;       // invalid record
}

uint8_t EEPROM_get_slot_number(void)
{
	uint32_t biggestSequence = 0;
	uint8_t slot = 0;
	uint8_t found = 0;

	EEPROM_Record record;

	for (uint8_t i = 0; i < NUM_RECORDS; i++)
	{
		if (EEPROM_Read(i, &record))
		{
			if (!found || record.sequence > biggestSequence)
			{
				biggestSequence = record.sequence;
				slot = i;
				found = 1;
			}
		}
	}

	return slot;
}


void EEPROM_saveTokenSerial(uint32_t serial)
{
    TokenRecord record;

    record.token_serial = serial;

    // Calculate CRC of the 4-byte serial
    record.crc = CRC16(
        (uint8_t *)&record.token_serial,
        sizeof(record.token_serial)
    );

    // Write 6 bytes to EEPROM starting at address 1010
    eeprom_update_block(
        &record,
        (void *)TOKEN_SERIAL_ADDR,
        sizeof(record)
    );
}

bool EEPROM_LoadTokenSerial(uint32_t *serial)
{
    TokenRecord record;

    // Read 6 bytes from EEPROM address 1010
    eeprom_read_block(
        &record,
        (const void *)TOKEN_SERIAL_ADDR,
        sizeof(record)
    );

    // Calculate CRC again
    uint16_t calculatedCRC = CRC16(
        (uint8_t *)&record.token_serial,
        sizeof(record.token_serial)
    );

    // Check CRC
    if (calculatedCRC != record.crc)
    {
        return false;
    }

    *serial = record.token_serial;

    return true;
}
