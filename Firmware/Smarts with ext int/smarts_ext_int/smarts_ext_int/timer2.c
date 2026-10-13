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

void timer2_init() {
	
	// Set prescaler to 1024
	TCCR0B |= (1 << CS21) | (1 << CS21) | (1 << CS20);
	
}