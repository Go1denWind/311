/*
 * ext_int.c
 *
 * Created: 13-Oct-26 7:44:04 PM
 *  Author: Caleb Tran
 */ 

#include "ext_int.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

void ext_int_init() {
	
	// Configure INT0 interrupts to trigger on rising edge
	EICRA |= (1 << ISC00) | (1 << ISC01);
	
	// Enable interrupts on INT0 pin
	EIMSK |= (1 << INT0);
	
}