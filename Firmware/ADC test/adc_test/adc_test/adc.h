/*
 * adc.h
 *
 * Created: 1/09/2025 9:38:29 pm
 *  Author: whoyo
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <stdint.h>

void adc_init();
uint16_t adc_read(uint8_t channel);
uint16_t adc_convert_mv(uint16_t value);

#endif /* ADC_H_ */