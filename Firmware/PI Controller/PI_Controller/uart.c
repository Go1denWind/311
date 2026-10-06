/*
 * uart.c
 *
 * Created: 7/10/2026 10:04:22 am
 *  Author: wwei275
 */ 

#include <avr/io.h>
#include <stdint.h>

#define BAUD_RATE 9600
#define F_CPU 16000000UL

void UART_init_interrupt(void){
	UCSR0B |= 1 << TXEN0 | 1 << RXEN0 | 1 << TXCIE0 | 1 << RXCIE0;
	//Enable transmit, receive, and interrupt, UCSZ02=0 (default - 8 data bits))
	UCSR0C |= 3 << UCSZ00;      //UMSEL0[1..0]=00 (default - UART mode), UPM0[1..0]=00 (default - no parity),
	//USBS0=0 (default - 1 stop bit), UCSZ0[1..0]=11 (default - 8 data bits)
	UBRR0 = F_CPU / ((uint32_t)16 * BAUD_RATE) - 1;
	//Set UBRR0 as per baud rate formula
}