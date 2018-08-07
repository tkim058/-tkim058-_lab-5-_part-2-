/*
 * [tkim058]_lab[5]_part[1].c
 * Partner(s) Name & E-mail: MinHwan Oh
 * Lab Section: B01
 * Assignment: Lab #5 Exercise #2
 * Exercise Description: [SynchSM]
 *
 * I acknowledge all content contained herein, excluding template or example
 * code, is my own original work.
 */

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned long _avr_timer_M = 1;//Start count from here, down to 0. Default 1ms
unsigned long _avr_timer_cntcurr = 0;//Current internal count of 1ms ticks
volatile unsigned char TimerFlag = 0;

void TimerOn()
{
	//avr timer/counter controller register TCCR1
	TCCR1B = 0x0B;	//bit3 = 0: CTC mode(clear timer on compare)
	//bit2bit1bit0 = 011: pre-scaler/64
	//0001011: 0x0B
	//so, 8MHz clock or 8,000,000/64 = 125,000 ticks/s
	//Thus, TCNT1 register will count at 125,000 ticks/s
	//AVR output compare register OCR1A.
	OCR1A = 125;	//Timer interrupt will be generated when TCNT1 == OCR1A
	//We want a 1ms tick. 0.001s * 125,000 ticks/s = 125
	//So when TCNT1 register equals 125,
	//1 ms has passed. Thus, we compare to 125.
	//AVR timer interrupt mask register
	TIMSK1 = 0x02;	// bit1: OCIE1A -- enables compare match interrupt
	
	//Initialize avr counter
	TCNT1 = 0;
	
	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr milliseconds
	
	//Enable global interrupts
	SREG |= 0x80;	//0x80: 1000000
}

void TimerOff()
{
	TCCR1B = 0x00;	// bit3bit1bit0 = 000: timer off
}

void TimerISR()
{
	TimerFlag = 1;
}

//In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	//CPU automatically calls when TCNT1 == OCR1 (every 1ms per TimerOn settings)
	_avr_timer_cntcurr--;	//count down to 0 rather than up to TOP
	if(_avr_timer_cntcurr == 0)
	{
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

//Set TimerISR() to tick every M ms
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

enum States {Init, next_led, prev_led, stay_pr, released} state;

unsigned char tmpB = 0x00;
unsigned char button = 0x00;
unsigned char cnt = 0x00;

void Tick()
{
	switch(state)
	{//state transition
		case Init:
		if(button)
		{
			state = Init;
		}
		else
		{
			state = next_led;
			cnt = 0x00;
			tmpB = 0x01;
		}
		break;
		
		case next_led:
		if((tmpB == 0x04) && (cnt == 0x0A))
		{
			state = prev_led;
			tmpB = tmpB >> 1;
			cnt = 0;
		}
		else if(button)
		{
			state = stay_pr;
		}
		else
		{
			state = next_led;
		}
		break;
		
		case prev_led:
		if((tmpB == 0x01) && (cnt == 0x0A))
		{
			state = next_led;
			tmpB = tmpB << 1;
			cnt = 0;
		}
		else if(button)
		{
			state = stay_pr;
		}
		else
		{
			state = prev_led;
		}
		break;
		
		case stay_pr:
		if(!button)
		{
			state = released;
		}
		else if(button)
		{
			state = stay_pr;
		}
		else
		{
			break;
		}
		break;
		
		case released:
		if(button)
		{
			state = Init;
		}
		else if(!button)
		{
			state = released;
		}
		else
		{
			break;
		}
		break;
		
		default:
		break;
	}
	
	switch(state)
	{//state action
		case Init:
		break;
		
		case next_led:
		if(cnt == 0x0A)
		{
			tmpB = tmpB << 1;
			cnt = 0;
		}
		else
		{
			cnt++;
		}
		break;
		
		case prev_led:
		if(cnt == 0x0A)
		{
			tmpB = tmpB >> 1;
			cnt = 0;
		}
		else
		{
			cnt++;
		}
		break;
		
		case stay_pr:
		break;
		
		case released:
		break;
		
		default:
		break;
	}
}


void main()
{
	
	DDRA = 0x00;	DDRB = 0xFF;
	PORTA = 0xFF;	PORTB = 0x00;
	
	TimerSet(30);
	TimerOn();
	
	state = Init;

	while (1)
	{
		button = ~PINA;
		Tick();
		while(!TimerFlag){}
		TimerFlag = 0;
		PORTB = tmpB;
	}
}

