/*
 * pi_control.h
 *
 * Created: 7/10/2026 9:28:20 am
 *  Author: wwei275
 */ 


#ifndef PI_CONTROL_H_
#define PI_CONTROL_H_

#include <avr/io.h>
#include <stdint.h>

float sat_limit_controller (float result);
uint8_t pi_mapped (uint16_t V_out, uint16_t V_ref);

#endif /* PI_CONTROL_H_ */