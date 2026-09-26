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
        sizeof(EEPROM_Record) - sizeof(uint16_t) //calculate crc only on the dat not on the crc (uint16) bytes itself.
    );

    return (crc == record->crc); // reurn true or false
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

 
void EEPROM_Save_Record(EEPROM_Record *record, uint8_t slot)
{
    record->crc = CRC16(
        (uint8_t *)record,
        sizeof(EEPROM_Record) - sizeof(uint16_t)
    );

    eeprom_update_block(
        record,
        (void *)((uint16_t)slot * RECORD_SIZE),
        RECORD_SIZE
    );
}
  //-----------------------------
uint8_t EEPROM_Read_Record(uint8_t slot, EEPROM_Record *record)
{
	ReadRecord(slot, record);

	if (RecordValid(record))
	{
		return 1;   // valid record
	}
	return 0;       // invalid record
}
  

