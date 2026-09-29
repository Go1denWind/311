/*
 * PI_Controller.c
 *
 * Created: 29/09/2026 10:40:25 am
 * Author : wwei275
 */ 

#include <avr/io.h>

#define VOLTAGE_KI 10
#define VOLTAGE_KP 100
#define T_SAMPLE 1/20000
#define PI_LIMIT 1590000

static float Int_out;
static float PI_out;

float sat_limit_controller	(float result){
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

void pi_controller (uint16_t V_out, uint16_t V_ref){
	int32_t V_err = -((int32_t)V-ref - V_out);
	int32_t Prop_out = Verr * VOLTAGE_KP;
	Int_out = sat_limit_controller(Int_out + (float)V_err * VOLTAGE_KI * T_SAMPLE);
	PI_out = sat_limit_controller(Prop_out + Int_out);
}

int main(void)
{
    /* Replace with your application code */
    while (1) 
    {
		pi_controller(V_out, V_ref);
    }
}

