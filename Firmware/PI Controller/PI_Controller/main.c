/*
 * PI_Controller.c
 *
 * Created: 29/09/2026 10:40:25 am
 * Author : wwei275
 */ 

#include <avr/io.h>
#include <stdint.h>
#include "pi_control.h"
#include "adc.h"
#include "timer.h"

#define V_REF 512 

volatile static uint16_t ADC_VALUE = 0;

int main(void)
{
    tc0_init_FPWM_100k();   //Initializing TC0 to generate a 100kHz 50% D PWM
    adc_init();             //Initializing ADC with auto trigger mode
    DDRB |= (1 << PB5);       // PB5 as output
    /* Replace with your application code */
    while (1) 
    {
        ADC_VALUE = adc_read_channel_pool();
        PINB = (1 << PB5);
        OCR0B = pi_mapped(ADC_VALUE, V_REF);
    }
}