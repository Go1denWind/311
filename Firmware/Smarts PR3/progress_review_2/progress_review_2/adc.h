/*
 * adc.h
 *
 * Created: 18-Aug-26 9:54:06 AM
 *  Author: Caleb Tran
 */ 

#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init();

uint8_t voltage_next();
uint8_t current_next();
uint8_t temperature_next();

uint16_t adc_to_vin(uint16_t count);

uint16_t vin_to_vsc(uint16_t vin);
uint16_t vin_to_isc(uint16_t vin);
uint16_t vin_to_tsc(uint16_t vin);

float get_capacity();
void reset_adc_cycle();

uint16_t get_number();

#endif /* ADC_H_ */