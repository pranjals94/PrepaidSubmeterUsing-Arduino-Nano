#ifndef LCD_H
#define LCD_H

#include <avr/io.h>

#define LCD_DATA_PORT     PORTD
#define LCD_DATA_DDR      DDRD

#define LCD_CONTROL_PORT  PORTC
#define LCD_CONTROL_DDR   DDRC

#define RS PC0
#define EN PC1

#define D4 PD4
#define D5 PD5
#define D6 PD6
#define D7 PD7

void lcd_send_nibble(uint8_t nibble);
void lcd_command(uint8_t cmd);
void lcd_data(uint8_t data);
void lcd_puts(const char *str);
void lcd_init(void);
void lcd_gotoxy(uint8_t row, uint8_t column);
#endif
