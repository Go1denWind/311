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

float adc_to_vin(uint16_t count);

float get_sc_charge();

#endif /* ADC_H_ */