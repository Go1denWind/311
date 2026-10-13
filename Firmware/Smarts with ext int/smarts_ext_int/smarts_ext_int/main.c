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

int main(void)
{
	
	usart_init(12); // 76800 baud rate
	timer0_init();
	timer1_init();
	adc_init();
	
	sei();
	
	// Configure I/O pins
	DDRB |= (1 << DDB0);
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC2) & ~(1 << DDC3);
	DDRC |= (1 << DDC4) | (1 << DDC5);
	DDRD |= (1 << DDD6) | (1 << DDD7);
	
	// Enable debug output pins
	DDRB |= (1 << DDB4) | (1 << DDB5);
	DDRD |= (1 << DDD2) | (1 << DDD3);
	
	// TODO: update I/O registers to match new PCB layout
	
    while (1)
    {
		
		// Update LEDs based on capacity reading
		if (get_capacity() > 666) {
			PORTD |= (1 << PORTD6) | (1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else if (get_capacity() > 333) {
			PORTD &= ~(1 << PORTD6);
			PORTD |= (1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else if (get_capacity() > 10) {
			PORTD &= ~(1 << PORTD6) & ~(1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else {
			PORTD &= ~(1 << PORTD6) & ~(1 << PORTD7);
			PORTB &= ~(1 << PORTB0);
		}
		
		// If one second has passed, print battery capacity to UART
		if (usart_to_do()) {
			
			usart_transmit_diagnostics(get_voltage(), get_current(), get_temperature(), get_capacity());
			
			// Clear USART flag
			usart_stop();
		}
    }
}

ISR(TIMER1_COMPA_vect) {
	
	reset_adc_cycle();
	
	// Manually interrupt flag bit so next rising edge of trigger source can be detected
	TIFR0 = (1 << OCF0A);
	
}