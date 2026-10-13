/*
 * coulomb_counting_test.c
 *
 * Created: 13-Aug-26 9:26:29 AM
 * Author : Caleb Tran
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL

#include "uart.h"
#include "timer0.h"
#include "timer1.h"
#include "timer2.h"
#include "ext_int.h"
#include "adc.h"

volatile int8_t temperature = 0;
volatile uint16_t voltage = 0;
volatile int16_t current = 0;

volatile float capacity = 0; // % * 10
// TODO: calculate capacity in terms of SOE instead of SOC?

// Status variables for touch detection
volatile uint8_t osc_startup = 0; // set to 1 when first rising edge is detected.
volatile uint8_t touch_curr = 0;
volatile uint8_t touch_ready = 1; // equal 1 if electrode was not being touched in previous osc cycle
volatile uint16_t adc_prev = 0;
volatile uint16_t touch_count = 0;

volatile uint8_t no_load = 0;
volatile uint8_t soc_startup = 0;

uint16_t count;


int main(void)
{
	
	usart_init(12); // 76800 baud rate
	timer0_init();
	timer1_init();
	adc_init();
	
	sei();
	
	// Configure I/O pins
	DDRB |= (1 << DDB0);
	DDRC &= ~(1 << DDC0) & ~(1 << DDC1) & ~(1 << DDC2) & ~(1 << DDC3);
	DDRC |= (1 << DDC4) | (1 << DDC5);
	DDRD |= (1 << DDD6) | (1 << DDD7);
	
	// Enable debug output pins
	DDRB |= (1 << DDB4) | (1 << DDB5);
	DDRD |= (1 << DDD2) | (1 << DDD3);
	
	// TODO: update I/O registers to match new PCB layout
	
    while (1)
    {
		
		// Update LEDs based on capacity reading
		if (capacity > 666) {
			PORTD |= (1 << PORTD6) | (1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else if (capacity > 333) {
			PORTD &= ~(1 << PORTD6);
			PORTD |= (1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else if (capacity > 10) {
			PORTD &= ~(1 << PORTD6) & ~(1 << PORTD7);
			PORTB |= (1 << PORTB0);
		} else {
			PORTD &= ~(1 << PORTD6) & ~(1 << PORTD7);
			PORTB &= ~(1 << PORTB0);
		}
		
		// If one second has passed, print battery capacity to UART
		if (usart_to_do()) {
			
			usart_transmit_diagnostics(voltage, current, temperature, capacity);
			
			// Clear USART flag
			usart_stop();
		}
		
		// If ADC sample has been taken, perform calculations
		if (dsp_to_do()) {
			
			count = ADC;
			
			if (temperature_next()) { // Proteus: 85 us
				
				// Send signal to isolate supercap if temperature outside safe range
				if ((count < 154) || (count > 870)) {
					PORTC |= (1 << PORTC5);
					} else {
					PORTC &= ~(1 << PORTC5);
					// NOTE: isolation state is held by a latch on the PCB, so potentially clearing this bit for the other samples should be OK
				}
				
				// Convert ADC sample to temperature and store result
				temperature = vin_to_tsc(adc_to_vin(count));
				
				// Change ADC channel to measure voltage
				ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
				ADMUX |= (1 << MUX1);
				
			} else if (voltage_next()) { // Proteus: 145 us if calculating SOC
				
				// Send signal to isolate supercap if temperature outside safe range
				if ((count < 154) || (count > 870)) {
					PORTC |= (1 << PORTC5);
					} else {
					PORTC &= ~(1 << PORTC5);
				}
				
				// Convert ADC sample to voltage and store result
				voltage = vin_to_vsc(adc_to_vin(count));
				
				// If MCU is starting up or no load is connected, calculate SOC from voltage
				if ((soc_startup == 0) || (no_load == 1)) {
					capacity = vsc_to_soc(voltage);
					soc_startup = 1;
				}
				
				// TODO: what happens if load is connected in between previous current scan and this voltage scan?
				// reading current then voltage within the cycle is better, but ponder further possible improvements?
				
				// Change ADC channel to measure current
				ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
				ADMUX |= (1 << MUX1) | (1 << MUX0);
				
			} else if (current_next()) { // Proteus: 150 us if calculating SOC
				
				// Send signal to isolate supercap if temperature outside safe range
				if ((count < 154) || (count > 870)) {
					PORTC |= (1 << PORTC5);
					} else {
					PORTC &= ~(1 << PORTC5);
				}
				
				// Convert ADC sample to current and store result
				current = vin_to_isc(adc_to_vin(count));
				
				if ((current < 3) && (current > -3)) {
					// If no load is connected, let SOC be calculated from upcoming voltage sample
					no_load = 1;
					} else {
					// Otherwise, calculate new SOC from current sample (Coulumb counting)
					no_load = 0;
					capacity = isc_to_soc(current, capacity);
					
					// TODO: account for extra-long pause between last sample in one ADC cycle and first sample in next ADC cycle.
				}
				
				// Change ADC channel to measure touch oscillator
				ADMUX &= ~(1 << MUX3) & ~(1 << MUX2) & ~(1 << MUX1) & ~(1 << MUX0);
				
			}
			
			// NOTE: maybe have touch as its own stage within a cycle like before, but use external interrupts instead of ADC?
			
			update_adc_status();
			dsp_stop(); // don't do calculations until next sample is received
			
		}
    }
}

ISR(TIMER1_COMPA_vect) {
	
	reset_adc_cycle();
	
	// Manually interrupt flag bit so next rising edge of trigger source can be detected
	TIFR0 = (1 << OCF0A);
	
}