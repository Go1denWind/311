/*
 * ProgressReview_UART.c
 *
 * Created: 26/08/2025 9:54:43 am
 * Author : Caleb Tran
 */ 

#include <avr/io.h>
#include <stdint.h>
#include "uart.h"
#include <util/delay.h>

#define F_CPU 2000000UL

#define RMSVoltage 17.5
#define PeakCurrent 420
#define Power 0

// Function prototypes
void extract_digits(uint16_t number, uint8_t digits[]);
int digit_to_ascii(uint8_t digit);

int main(void)
{
	
	// Initialise baud rate
    usart_init(12);
	
	// Initialise arrays to store digits of data to be transmitted
	uint8_t v_digits[3];
	uint8_t i_digits[3];
	uint8_t p_digits[3];
	
    while (1)
    {
		
		// Delay for data to be transmitted once every second
		_delay_ms(1000);
		
		// Extract digits of RMS voltage, peak current, and power
		extract_digits(RMSVoltage * 10, v_digits);
		extract_digits(PeakCurrent, i_digits);
		extract_digits(Power * 100, p_digits);
		
		// Transmit RMS voltage message
		usart_transmit_array("RMS Voltage is: ");
		
		// Transmit RMS voltage value, followed by a new line
		uint8_t v_tens = digit_to_ascii(v_digits[0]);
		uint8_t v_ones = digit_to_ascii(v_digits[1]);
		uint8_t v_tenths = digit_to_ascii(v_digits[2]);
		
		usart_transmit(v_tens);
		usart_transmit(v_ones);
		usart_transmit(46); // decimal point
		usart_transmit(v_tenths);
		
		usart_transmit(10);
		usart_transmit(13);
		
		// Transmit peak current message
		usart_transmit_array("Peak Current is: ");
		
		// Transmit peak current value, followed by a new line
		uint8_t i_hundreds = digit_to_ascii(i_digits[0]);
		uint8_t i_tens = digit_to_ascii(i_digits[1]);
		uint8_t i_ones = digit_to_ascii(i_digits[2]);
		
		usart_transmit(i_hundreds);
		usart_transmit(i_tens);
		usart_transmit(i_ones);
		
		usart_transmit(10);
		usart_transmit(13);
		
		// Transmit power message
		usart_transmit_array("Power is: ");
		
		// Transmit power value, followed by two new lines
		uint8_t p_ones = digit_to_ascii(p_digits[0]);
		uint8_t p_tenths = digit_to_ascii(p_digits[1]);
		uint8_t p_hundredths = digit_to_ascii(p_digits[2]);
		
		usart_transmit(p_ones);
		usart_transmit(46); // decimal point
		usart_transmit(p_tenths);
		usart_transmit(p_hundredths);
		
		usart_transmit(10);
		usart_transmit(13);
		
		usart_transmit(10);
		usart_transmit(13);
    }
}

void extract_digits(uint16_t number, uint8_t digits[]) {
	
	// Function to extract digits of a three-digit
	// number and place them in a given array.

	digits[2] = number % 10; // ones

	number /= 10;
	digits[1] = number % 10; // tens
	digits[0] = number / 10; // hundreds
	
}

int digit_to_ascii(uint8_t digit) {
	
	// Function that returns the corresponding
	// ASCII value of a given digit.
	
	return digit + 48;
}