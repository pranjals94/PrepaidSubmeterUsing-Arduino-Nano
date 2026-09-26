
#ifndef UART_H
#define UART_H

#include <stdint.h>

// Prevents C++ name mangling when compiled inside an Arduino project
#ifdef __cplusplus
extern "C" {
#endif

// UART Function Declarations
void UART_Init(void);
void UART_SendChar(char data);
char UART_ReceiveChar(void);
void UART_SendString(const char *str);
void UART_SendUInt(uint16_t value);
void UART_SendInt(int16_t value);

#ifdef __cplusplus
}
#endif

#endif // UART_H
