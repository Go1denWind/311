/*
 * uart.c
 *
 * Created: 21/10/2026 12:19:18 pm
 *  Author: wwei275
 */ 

#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define F_CPU 16000000UL
#define BAUD_RATE 9600UL

extern uint8_t tx_length;
extern uint8_t digits[4];
extern volatile uint8_t tx_ready;
extern volatile uint8_t tx_index;
extern char tx_buffer[24];

void UART_init_interrupt(void){
	UCSR0B |= 1 << TXEN0 | 1 << TXCIE0;
	//Enable transmit, receive, and interrupt, UCSZ02=0 (default - 8 data bits))
	UCSR0C |= 3 << UCSZ00;      //UMSEL0[1..0]=00 (default - UART mode), UPM0[1..0]=00 (default - no parity),
	//USBS0=0 (default - 1 stop bit), UCSZ0[1..0]=11 (default - 8 data bits)
	UBRR0 = F_CPU / ((uint32_t)16 * BAUD_RATE) - 1;
	//Set UBRR0 as per baud rate formula
}

void voltage_to_digits(int16_t adc){
	int16_t voltage_mv = 10000UL*adc/1024UL; //convert ADC back to mV
	//Separate digits
	digits[0] = voltage_mv/1000;
	digits[1] = (voltage_mv/100)%10;
	digits[2] = (voltage_mv/10)%10;
	digits[3] = voltage_mv%10;
}

void UART_start(uint8_t length){
	tx_length = length;
	tx_index = 1; //index is 1 because index 0 is started in this function
	tx_ready = 0;
	UDR0 = tx_buffer[0];
}

ISR(USART_TX_vect){
	if (tx_length > tx_index){
		UDR0 = tx_buffer[tx_index++]; //send string byte by byte
	}
	else{
		tx_ready = 1; //whole message sent
	}
}