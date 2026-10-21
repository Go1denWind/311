/*
 * uart.c
 *
 * Created: 21/10/2026 12:19:18 pm
 *  Author: wwei275
 */ 

#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <stdint.h>
void UART_init_interrupt(void);

void voltage_to_digits(int16_t adc);

void UART_start(uint8_t length);
	
#endif /* UART_H_ */