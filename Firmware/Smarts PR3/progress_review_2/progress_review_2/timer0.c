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
	
	// Set prescaler to 1024
	TCCR0B |= (1 << CS02) | (1 << CS00);
	
	// Trigger ADC samples every 8 ms (i.e. take a sample every 8 ms)
	OCR0A = 124;

}