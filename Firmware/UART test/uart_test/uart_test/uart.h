/*
 * uart.h
 *
 * Created: 26/08/2025 9:57:18 am
 *  Author: Caleb Tran
 */ 


#ifndef UART_H_
#define UART_H_
#include <stdint.h>
#include <avr/io.h>

void usart_init(uint16_t ubrr);
void usart_transmit(uint8_t data);
void usart_transmit_array(char* msg);

#endif /* UART_H_ */