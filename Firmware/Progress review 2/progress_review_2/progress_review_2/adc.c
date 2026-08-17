/*
 * adc.c
 *
 * Created: 18-Aug-26 9:56:24 AM
 *  Author: Caleb Tran
 */ 

#include "adc.h"

#include <avr.io/h>
#include <stdint.h>

void adc_init() {
	
	// Configure ADC pins to be input
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1);
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Enable ADC
	ADCSRA |= (1 << ADEN);
	
	// Start first conversion
	ADCSRA |= (1 << ADSC);
	
	// Set prescaler to 16 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2);
	
	// Disable digital input buffers on ADC pins, since only analog signals are being fed into the ADC pins
	DIDR0 = 0x3F;

}