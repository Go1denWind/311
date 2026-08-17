#include <avr/io.h>
#include <string.h>

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

void usart_transmit_num(uint16_t number, uint8_t decimal_pos) {
	
	uint8_t digits[3];
	
	// Separate each digit from 'number'
	digits[0] = number / 100;
	number %= 100;
	
	digits[1] = number / 10;
	digits[2] = number % 10;
	
	for (uint8_t i = 0; i < 3; i++) {
		
		// Transmit digit
		usart_transmit(digits[i] + '0');
		
		// Transmit decimal place after the required digit
		if (i == decimal_pos) {
			usart_transmit('.');
		}
	}
	
	
	
}