#include <avr/io.h>
#include "uart.h"

#define F_CPU 16000000UL
#define BAUD 9600UL

#define UBRR_VALUE ((F_CPU / (16UL * BAUD)) - 1)

void UART_Init(void)
{
	// Set baud rate
	UBRR0H = (uint8_t)(UBRR_VALUE >> 8);
	UBRR0L = (uint8_t)UBRR_VALUE;

	// Enable transmitter and receiver
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);

	// 8 data bits, 1 stop bit, no parity
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void UART_SendChar(char data)
{
	// Wait until transmit buffer is empty
	while (!(UCSR0A & (1 << UDRE0)));

	UDR0 = data;
}

char UART_ReceiveChar(void)
{
	// Wait until data is received
	while (!(UCSR0A & (1 << RXC0)));

	return UDR0;
}

void UART_SendString(const char *str)
{
	while (*str)
	{
		UART_SendChar(*str);
		str++;
	}
}

/* ---------------------------------------------------------
   Send unsigned integer
   --------------------------------------------------------- */

void UART_SendUInt(uint16_t value)
{
    char buffer[6];
    uint8_t i = 0;

    if (value == 0)
    {
        UART_SendChar('0');
        return;
    }

    while (value > 0)
    {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0)
    {
        UART_SendChar(buffer[--i]);
    }
}


/* ---------------------------------------------------------
   Send signed integer
   --------------------------------------------------------- */

void UART_SendInt(int16_t value)
{
    if (value < 0)
    {
        UART_SendChar('-');
        value = -value;
    }

    UART_SendUInt((uint16_t)value);
}