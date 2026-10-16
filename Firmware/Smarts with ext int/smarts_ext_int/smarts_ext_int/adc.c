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

#define T_SAMPLE_CURRENT 0.002f // s
#define T_SAMPLE_CURRENT_LONG 0.040f // s

#define V_SENS_GAIN 3.48485f // voltage gain, mV/mV
#define V_SENS_OFFSET 8471.07f // mV

#define I_SENS_GAIN 6.8f // trans-resistance gain, mV/mA
#define I_SENS_REF 2500.0f // mV

volatile uint16_t temp_lookup[101] = {4599, 4579, 4556, 4531, 4503,
									4471, 4446, 4418, 4387, 4353,
									4314, 4283, 4249, 4212, 4170, 
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
									664, 644, 625, 605, 586,
									566, 549, 533, 56, 499,
									482};
volatile uint8_t temp_lookup_size = 100;

volatile uint8_t adc_status = 3;
// 0: sample temperature next
// 1: sample voltage next
// 2: sample current next
// 3: "sample" touch next (handled by external interrupts and T/C2)

volatile uint16_t adc_samples = 0;

volatile uint8_t dsp_flag = 0;
volatile uint8_t usart_flag = 0;

void adc_init() {
	
	// Select Vcc as reference voltage
	ADMUX |= (1 << REFS0);
	
	// Set prescaler to 128 (i.e. ADC clock frequency of 125 kHz)
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	
	// Read from ADC1 (temperature) initially
	ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1);
	ADMUX |= (1 << MUX0);
	
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

ISR(ADC_vect) {
	
	// Manually clear interrupt flag bit so next rising edge of trigger source can be detected
	TIFR0 = (1 << OCF0A);
	
	// Mark calculations as "to-do", for main routine
	dsp_flag = 1;
	
	// Update ADC status variable so appropriate processing is done for next sample
	adc_status++;
	if (adc_status >= 3) {
		adc_status = 0;
	}
	
	// Stop after 1440 samples (960 ms) and start UART, sampling will resume at the next 1 second interrupt
	adc_samples++;
	if (adc_samples >= 1440) {
	// if (adc_samples >= 60) { // debug
		ADCSRA &= ~(1 << ADATE);
		usart_flag = 1;
	}
	
}

void update_adc_status() {
	
	// Update ADC status variable so appropriate processing is done for next sample
	adc_status++;
	if (adc_status >= 3) {
		adc_status = 0;
	}
	
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

uint16_t adc_to_vin(uint16_t count) {
	return (uint16_t)(count * 5 / 1.024); // mV
}

uint16_t vin_to_vsc(uint16_t vin) {
	return (vin + V_SENS_OFFSET) / V_SENS_GAIN; // mV
}

int16_t vin_to_isc(uint16_t vin) {
	return ((int16_t)vin - (int16_t)I_SENS_REF) / I_SENS_GAIN; // mA
}

int8_t vin_to_tsc(uint16_t vin) {
	
	// Find index of lookup value closest to V_in via "SAR-type" algorithm.
	
	uint8_t no_of_guesses = 0;
	
	uint8_t guess = 50;
	
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
	
	return (int8_t)guess - 20;
}

float vsc_to_soc(uint16_t vsc) {
	float soc = ((int16_t)vsc - 2650) / 850.0 * 1000; // % * 10
	
	// Bound capacity between 0% and 100%
	if (soc > 1000) {
		return 1000;
	} else if (soc < 0) {
		return 0;
	} else {
		return soc;
	}
}

float isc_to_soc(int16_t isc, float capacity) {
	
	float soc;
	
	if (adc_samples < 6) {
		soc = capacity + ((float)isc * T_SAMPLE_CURRENT_LONG) / 127.5f;
	} else {
		soc = capacity + ((float)isc * T_SAMPLE_CURRENT) / 127.5f; // TODO: change back to integer calculations to make this faster
	}
	
	// Bound capacity between 0% and 100%
	if (soc > 1000) {
		return 1000;
	} else if (soc < 0) {
		return 0;
	} else {
		return soc;
	}
}

void reset_adc_cycle() {
	
	adc_samples = 0;
	ADCSRA |= (1 << ADATE);
}

uint8_t usart_to_do() {
	return usart_flag;
}

void usart_stop() {
	usart_flag = 0;
}

uint8_t dsp_to_do() {
	return dsp_flag;
}

void dsp_stop() {
	dsp_flag = 0;
}