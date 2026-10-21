/*
 * PI_Controller.c
 *
 * Created: 29/09/2026 10:40:25 am
 * Author : wwei275
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>
#include "pi_control.h"
#include "adc.h"
#include "timer.h"
#include "uart.h"

#define V_REF 512 

volatile uint16_t ADC_VALUE = 0;
volatile uint8_t adc_ready = 0;
volatile uint8_t tx_ready = 1;
volatile uint8_t tx_index = 0;
uint8_t digits[4];
uint8_t tx_length = 1;
char tx_buffer[24];

int main(void)
{
    tc0_init_FPWM_100k();   //Initializing TC0 to generate a 100kHz 50% D PWM
    adc_init();             //Initializing ADC with auto trigger mode
	UART_init_interrupt(); //Initializing UART
    DDRB |= (1 << PB5);       // PB5 as output
	sei();
    /* Replace with your application code */
	
    while (1) 
    {
		if (adc_ready == 1){
			cli(); //Disable interrupts to prevent race condition
			uint16_t ADC_buffer = ADC_VALUE;
			adc_ready = 0; //Ready to sample again
			sei();
			OCR0B = pi_mapped(ADC_buffer, V_REF); //Map error as a PWM
			if (tx_ready == 1){
				voltage_to_digits(ADC_buffer); //convert ADC value to mv
				sprintf(tx_buffer, "Voltage = %d.%d%d%d V\r\n", digits[0], digits[1], digits[2], digits[3]);
				UART_start((uint8_t)strlen(tx_buffer)); //start UART
			}
		}
    }
}