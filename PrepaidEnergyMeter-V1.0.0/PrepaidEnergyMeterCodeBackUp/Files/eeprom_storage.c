/*
 * eeprom_storage.c
 *
 * Created: 07-09-2026 05:45:06:PM
 *  Author: pranj
 */ 
#include <avr/eeprom.h>
#include <stdint.h>
#include "eeprom_storage.h"

#define EEPROM_SIZE       500 // 512 bytes total rest bytes reserved for token serial no storage(uint32)
#define RECORD_SIZE       10
#define NUM_RECORDS      (EEPROM_SIZE / RECORD_SIZE) // 0-49 (50) records/slots/sectors


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

uint8_t eeprom_get_slot_number(void)
{
	uint32_t biggestSequence = 0;
	uint8_t slot = 0;
	uint8_t found = 0;

	EEPROM_Record record;

	for (uint8_t i = 0; i < 50; i++)
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
