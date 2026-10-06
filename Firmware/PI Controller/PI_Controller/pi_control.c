#include <avr/io.h>
#include <stdint.h>

#define VOLTAGE_KI 300
#define VOLTAGE_KP 1
#define T_SAMPLE (1.0f/10000.0f)
#define PI_LIMIT 48
#define OCR 159

static float Int_out;
static float PI_out;

float sat_limit_controller (float result){
    if (result > PI_LIMIT){
        return PI_LIMIT;
    }
    else if (result < -PI_LIMIT){
        return -PI_LIMIT;
    }
    else{
        return result;
    }
}

uint8_t pi_mapped (uint16_t V_out, uint16_t V_ref){
	int32_t V_err = -((int32_t)V_ref - V_out); //Calculating error voltage
	int32_t Prop_out = V_err * VOLTAGE_KP; //Proportional error term
	Int_out = sat_limit_controller(Int_out + (float)V_err * VOLTAGE_KI * T_SAMPLE); //Calculating integral term and limiting to PI_LIMIT
	PI_out = sat_limit_controller(Prop_out + Int_out); //Calculating PI_out and limiting by PI_LIMIT
	uint8_t PI_out_mapped = (uint8_t) ((PI_out * (0.6f*(float)OCR/(2.0f * PI_LIMIT))) + ((float)OCR/2.0f) + 0.5f);
	return PI_out_mapped;
}