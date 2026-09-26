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
#include "adc.h"

#define T_SAMPLE 0.008 // s

// Assume initial full capacity (3.8 V)
volatile float capacity; // %

/*
// Fake array of samples to loop through (in mA, supercap discharging)
volatile double fake_samples[4] = {250, 250, 250, 250};
volatile uint8_t i = 0;
*/

volatile uint8_t usart_to_do = 0;

int main(void)
{
	usart_init(12);
	timer0_init();
	timer1_init();
	adc_init(); // NOTE: might block UART on Proteus
	
	sei();
	
	// Configure I/O pins
	DDRB |= (1 << DDB0) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5);
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC2) & ~(1 << DDC3);
	DDRD |= (1 << DDD1); // TODO: check required I/O for SPI and RESET pins
	
	// Enable debug output pins
	DDRB |= (1 << DDB1) | (1 << DDB2);
	DDRC |= (1 << DDC4) | (1 << DDC5);
	DDRD |= (1 << DDD2) | (1 << DDD3);
	
    while (1)
    {
		
		// If one second has passed, print battery capacity to UART
		if (usart_to_do == 1) {
			
			// Toggle debug PB2 pin
			PINB = (1 << PINB2);
			
			usart_transmit_array("C: ");
			
			capacity = get_capacity();
			usart_transmit_num((uint16_t)(get_number()), 3);
			usart_transmit('\%');
			usart_transmit('\n');
			
			// Clear USART flag
			usart_to_do = 0;
		}
    }
}

/*
ISR(TIMER0_COMPA_vect) {
	
	// Calculate new capacity via coulomb counting
	capacity -= fake_samples[i] * T_SAMPLE;
	
	// Increment i to use next "fake" sample
	i = (i + 1) % 4;
	
	// Toggle debug PB0 pin
	PINB = (1 << PINB0);
	
}
*/

ISR(TIMER1_COMPA_vect) {
	
	// Set USART flag, main function will transmit diagnostics
	usart_to_do = 1;
	
	reset_adc_cycle();
}