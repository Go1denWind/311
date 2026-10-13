/*
 * timer2.c
 *
 * Created: 13-Oct-26 7:34:49 PM
 *  Author: whoyo
 */ 

#include "timer2.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

void timer2_start() {
	
	// Clear T/C2 value
	TCNT2 = 0;
	
	// Set prescaler to 1024
	TCCR0B |= (1 << CS21) | (1 << CS21) | (1 << CS20);
	
}

void timer2_clear() {
	
}