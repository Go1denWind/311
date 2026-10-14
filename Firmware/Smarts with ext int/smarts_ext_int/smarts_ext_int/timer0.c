/*
 * timer0.c
 *
 * Created: 16-Aug-26 8:28:03 PM
 *  Author: Caleb Tran
 */ 

#include "timer0.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

void timer0_init() {
	
	// CTC mode
	TCCR0A |= (1 << WGM01);
	
	// Set prescaler to 64
	TCCR0B |= (1 << CS01) | (1 << CS00);
	
	// Trigger ADC samples every 400 us (i.e. take an ADC sample every 400 us)
	OCR0A = 99;
	
	// NOTE: ADC requires 104 us (13 ADC clock cycles) to take a conversion, so maximum limit for this interrupt is 104 us.

}