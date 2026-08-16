/*
 * coulomb_counting_test.c
 *
 * Created: 13-Aug-26 9:26:29 AM
 * Author : Caleb Tran
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 2000000UL

#include "uart.h"
#include "timer0.h"
#include "timer1.h"

#define T_SAMPLE 0.008 // s

// Assume initial full capacity (3.8 V)
volatile double capacity = 64800; // mAs

// Fake array of samples to loop through (in mA, supercap discharging)
volatile double fake_samples[4] = {1, 0, 0, 0};
volatile uint8_t i = 0;

volatile uint8_t usart_to_do = 0;

int main(void)
{

	sei();
	usart_init(9600);
	timer0_init();
	timer1_init();
	
	// Set PB0 and PB1 to output (debugging)
	DDRB |= (1 << DDB0) | (1 << DDB1);
	
	// Set USART TX pin to output
	DDRD |= (1 << DDD1);
	
    while (1)
    {
		// If one second has passed, print battery capacity to UART
		if (usart_to_do) {
			uint16_t number = capacity / 100;
			usart_transmit_array("hi");
			
			// Clear USART flag
			usart_to_do = 0;
		}
    }
}

ISR(TIMER0_COMPA_vect) {
	
	// Calculate new capacity via coulomb counting
	capacity -= fake_samples[i] * T_SAMPLE;
	
	// Increment i to use next "fake" sample
	i = (i + 1) % 4;
	
	// Toggle debug PB0 pin
	PINB = (1 << PINB0);
	
}

ISR(TIMER1_COMPA_vect) {
	
	// Toggle debug PB1 pin
	PINB = (1 << PINB1);
	
	// Set USART flag, main function will transmit diagnostics
	usart_to_do = 1;
}