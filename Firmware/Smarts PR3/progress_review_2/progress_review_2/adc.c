/*
 * adc.c
 *
 * Created: 18-Aug-26 9:56:24 AM
 *  Author: Caleb Tran
 */ 

#include "adc.h"

#include <avr/io.h>
#include <stdint.h>

#define V_SENS_GAIN 3.48485f // voltage gain, mV/mV
#define V_SENS_OFFSET 8471.07f // mV

#define I_SENS_GAIN 6.8f // trans-resistance gain, mV/mA
#define I_SENS_REF 2500.0f // mV

volatile uint8_t adc_status = 0;
// 0: sample voltage next
// 1: sample current next
// 2: sample temperature next

volatile uint16_t adc_samples = 0;

volatile float capacity = 0; // %

volatile uint16_t number = 0; // debug

void adc_init() {
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Set prescaler to 16 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2);
	
	// Read from ADC1 (voltage) initially
	ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
	ADMUX |= (1 << MUX0);
	
	// Set ADC conversions to occur every 8 ms (T/C0 compare match A)
	ADCSRB |= (1 << ADTS1) | (1 << ADTS0);
	ADCSRA |= (1 << ADATE);
	
	// Disable digital input buffers on ADC pins, since only analog signals are being fed into the ADC pins
	// DIDR0 = 0x3F;
	
	// Enable ADC and conversion complete interrupts
	ADCSRA |= (1 << ADEN) | (1 << ADIE);
	
	// Start first conversion
	ADCSRA |= (1 << ADSC);

}

ISR(ADC_vect) {
	
	number = ADC;
	
	PINC = (1 << PINC4);
	
	if (adc_samples == 50) {
		ADCSRA &= ~(1 << ADATE);
		ADCSRA &= ~(1 << ADEN);
	}
	
	// capacity = (vin_to_vsc(adc_to_vin(ADC)) - 2650) / 850.0 * 100;
	
	capacity = (float)ADC;
	
	adc_samples++;
	
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

uint16_t adc_to_vin(uint16_t count) {
	return (uint16_t)(count * 5 / 1.024); // mV
}

uint16_t vin_to_vsc(uint16_t vin) {
	return (vin + V_SENS_OFFSET) / V_SENS_GAIN; // mV
}

uint16_t vin_to_isc(uint16_t vin) {
	return (vin - I_SENS_REF) / I_SENS_GAIN; // mA
}

uint16_t vin_to_tsc(uint16_t vin) {
	return 67;
	
	// TODO: implement temperature calculation.
}

float get_capacity() {
	return capacity;
}

void reset_adc_cycle() {
	adc_samples = 0;
	ADCSRA |= (1 << ADATE);
}

uint16_t get_number() {
	return number; // debug
}