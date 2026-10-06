/*
 * adc.h
 *
 * Created: 7/10/2026 9:29:22 am
 *  Author: wwei275
 */ 


#ifndef ADC_H_
#define ADC_H_

#include <avr/io.h>
#include <stdint.h>

void adc_init(void);
uint16_t adc_read_channel_pool(void);

#endif /* ADC_H_ */