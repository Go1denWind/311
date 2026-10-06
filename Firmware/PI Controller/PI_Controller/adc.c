#include <avr/io.h>
#include <stdint.h>

//This function configures the ADC to use AVCC as reference and read ADC0 in auto trigger mode
void adc_init(void){
	DDRC &= ~(1 << PC0); //PC0 as input
	PORTC &= ~(1 << PC0); //pull-up disabled
	ADMUX |= 1 << REFS0;  //AVCC set as reference, ADC0 selected and results are right adjusted
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (0b110 << ADPS0);
	//Enable ADC, start conversion, setup auto-trigger and set prescaler to 64
	ADCSRB |= (0b100 << ADTS0);   //Use TC0 overflow as auto- trigger source
	DIDR0 = 1 << ADC0D;   //ADC0 buffer disabled
}

//This function reads an ADC channel and return results
uint16_t adc_read_channel_pool(void){
	while ((ADCSRA & (1 << ADIF)) == 0) { //ADIF bit is checked to see if it is 0
		;                                   //If ADIF bit is not 1, wait until it becomes 1
	}
	ADCSRA |= 1 << ADIF;                  //Clear the ADIF flag
	TIFR0 |= 1 << TOV0;                     //Clear the TC0 overflow flag
	return ADC;                           //Returning the ADC value
}