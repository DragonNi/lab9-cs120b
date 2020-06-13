/*	Author: 
 *  Partner(s) Name: 
 *	Lab Section: 22
 *	Assignment: Lab #9  Exercise #3
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


unsigned char counter = 0;
unsigned char waitCounter = 1;
double notes[21] = {329.62, 329.63, 329.63, 329.63, 293.66,329.63, 440.00, 329.66, 293.66, 293.66, 293.66, 293.66, 293.66, 293.66, 261.63, 293.66, 392.00, 293.66, 261.63, 246.94, 261.63};
unsigned char noteLength[21] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,5};
unsigned char waitTimes[21] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
unsigned char on = 0;
enum States{Start, off, play, wait, hold} state;
void Tick(){
	unsigned char temp_A = ~PINA;
	switch(state){
		case Start:
			PWM_on();
			state = off;
			break;
		case off:
			if((temp_A & 0x01) == 0x01){
				state = play;
			}
			else{
				state = off;
			}
			break;
		case play:
			if(waitCounter < noteLength[counter]){
				state = play;
			}
			else{
				waitCounter = 1;
				state = wait;
			}
			break;
		case wait:
			if(waitCounter < noteLength[counter]){
				state = wait;
			}
			else if(counter !=20){
				waitCounter= 1;
				counter++;
				state = play;
			}
			else if((temp_A & 0x01) == 0x01 && counter == 20){
				counter = 0;
				state = hold;
			}
			else{
				counter = 0;
				state = off;
			}
			 
			break;
		case hold:
			if((temp_A & 0x01) == 0x01){
				state = hold;
			}
			else{
				state = off;
			}
			break;

		default:
			state = Start;
			break;
	}
	switch(state){
		case Start:
			break;
		case off:
			break;
		case play:
			set_PWM(notes[counter]);
			waitCounter++;
			break;
		case wait:
			set_PWM(0);
			waitCounter++;
			break;
		case hold:
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

	TimerSet(200);
	TimerOn();	
	PWM_on();
    while (1) {

	Tick();
	while(!TimerFlag);

	TimerFlag = 0;
    }
    return 1;
}
