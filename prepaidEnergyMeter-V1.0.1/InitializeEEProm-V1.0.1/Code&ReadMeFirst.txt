/*Make a copy of the .eep file before using otherwise it gets deleted or corrupted.*/



#include "eeprom_storage.h"
#include <EEPROM.h>

void setup() {
    EEPROM_Record record;
    
  // put your setup code here, to run once:
      for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.update(i, 0x00);
    }

    record.imp_kWh      = 5UL;
    record.sequence     = 1UL;
    record.token_serial = 1;
    record.status       = 0b00001000;

   EEPROM_Save_Record(&record, 0);

}

void loop() {
  // put your main code here, to run repeatedly:

}
