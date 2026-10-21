#include <avr/io.h>
#include <stdint.h>

//This function configures TC0 to operate in fast PWM mode with a 100kHz frequency and a 50% D
void tc0_init_FPWM_100k(void){
	TCCR0A = 2<<COM0B0 | 3<<WGM00;    //WGM0[2..0] set to 111 for Fast PWM mode & COM0B[1..0] for clear on match with OCR0B
	TCCR0B = 1<<WGM02 | 1<<CS00;      //WGM[2..0] to 111 and CS0[2..0] set to 001 for a clock divider of 1
	OCR0A = 159;                      //Loading OCR0A with 159 to setup a 100kHz PWM
	OCR0B = 79;                       //Loading OCR0B with 79 to setup 50% D
	DDRD |= 1<<PIND5;                 //PD5 setup as an output to generate PWM
}