#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

extern volatile uint16_t ADC_VALUE;
extern volatile uint8_t adc_ready;

ISR(ADC_vect){
	PINB |= (1<<PINB5);         //Toggling PB5 to indicate ISR trigger
	ADC_VALUE = ADC;            //Storing ADCL and ADCH values in a global variable
	adc_ready = 1;   //Updating state to indicate new data available
	TIFR0 |= 1 << TOV0;         //Clear the TC0 overflow flag to prepare for next conversion
}


//This function configures the ADC to use AVCC as reference and read ADC0 in auto trigger mode
void adc_init(void){
	DDRC &= ~(1 << PC0); //PC0 as input
	PORTC &= ~(1 << PC0); //pull-up disabled
	ADMUX |= 1 << REFS0;  //AVCC set as reference, ADC0 selected and results are right adjusted
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE) | (0b110 << ADPS0) | (1<<ADIE);
	//Enable ADC, start conversion, setup auto-trigger and set prescaler to 64
	ADCSRB |= (0b100 << ADTS0);   //Use TC0 overflow as auto- trigger source
	DIDR0 = 1 << ADC0D;   //ADC0 buffer disabled
}