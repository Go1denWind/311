/*
 * Lab4_Microchip2.c
 *
 * Created: 1/09/2025 9:31:46 pm
 * Author : whoyo
 */ 

#include <avr/io.h>
#include <util/delay.h>
#include "adc.h"
#include "uart.h"

#define F_CPU 2000000UL

int main(void)
{
	usart_init(12);
    adc_init();
	
	uint16_t ch_zero_samples[40];
	uint16_t ch_one_samples[40];
	
	for (uint8_t i = 0; i < 40; i++) {
		
		// Gather 40 readings from ADC0 and ADC1 each
		ch_zero_samples[i] = adc_convert_mv(adc_read(0));
		ch_one_samples[i] = adc_convert_mv(adc_read(1));
		
	}
	
	for (uint8_t i = 0; i < 40; i++) {
		
		// Transmit readings over UART
		usart_transmit_mv(ch_zero_samples[i]);
		usart_transmit_array(", ");
		usart_transmit_mv(ch_one_samples[i]);
		
		usart_transmit_array("\r\n");
		
	}
	
    while (1) 
    {
		
			
    }
}