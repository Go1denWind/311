/*
 * timer1.c
 *
 * Created: 16-Aug-26 9:03:25 PM
 *  Author: Caleb Tran
 */ 

#include "timer1.h"

#include <avr/io.h>
#include <stdint.h>

void timer1_init() {
	
	// CTC mode
	TCCR1B |= (1 << WGM12);
	
	// Set prescaler to 1024
	TCCR1B |= (1 << CS12) | (1 << CS10);
	
	// Configure TC1 interrupts to trigger every 1 s (i.e. display diagnostics every 1 s)
	OCR1A = 15624;
	
	// Enable interrupts
	TIMSK1 |= (1 << OCIE1A);
}