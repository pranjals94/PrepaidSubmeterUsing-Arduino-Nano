#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "eeprom_storage.h"

EEPROM_Record record;

LiquidCrystal lcd(0, 1, 4, 5, 6, 7);

void setup()
{
    lcd.begin(16, 2);

    // -------------------------
    // WRITE
    // -------------------------

    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.update(i, 0x00);
    }


    record.imp_kWh      = 5UL;
    record.sequence     = 1UL;
    record.token_serial = 1;
    record.status       = 0b00001000;

   EEPROM_Save_Record(&record, 0);
    // write  0x00 to all other adresses


    lcd.clear();
    lcd.print("EEPROM WRITE");
    delay(1000);

    // -------------------------
    // READ
    // -------------------------

    if (EEPROM_Read_Record(0, &record))
    {
        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("Imp:");
        lcd.print(record.imp_kWh);

        lcd.setCursor(0, 1);
        lcd.print("Seq:");
        lcd.print(record.sequence);

        delay(2000);

        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("Serial:");
        lcd.print(record.token_serial);

        lcd.setCursor(0, 1);
        lcd.print("Status:");
        lcd.print(record.status, BIN);
    }
    else
    {
        lcd.clear();
        lcd.print("EEPROM ERROR");

        lcd.setCursor(0, 1);
        lcd.print("CRC Invalid");
    }
}

void loop()
{
}