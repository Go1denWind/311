/*
 * uart.c
 *
 * Created: 26/08/2025 9:56:02 am
 *  Author: Caleb Tran
 */ 

#include "uart.h"

void usart_init(uint16_t ubrr) {
	
	// Enable USART transmitter
	UCSR0B |= (1 << TXEN0);
	
	// Extract 8 LSBs and write them to UBRR0L
	UBRR0L = ubrr % 256;
	
	// Extract 4 MSBs and write them to UBRR0H
	UBRR0H |= (ubrr / 256);
	
}

void usart_transmit(uint8_t data) {
	
	while (1) {
		
		if (UCSR0A & (1 << UDRE0)) {
			
			// Write data to UDR0 register to transmit if ready, then exit the function
			UDR0 = data;
			break;
			
			} else {
			
			// If UDR0 is not ready, wait and try again
			continue;
		}
	}
}

void usart_transmit_array(char* msg) {
	
	// Loop through and transmit each byte in the array
	for (uint8_t i = 0; i < strlen(msg); i++){
		usart_transmit(msg[i]);
	}
}

void usart_transmit_mv(uint16_t mv) {
	
	// Extract digits from given value in mV
	uint16_t num = mv;
	uint8_t ones = num % 10;
	
	num /= 10;
	uint8_t tens = num % 10;
	
	num /= 10;
	uint8_t hundreds = num % 10;
	uint8_t thousands = num / 10;
	
	// Transmit digits via UART
	usart_transmit(thousands + '0');
	usart_transmit(hundreds + '0');
	usart_transmit(tens + '0');
	usart_transmit(ones + '0');
	
}