/*
 * coulomb_counting_test.c
 *
 * Created: 13-Aug-26 9:26:29 AM
 * Author : Caleb Tran
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#include "uart.h"
#include "timer0.h"
#include "timer1.h"
#include "adc.h"

volatile uint8_t usart_to_do = 0;

volatile uint8_t debug = 0;

int main(void)
{
	
	usart_init(12); // 76800 baud rate
	timer0_init();
	timer1_init();
	adc_init(); // NOTE: might block UART on Proteus
	
	sei();
	
	// Configure I/O pins
	DDRB |= (1 << DDB0) | (1 << DDB3) | (1 << DDB4) | (1 << DDB5);
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC2) & ~(1 << DDC3);
	DDRD |= (1 << DDD1) | (1 << DDD4) | (1 << DDD5) | (1 << DDD6) | (1 << DDD7); // TODO: check required I/O for SPI and RESET pins
	
	// Enable debug output pins
	DDRB |= (1 << DDB1) | (1 << DDB2);
	DDRC |= (1 << DDC4) | (1 << DDC5);
	DDRD |= (1 << DDD2) | (1 << DDD3);
	
    while (1)
    {
		
		// If one second has passed, print battery capacity to UART
		if (usart_to_do == 1) {
			
			usart_transmit_diagnostics(get_voltage(), get_current(), get_temperature(), get_capacity());
			
			// Clear USART flag
			usart_to_do = 0;
		}
    }
}

ISR(TIMER1_COMPA_vect) {
	
	// Set USART flag, main function will transmit diagnostics
	usart_to_do = 1;
	
	reset_adc_cycle();
	
	// Manually interrupt flag bit so next rising edge of trigger source can be detected
	TIFR0 = (1 << OCF0A);
	
}