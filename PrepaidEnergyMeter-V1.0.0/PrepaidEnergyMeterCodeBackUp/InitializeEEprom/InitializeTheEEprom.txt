#include <EEPROM.h>
#include "eeprom_storage.h"
/*
#define EEPROM_SIZE       1010
#define RECORD_SIZE       10
#define NUM_RECORDS      (EEPROM_SIZE / RECORD_SIZE) // 0-100 (101) records/slots/sectors
#define ALARM_STATUS_ADDR (TOKEN_SERIAL_ADDR + sizeof(TokenRecord))

typedef struct
{
    uint32_t token_serial;
    uint16_t crc;
} TokenRecord;

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
*/
void setup()
{
    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.update(i, 0x00);
    }

  // initialize the token serial no to 2 max value is uint16
	EEPROM_saveTokenSerial(01); //TOKEN_SERIAL_ADDR 1010 only run once for the frist time 
  EEPROM.update(ALARM_STATUS_ADDR, 0b00001000);
}

void loop()
{
}