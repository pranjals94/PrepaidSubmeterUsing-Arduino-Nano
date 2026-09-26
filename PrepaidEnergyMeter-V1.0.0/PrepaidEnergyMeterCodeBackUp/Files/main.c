// write all the eeprom memory to zeros initially
// if using diodes in circuit check the voltage drop
// if the voltage drops too much like 4.5 volts , some 
//modules like lcd works abnormally.

#define F_CPU 16000000UL
#include <stdlib.h>
#include <stdbool.h>

#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "eeprom_storage.h"
#include "aes.h"
#include "lcd.h"

//imp_kwh must be initialize to 0, when on actual deployment
volatile uint32_t imp_kWh = 800000UL; // vslue Range 0 ? 4,294,967,295;
volatile uint32_t sequence;
volatile uint8_t slot;
 bool powerDown = false;

// INT0: PD2 falling edge (High priority interrupt run power down code here)
ISR(INT0_vect)
{
	EEPROM_Save_imp(imp_kWh,sequence,slot);
	PORTC |= (1 << PC2);
	powerDown = true;
	_delay_ms(500);
}

// INT1: PD3 falling edge; imp/kwhr counter
ISR(INT1_vect)
{
	if(imp_kWh<3){ //(imp_kWh<1) is ok but for safety reasons keep it (imp_kWh<3)
		imp_kWh=0;
	}else{
		imp_kWh--;
	}
	PORTC |=(1 << PC3);
}

void init_interrupts(){
	// PD2 (INT0) PD3 (INT1) as input
	DDRD &= ~((1 << PD2)|(1 << PD3));
	// Enable internal pull-up on INT0 and INT1
	PORTD |= (1 << PD2)|(1 << PD3);
	    
	MCUCR &= ~((1 << ISC00) | (1 << ISC10));
	MCUCR |=  ((1 << ISC01) | (1 << ISC11));
	GICR |= (1 << INT0) | (1 << INT1);
}

void beep(void) // Active buzzer
{
	// Your wiring: +5V -> beeper -> PB1
	// HIGH = OFF
	PORTB &= ~(1 << PB1);
	_delay_ms(100);
	PORTB |= (1 << PB1);
	_delay_ms(100);
}

void bytes_to_hex(const uint8_t *input, char *output, uint8_t length)
{
	const char hex[] = "0123456789ABCDEF";

	for (uint8_t i = 0; i < length; i++)
	{
		output[i * 2]     = hex[(input[i] >> 4) & 0x0F];
		output[i * 2 + 1] = hex[input[i] & 0x0F];
	}

	output[length * 2] = '\0';
}

int main(void)
{
	// PC2 as output for leds
    DDRC |= (1 << PC2)|(1 << PC3);
	DDRB |=(1<<PB1); // for beeper
	
	PORTB&=~(1<<PB1);// turn on beep
  
	init_interrupts();
    // Enable global interrupts
    sei();
	lcd_init();
	char buffer[12];
	uint32_t whole =0, value;
	uint16_t decimal = 0;
	slot = eeprom_get_slot_number(); // get the newest slot number
	EEPROM_Record record={0};// initially all zero
	if(EEPROM_Read(slot,&record)){// if crc valid
		slot++;
		if(slot >=50){slot=0;}
		sequence = record.sequence;
		sequence++;
		if(sequence>=4294967290)// max value of uint32_t 4294967295
		{
			//discard the meter
		}
		imp_kWh = record.value;
		
		if(imp_kWh>=4294967290)// max value of uint32_t 4294967295
		{
			//imp_kwh units not allowed to this extent
			// use this if loop during top up recharge
		}
	}



//-----AES--------------
/*
 * AES-128 key
 * Must be exactly 16 bytes.
 */

/*
unsigned char aesKey[17] ="keycode123456789";//last byte for '\0' termination
// token, exactly 16 bytes or give padding if empty
unsigned char token_message[17] ="154356253401|150";//"serial,Id,|kwhr.. 
char hexCiphertext[33];
	// the key needs to be patched for optimization for microcontrollers
	//and stored in the aesKey, so the original key is no longer available
	//in the aesKey variable, make a copy if needed.
	// aesKey now contains the patched key
	aesKeyPatch(aesKey);
	
	
	// encrypt using the patched key and store the encrypted data in 
	// the message variable itself. thus original message will be lost
	aesCipher(aesKey, token_message);
	lcd_gotoxy(0, 0);
	lcd_puts(token_message);
	
	lcd_gotoxy(1, 0);
	bytes_to_hex(token_message, hexCiphertext, 16);
	lcd_puts(hexCiphertext);
	
	// decrypt using the patched key
	//aesInvCipher(aesKey, token_message);
	//lcd_gotoxy(1, 0);
	//lcd_puts(token_message);
	*/

	PORTB|=(1<<PB1);// turn off beep
//--------------------------
	while (1)
	{	
		PORTC &=~(1 << PC2);// turn off the power cut indicator led
		if(powerDown){
			lcd_init();
			powerDown = false;
		}
		if(imp_kWh<3){
			beep();
			//relay off
		}else{
			//relay on
		}
		value =imp_kWh;
		whole = value / 3200UL;
		decimal = ((value % 3200UL) * 100UL) / 3200UL;
		

		//ultoa(imp_kWh, buffer, 10); // for unsined 32 bit
		ultoa(whole, buffer, 10);
		lcd_gotoxy(0, 4);
		lcd_puts(buffer);
		
		lcd_data('.');
		itoa(decimal, buffer, 10);
		if (decimal < 10)
		lcd_data('0');
		lcd_puts(buffer);
		lcd_puts(" kWh");
		
		ultoa(imp_kWh, buffer, 10); // for unsined 32 bit
		lcd_gotoxy(1, 3);
		lcd_puts(buffer);
		lcd_puts(" Imp");
		_delay_ms(50);
		PORTC &=~(1 << PC3);
		_delay_ms(50);
	}
}