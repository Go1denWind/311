/*
 * timer1.c
 *
 * Created: 16-Aug-26 9:03:25 PM
 *  Author: whoyo
 */ 

#include "timer1.h"

#include <avr/io.h>
#include <stdint.h>

void timer1_init() {
	
	// CTC mode
	TCCR1B |= (1 << WGM12);
	
	// Set prescaler to 64
	TCCR1B |= (1 << CS11) | (1 << CS10);
	
	// Configure TC1 interrupts to trigger every 1 s (i.e. display diagnostics every 1 s)
	OCR1A = 31249;
	
	// Enable interrupts
	TIMSK1 |= (1 << OCIE1A);
}