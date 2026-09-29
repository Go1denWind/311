/*
 * adc.c
 *
 * Created: 18-Aug-26 9:56:24 AM
 *  Author: Caleb Tran
 */ 

#include "adc.h"

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define V_SENS_GAIN 3.48485f // voltage gain, mV/mV
#define V_SENS_OFFSET 8471.07f // mV

#define I_SENS_GAIN 6.8f // trans-resistance gain, mV/mA
#define I_SENS_REF 2500.0f // mV

volatile uint16_t temp_lookup[81] = {4314, 4283, 4249, 4212, 4170, 
									4124, 4087, 4047, 4003, 3955, 
									3903, 3860, 3815, 3765, 3712, 
									3653, 3606, 3556, 3501, 3443, 
									3380, 3329, 3276, 3219, 3157, 
									3092, 3039, 2984, 2925, 2862, 
									2796, 2742, 2686, 2627, 2565, 
									2500, 2447, 2393, 2335, 2276, 
									2213, 2162, 2109, 2054, 1997, 
									1938, 1890, 1841, 1790, 1737, 
									1683, 1640, 1595, 1550, 1503, 
									1455, 1416, 1376, 1335, 1293, 
									1251, 1216, 1181, 1145, 1108, 
									1071, 1040, 1009, 978, 946, 
									914, 888, 861, 834, 807, 
									779, 757, 734, 711, 687, 
									664};
volatile uint8_t temp_lookup_size = 80;

volatile uint8_t adc_status = 0;
// 0: sample temperature next
// 1: sample voltage next
// 2: sample current next
// 3: sample touch oscillator next

volatile uint16_t adc_samples = 0;

volatile int8_t temperature = 0;
volatile uint16_t voltage = 0;
volatile int16_t current = 0;

// Status variables for touch detection
volatile uint8_t osc_startup = 0; // set to 1 when first rising edge is detected.
volatile uint8_t touch_curr = 0;
volatile uint8_t touch_ready = 1; // equal 1 if electrode was not being touched in previous osc cycle
volatile uint16_t adc_prev = 0;
volatile uint16_t touch_count = 0;

volatile float capacity = 0;

void adc_init() {
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Set prescaler to 128 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Read from ADC0 (temperature) initially
	ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
	
	// Set ADC conversions to occur every 8 ms (T/C0 compare match A)
	ADCSRB |= (1 << ADTS1) | (1 << ADTS0);
	ADCSRA |= (1 << ADATE);
	
	// Disable digital input buffers on ADC pins, since only analog signals are being fed into the ADC pins
	DIDR0 = 0x0F;
	
	// Enable ADC and conversion complete interrupts
	ADCSRA |= (1 << ADEN) | (1 << ADIE);
	
	// Start first conversion
	ADCSRA |= (1 << ADSC);

}

void adc_init_debug() {
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Enable ADC
	ADCSRA |= (1 << ADEN);
	
	// Set prescaler to 128 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
}

uint16_t adc_read_debug(uint8_t channel) {
	
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

ISR(ADC_vect) {
	
	PINC = (1 << PINC4);
	
	// Manually interrupt flag bit so next rising edge of trigger source can be detected
	TIFR0 = (1 << OCF0A);
	
	if (temperature_next()) {
		temperature = vin_to_tsc(adc_to_vin(ADC));
		ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
		ADMUX |= (1 << MUX0);
	} else if (voltage_next()) {
		voltage = vin_to_vsc(adc_to_vin(ADC));
		ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
		ADMUX |= (1 << MUX1);
	} else if (current_next()) {
		current = vin_to_isc(adc_to_vin(ADC));
		ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
		ADMUX |= (1 << MUX1) | (1 << MUX1);
	} else if (touch_next()) {
		ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
		
		touch_count++;
		
		if ((ADC > 614) && (adc_prev < 410)) {
			
			if (osc_startup == 0) {
				
				osc_startup = 1;
				touch_count = 0;
				
			} else {
				
				if (touch_count > 135) {
					
					touch_curr = 1;
					
					if (touch_ready) { // if this is a new touch
						touch_ready = 0;
						// Toggle output isolation switch
						PIND = (1 << PIND4);
					} else { // not being touched
						touch_curr = 0;
						touch_ready = 1; // prepare to detect new touch
					}
					
					touch_count = 0;
					
				}
			}
		}
		
		adc_prev = ADC;
		
	}
	
	adc_status++;
	adc_samples++;
	
	if (adc_status >= 4) {
		adc_status = 0;
	}
	
	if (adc_samples >= 120) {
		ADCSRA &= ~(1 << ADATE);
	}
	
	PINC = (1 << PINC4);
	
}

uint8_t temperature_next() {
	return (adc_status == 0);
}

uint8_t voltage_next() {
	return (adc_status == 1);
}

uint8_t current_next() {
	return (adc_status == 2);
}

uint8_t touch_next() {
	return (adc_status == 3);
}

uint16_t adc_to_vin(uint16_t count) {
	return (uint16_t)(count * 5 / 1.024); // mV
}

uint16_t vin_to_vsc(uint16_t vin) {
	return (vin + V_SENS_OFFSET) / V_SENS_GAIN; // mV
}

int16_t vin_to_isc(uint16_t vin) {
	return (vin - I_SENS_REF) / I_SENS_GAIN; // mA
}

int8_t vin_to_tsc(uint16_t vin) {
	
	// Find index of lookup value closest to V_in via "SAR-type" algorithm.
	
	uint8_t no_of_guesses = 0;
	
	uint8_t guess = 40;
	
	uint8_t min = 0;
	uint8_t max = temp_lookup_size;
	
	uint8_t prev_guess;
	
	while(1) {

		no_of_guesses++;

		prev_guess = guess;
		
		if (temp_lookup[guess] < vin) {
			// If look-up value is too low, guess a lower index next time
			max = guess;
			guess = (guess + min) / 2;
			} else if (temp_lookup[guess] == vin) {
			break;
			} else {
			// If look-up value is too high, guess a higher index next time
			min = guess;
			guess = (guess + max) / 2;
		}

		if (prev_guess == guess) {
			break;
		}

		if (no_of_guesses > 10) {
			return -100; // return value "obviously" out of plausible range -- error has occurred.
		}
	}
	
	return (int8_t)guess - 10;
}

int8_t get_temperature() {
	return temperature;
}

uint16_t get_voltage() {
	return voltage;
}

int16_t get_current() {
	return current;
}

float get_capacity() {
	return capacity;
}

void reset_adc_cycle() {
	
	adc_samples = 0;
	ADCSRA |= (1 << ADATE);
}