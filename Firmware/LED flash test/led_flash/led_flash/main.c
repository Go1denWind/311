/*
 * led_flash.c
 *
 * Created: 7/08/2026 8:26:52 AM
 * Author : ctra157
 */ 

#include <avr/io.h>


int main(void)
{
	
	DDRB = 0x01; // PB0 output
	DDRC = 0xFE; // PC0 input
	PORTC = 0x01; // activate pull-up resistor
	
    /* Replace with your application code */
    while (1) 
    {
		if (~PINC & (1 << PINC0)) {
			// set output pin
			PORTB |= (1 << PORTB0);
		} else {
			// clear output pin
			PORTB &= ~(1 << PORTB0);
		}
    }
}

