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

void update_adc_status();

uint8_t temperature_next();
uint8_t voltage_next();
uint8_t current_next();

uint16_t adc_to_vin(uint16_t count);

uint16_t vin_to_vsc(uint16_t vin);
int16_t vin_to_isc(uint16_t vin);
int8_t vin_to_tsc(uint16_t vin);

float vsc_to_soc(uint16_t vsc);
float isc_to_soc(int16_t isc, float capacity);

void reset_adc_cycle();

uint8_t usart_to_do();
void usart_stop();

uint8_t dsp_to_do();
void dsp_stop();

#endif /* ADC_H_ */