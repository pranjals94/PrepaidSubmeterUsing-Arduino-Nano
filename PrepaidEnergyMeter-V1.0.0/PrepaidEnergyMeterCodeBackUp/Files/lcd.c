/*
 * lcd.c
 *
 * Created: 09-09-2026 06:37:46:PM
 *  Author: pranj
 */ 
#define F_CPU 16000000UL
#include "lcd.h"
#include <util/delay.h>

// --------------------------------------------------
// Send one 4-bit nibble
// --------------------------------------------------
void lcd_send_nibble(uint8_t nibble)
{
	// Clear only PD4-PD7
	LCD_DATA_PORT &= ~((1 << D4) |
	(1 << D5) |
	(1 << D6) |
	(1 << D7));

	// Put nibble on D4-D7
	if (nibble & 0x01)
	LCD_DATA_PORT |= (1 << D4);

	if (nibble & 0x02)
	LCD_DATA_PORT |= (1 << D5);

	if (nibble & 0x04)
	LCD_DATA_PORT |= (1 << D6);

	if (nibble & 0x08)
	LCD_DATA_PORT |= (1 << D7);

	// Enable pulse
	LCD_CONTROL_PORT |= (1 << EN);
	_delay_us(1);

	LCD_CONTROL_PORT &= ~(1 << EN);
	_delay_us(100);
}


// --------------------------------------------------
// Send command
// --------------------------------------------------
void lcd_command(uint8_t cmd)
{
	// RS = 0 -> command
	LCD_CONTROL_PORT &= ~(1 << RS);

	// High nibble
	lcd_send_nibble(cmd >> 4);

	// Low nibble
	lcd_send_nibble(cmd & 0x0F);

	if (cmd == 0x01 || cmd == 0x02)
	_delay_ms(2);
}


// --------------------------------------------------
// Send character
// --------------------------------------------------
void lcd_data(uint8_t data)
{
	// RS = 1 -> data
	LCD_CONTROL_PORT |= (1 << RS);

	// High nibble
	lcd_send_nibble(data >> 4);

	// Low nibble
	lcd_send_nibble(data & 0x0F);
}


// --------------------------------------------------
// Send string
// --------------------------------------------------
void lcd_puts(const char *str)
{
	while (*str)
	{
		lcd_data(*str);
		str++;
	}
}


// --------------------------------------------------
// LCD initialization
// --------------------------------------------------
void lcd_init(void)
{
	// PD0, PD1, PD2, PD4-PD7 as outputs
	LCD_CONTROL_DDR |= (1 << RS) |(1 << EN);

	LCD_DATA_DDR |= (1 << D4) |
	(1 << D5) |
	(1 << D6) |
	(1 << D7);

	// Initial states
	LCD_CONTROL_PORT &= ~(1 << RS);
	LCD_CONTROL_PORT &= ~(1 << EN);

	_delay_ms(40);


	// ------------------------------------------
	// HD44780 4-bit initialization
	// ------------------------------------------

	// Send 0x03
	lcd_send_nibble(0x03);
	_delay_ms(5);

	// Send 0x03
	lcd_send_nibble(0x03);
	_delay_us(150);

	// Send 0x03
	lcd_send_nibble(0x03);
	_delay_us(150);

	// Select 4-bit mode
	lcd_send_nibble(0x02);
	_delay_us(150);


	// ------------------------------------------
	// LCD configuration
	// ------------------------------------------

	// 4-bit, 2 lines, 5x8 font
	lcd_command(0x28);

	// Display ON, cursor OFF, blink OFF
	lcd_command(0x0C);

	// Entry mode: cursor increments
	lcd_command(0x06);

	// Clear display
	lcd_command(0x01);
	_delay_ms(2);
}


// --------------------------------------------------
// Set cursor
// --------------------------------------------------
void lcd_gotoxy(uint8_t row, uint8_t column)
{
	uint8_t address;

	if (row == 0)
	address = 0x00 + column;
	else
	address = 0x40 + column;

	lcd_command(0x80 | address);
}
