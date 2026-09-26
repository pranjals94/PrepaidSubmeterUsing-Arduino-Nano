typedef struct
{
    uint32_t sequence; // used to find the latest saved eeprom value
    uint32_t imp_kWh;// any 4 byte value , usually imp_kWh
    uint16_t token_serial; // newly addeded
    uint8_t  status; // newly addeded
    uint16_t crc;
} EEPROM_Record;

//------------------------------------------------------

    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.update(i, 0x00);
    }


    record.imp_kWh      = 5UL;
    record.sequence     = 1UL;
    record.token_serial = 1;
    record.status       = 0b00001000;

   EEPROM_Save_Record(&record, 0);
//---------------------------------------------------------

