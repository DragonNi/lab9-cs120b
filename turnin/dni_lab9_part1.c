/*	Author: 
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #9  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void set_PWM(double frequency){
	static double current_frequency;

	if(frequency != current_frequency){
		if(!frequency){
			TCCR3B &= 0x08;
		}
		else{
			TCCR3B |= 0x03;
		}

		if(frequency < 0.954){
			OCR3A = 0xFFFF;
		}
		else if(frequency > 31250){
			OCR3A = 0x0000;
		}
		else{
			OCR3A = (short)(8000000/(128 * frequency)) - 1;
		}

		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on(){
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}


enum States{start, off, b1, b2, b3} state;
void Tick(){
	unsigned char tempA = ~PINA;
	//state transistions
	switch(state){
		case start:
			state = off;
			break;

		case off:
			if((tempA & 0x07) == 0x01){
				state = b1;
			}
			else if((tempA & 0x07) == 0x02){
				state = b2;
			}
			else if((tempA & 0x07) == 0x04){
				state = b3;
			}
			else{
				state = off;
			}

			break;

		case b1:
			if((tempA & 0x07) == 0x01){
				state = b1;
			}
			else{
				state = off;
			}
			break;

		case b2:
			if((tempA & 0x07) == 0x02){
				state = b2;
			}
			else{
				state = off;
			}
			break;

		case b3:
			if((tempA & 0x07) == 0x04){
				state = b3;
			}
			else{
				state = off;
			}
			break;

		default:
			state = start;
			break;
	}

	switch(state){
		case start:
			break;
		case off:
			set_PWM(0);
			break;
		case b1:
			set_PWM(261.63);
			break;
		case b2:
			set_PWM(293.61);
			break;
		case b3:
			set_PWM(329.63);
			break;
			
		default:
			break;
	}

}

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr == 0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}




int main(void) {
    /* Insert DDR and PORT initializations */
    /* Insert your solution below */
	DDRA = 0x00;	PORTA = 0xFF;
	DDRB = 0xFF;	PORTB = 0x00;
	
	TimerSet(300);
	TimerOn();	
	PWM_on();
    while (1) {
	Tick();
    }
    return 1;
}
