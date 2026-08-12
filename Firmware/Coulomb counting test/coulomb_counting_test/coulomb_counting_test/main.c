/*
 * coulomb_counting_test.c
 *
 * Created: 13-Aug-26 9:26:29 AM
 * Author : Caleb Tran
 */ 

#include <avr/io.h>
#include <stdint.h>

#define T_SAMPLE 0.001

int main(void)
{
	
	// Fake array of samples to loop through (in mA, supercap discharging)
	double fake_samples[4] = {1, 1, 1, 1};
		uint8_t i = 0;
	
	// Assume initial full capacity, 3.8 V
	double capacity = 64800; // mAs
	
    /* Replace with your application code */
    while (1)
    {
		
    }
}

