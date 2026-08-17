#ifndef UART_H_
#define UART_H_
#include <stdint.h>

void usart_init(uint16_t ubrr);
void usart_transmit(uint8_t data);
void usart_transmit_array(char* msg);
void usart_transmit_num(uint16_t number, uint8_t decimal_pos);

#endif /* UART_H_ */