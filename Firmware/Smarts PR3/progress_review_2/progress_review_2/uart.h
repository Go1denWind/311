#ifndef UART_H_
#define UART_H_
#include <stdint.h>

void usart_init(uint16_t ubrr);
void usart_transmit(uint8_t data);
void usart_transmit_array(char* msg);
void usart_transmit_num_3sf(uint16_t number, uint8_t decimal_pos);

void usart_transmit_voltage(uint16_t number);
void usart_transmit_current(uint16_t number);
void usart_transmit_temp(int8_t number);
void usart_transmit_soc(uint16_t number);

void usart_transmit_diagnostics(uint16_t voltage, uint16_t current, int8_t temp, uint16_t soc);

#endif /* UART_H_ */