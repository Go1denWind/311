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
	
	// Only prints first character in Proteus for some reason
	
	// Loop through and transmit each byte in the array
	for (uint8_t i = 0; i < (uint8_t)(strlen(msg)); i++){
		usart_transmit(msg[i]);
	}
}

void usart_transmit_num_3sf(uint16_t number, uint8_t decimal_pos) {
	
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

void usart_transmit_voltage(uint16_t number) {
	usart_transmit_array("Voltage:      ");
	usart_transmit_num_3sf(number/10, 0);
	usart_transmit_array(" V\n\r");
}

void usart_transmit_current(int16_t number) {
	usart_transmit_array("Current:      ");
	
	int16_t transmit_num;
	
	if (number < 0) {
		usart_transmit('-');
		transmit_num = number * -1;
		} else {
		usart_transmit(' ');
		transmit_num = number;
	}
	
	usart_transmit_num_3sf(transmit_num, 3);
	usart_transmit_array(" mA\n\r");
}

void usart_transmit_temp(int8_t number) {
	
	usart_transmit_array("Temperature: ");
	
	int8_t transmit_num;
	
	if (number < 0) {
		usart_transmit('-');
		transmit_num = number * -1;
	} else {
		usart_transmit(' ');
		transmit_num = number;
	}
	
	usart_transmit(transmit_num/10 + '0');
	usart_transmit(transmit_num%10 + '0');
	
	usart_transmit_array(" C\n\r");
}

void usart_transmit_soc(uint16_t number) {
	usart_transmit_array("SOC:         ");
	
	if (number == 1000) {
		usart_transmit('1');
		usart_transmit_num_3sf(0, 1);
	} else {
		usart_transmit(' ');
		usart_transmit_num_3sf(number, 1);
	}
	
	usart_transmit_array("\%\n\r");
}

void usart_transmit_diagnostics(uint16_t voltage, int16_t current, int8_t temp, uint16_t soc) {
	usart_transmit_voltage(voltage);
	usart_transmit_current(current);
	usart_transmit_temp(temp);
	usart_transmit_soc(soc);
	usart_transmit_array("\n\r");
}

