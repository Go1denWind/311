/*
 * adc.c
 *
 * Created: 18-Aug-26 9:56:24 AM
 *  Author: Caleb Tran
 */ 

#include "adc.h"

#include <avr/io.h>
#include <stdint.h>

volatile uint8_t adc_status = 0;
// 0: sample voltage next
// 1: sample current next
// 2: sample temperature next

volatile float sc_charge = 0; // mAs

void adc_init() {
	
	// Configure ADC pins to be input
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1);
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Read from ADC0 (voltage) initially
	ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
	
	// Set prescaler to 16 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2);
	
	// Set ADC conversions to occur every 8 ms (T/C0 compare match A)
	ADCSRB |= (1 << ADTS1) | (1 << ADTS0);
	
	// Disable digital input buffers on ADC pins, since only analog signals are being fed into the ADC pins
	DIDR0 = 0x3F;
	
	// Enable ADC and conversion complete interrupts
	ADCSRA |= (1 << ADEN) | (1 << ADIE);
	
	// Start first conversion
	ADCSRA |= (1 << ADSC);
	
	

}

ISR(ADC_vect) {
	
	// Toggle debug pin
	PINB = (1 << PINB0);
	
	if (voltage_next()) { // Note: assumes SC voltage is not amplified or shifted before going into the ADC.
		
		float sc_voltage = adc_to_vin(ADC);
		
		// Convert SC voltage to charge inside SC (assume straight-line relationship between SC voltage and SC charge, and assuming no current through SC, initially)
		sc_charge = 64800 * (sc_voltage - 2.5) / 1.3; // Note: find a way to put this outside of ISR to reduce blocking?
		
		// Change ADC channel to read SC current
		ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1);
		ADMUX |= (1 << MUX0);
		
		// Change status variable so current routine is run on next ADC conversion
		adc_status = 1;
	}
}

uint8_t voltage_next() {
	return (adc_status == 0);
}

uint8_t current_next() {
	return (adc_status == 1);
}

uint8_t temperature_next() {
	return (adc_status == 2);
}

float adc_to_vin(uint16_t count) {
	return count * 5 / 1024.0;
}

float get_sc_charge() {
	return sc_charge;
}