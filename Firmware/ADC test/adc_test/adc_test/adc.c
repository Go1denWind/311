/*
 * adc.c
 *
 * Created: 1/09/2025 9:33:43 pm
 *  Author: whoyo
 */ 

#include "adc.h"
#include <avr/io.h>

void adc_init() {
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Enable ADC
	ADCSRA |= (1 << ADEN);
	
	// Enable interrupts when ADC conversion is complete
	// ADCSRA |= (1 << ADIE);
	
	// Set prescaler to /16 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2);
}

uint16_t adc_read(uint8_t channel) {
	
	// Set ADC channel to read, using the given argument
	ADMUX &= 0xF0;
	ADMUX |= channel;
	
	// Start ADC conversion
	ADCSRA |= (1 << ADSC);
	
	// When the conversion is complete, return the ADC result
	while (1) {
		
		if (ADCSRA & (1 << ADIF)) {
			ADCSRA |= (1 << ADIF); // clear flag
			return ADC;
		}
		
	}
	
}

uint16_t adc_convert_mv(uint16_t value) {
	
	// Convert the given raw ADC count to the
	// corresponding voltage in mV.
	
	float result = (uint32_t)value * 5000 / 1024;
	
	// Return the rounded result
	if ((uint16_t)(result * 10) % 10 < 5) {
		return (uint16_t)result;
	} else {
		return (uint16_t)result + 1;
	}
}