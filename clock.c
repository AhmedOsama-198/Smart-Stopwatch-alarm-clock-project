#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define RESET_PB			PD2
#define PAUSE_PB			PD3
#define RESUME_PB			PB2
#define MODE_TOGGLE_PB		PB7
#define HOURS_INC_PB		PB1
#define HOURS_DEC_PB		PB0
#define MIN_INC_PB			PB4
#define MIN_DEC_PB			PB3
#define SEC_INC_PB			PB6
#define SEC_DEC_PB			PB5
#define RED_LED				PD4
#define YELLOW_LED			PD5
#define BUZZER				PD0

#define NUM_OF_7_SEG		6

#define NUMBER_OF_COMPARE_MATCHE_PER_SECOUND 16

short num[NUM_OF_7_SEG] = {0}, flag = 0;

unsigned char g_tick=0;

enum mode{
	COUNTDOWN, COUNTUP
}timerMode = COUNTUP;

void Timer1_Comp_STOPWATCH(void);
void push_button_INT0(void);
void push_button_INT1(void);
void push_button_INT2(void);
void display_numbers(void);
void switchmode(void);
void set_countdown_time(void);
void handlecountup(void);
void handlecountdown(void);



int main (void)
{
	DDRB &= ~ 0B01111011;
	PORTB |= 0B01111011;
	DDRA |= 0b00111111;
	DDRC |= 0x0F;
	DDRD |= (1 << RED_LED) | (1 << YELLOW_LED) |(1 << BUZZER);
	PORTD |= (1 << RED_LED);
	PORTD &=~(1<<BUZZER);
	SREG |= (1 << 7);

	Timer1_Comp_STOPWATCH();
	push_button_INT0();
	push_button_INT1();
	push_button_INT2();

	while (1)
	{
		if(PINB & (1<<MODE_TOGGLE_PB)){
			while(PINB & (1<<MODE_TOGGLE_PB));

			switchmode();
		}
		if (flag == 1)
		{
			switch(timerMode){
			case COUNTUP:

				handlecountup();

				break;

			case COUNTDOWN:

				handlecountdown();

				break;
			}

			flag = 0;

		}

		display_numbers();
	}
}

void display_numbers(void){
	for (unsigned char i = 0; i < NUM_OF_7_SEG; i++)
	{
		PORTC = (PORTC & 0xF0) | (num[i] & 0x0F);
		PORTA |= (1 << i);
		_delay_ms(2);
		PORTA &= ~(1 << i);
	}
}

void Timer1_Comp_STOPWATCH (void)
{
	TCNT1 = 0;
	TCCR1A = (1 << FOC1A);
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);
	OCR1A = 970;//top value
	TIMSK |= (1 << OCIE1A);
}

void push_button_INT0 (void)
{
	DDRD &= ~(1 << RESET_PB);
	PORTD |= (1 << RESET_PB);

	MCUCR |= (1 << ISC01);
	GICR |= (1 << INT0);
}

void push_button_INT1 (void)
{
	DDRD &= ~(1<<PAUSE_PB);

	MCUCR |= (1 << ISC11) | (1 << ISC10);
	GICR |= (1 << INT1);
}

void push_button_INT2 (void)
{
	DDRB &= ~(1 << RESUME_PB);
	PORTB |= (1 << RESUME_PB);

	MCUCSR &= ~(1 << ISC2);
	GICR |= (1 << INT2);
}



void display(void){
	for (unsigned char i = 0; i < NUM_OF_7_SEG; i++)
	{
		PORTC = (PORTC & 0xF0) | (num[i] & 0x0F);
		PORTA |= (1 << i);
		_delay_ms(2);
		PORTA &= ~(1 << i);
	}
}

void switchmode(void){
	switch(timerMode){
	case COUNTDOWN:
		timerMode = COUNTUP;
		PORTD &=~(1<< YELLOW_LED);
		PORTD |= (1 << RED_LED);
		for (unsigned char i = 0; i < NUM_OF_7_SEG; i++)
		{
			num[i] = 0;//clearing all 7-segment displays
		}
		TCNT1=0;
		g_tick = 0;
		PORTD &=~ (1<< BUZZER);

		break;
	case COUNTUP:
		timerMode = COUNTDOWN;
		PORTD &=~(1<< RED_LED);
		PORTD |= (1 << YELLOW_LED);
		set_countdown_time();
		PORTD &=~ (1<< BUZZER);

		break;
	}
}

void set_countdown_time(void)
{
	for (unsigned char i = 0; i < NUM_OF_7_SEG; i++)
	{
		num[i] = 0;
	}
	TCCR1B = 0;
	TCNT1=0;
	g_tick = 0;

	while((PINB&(1<<RESUME_PB))){
		display();

		if(!(PINB & (1<<HOURS_INC_PB))){
			num[4]++;
			if(num[4] == 10){
				if(num[5] == 9){
					num[4] = 9;
				}
				else{
					num[5]++;
					num[4] = 0;
				}
			}
			while(!(PINB & (1<<HOURS_INC_PB))){
				display_numbers();
			}
		}
		if(!(PINB & (1<<HOURS_DEC_PB))){
			num[4]--;
			if(num[4] == -1){
				if(num[5] == 0){
					num[4] = 0;
				}
				else{
					num[5]--;
					num[4] = 9;
				}
			}
			while(!(PINB & (1<<HOURS_DEC_PB))){
				display_numbers();
			}
		}

		if(!(PINB & (1<<MIN_INC_PB))){
			num[2]++;
			if(num[2] == 10){
				if(num[3] == 5){
					num[2] = 9;
				}
				else{
					num[3]++;
					num[2] = 0;
				}
			}
			while(!(PINB & (1<<MIN_INC_PB))){
				display_numbers();
			}
		}
		if(!(PINB & (1<<MIN_DEC_PB))){
			num[2]--;
			if(num[2] == -1){
				if(num[3] == 0){
					num[2] = 0;
				}
				else{
					num[3]--;
					num[2] = 9;
				}
			}
			while(!(PINB & (1<<MIN_DEC_PB))){
				display_numbers();
			}
		}
		if(!(PINB & (1<<SEC_INC_PB))){
			num[0]++;
			if(num[0] == 10){
				if(num[1] == 5){
					num[0] = 9;
				}
				else{
					num[1]++;
					num[0] = 0;
				}
			}
			while(!(PINB & (1<<SEC_INC_PB))){
				display_numbers();
			}
		}
		if(!(PINB & (1<<SEC_DEC_PB))){
			num[0]--;
			if(num[0] == -1){
				if(num[1] == 0){
					num[0] = 0;
				}
				else{
					num[1]--;
					num[0] = 9;
				}
			}
			while(!(PINB & (1<<SEC_DEC_PB))){
				display_numbers();
			}
		}
	}

}

void handlecountup(void)
{

	num[0]++;

	if (num[0] == 10)
	{
		num[0] = 0;
		num[1]++;

	}

	if (num[1] == 6)
	{
		num[1] = 0;
		num[2]++;
	}

	if (num[2] == 10)
	{
		num[2] = 0;
		num[3]++;
	}

	if (num[3] == 6)
	{
		num[3] = 0;
		num[4]++;
	}

	if (num[4] == 10)
	{
		num[4] = 0;
		num[5]++;
	}

	if (num[5] == 10)
	{
		num[5] = 0;
	}
}
void handlecountdown(void)
{
	if(num[0]==0)
	{
		if(num[1]== 0)
		{
			if(num[2]==0)
			{
				if(num[3]==0)
				{
					if(num[4]==0)
					{
						if(num[5]==0)
						{

							PORTD |= (1<< BUZZER);
						}
						else
						{
							num[5]--;
							num[4] = 9;
							num[3] = 5;
							num[2] = 9;
							num[1] = 5;
							num[0] = 9;
						}
					}
					else
					{
						num[4]--;
						num[3] = 5;
						num[2] = 9;
						num[1] = 5;
						num[0] = 9;
					}
				}
				else
				{
					num[3]--;
					num[2] = 9;
					num[1] = 5;
					num[0] = 9;
				}
			}
			else
			{
				num[2]--;
				num[1] = 5;
				num[0] = 9;
			}
		}
		else
		{
			num[1]--;
			num[0] = 9;
		}
	}
	else
	{
		num[0]--;
	}
}


ISR (TIMER1_COMPA_vect)
{
	g_tick++;
	if (g_tick == NUMBER_OF_COMPARE_MATCHE_PER_SECOUND)
	{
		flag = 1;
		g_tick = 0;
	}
}
ISR (INT0_vect)
{
	for (unsigned char i = 0; i < NUM_OF_7_SEG; i++)
	{
		num[i] = 0;
	}
	TCNT1=0;
	g_tick = 0;

	PORTD &=~ (1<< BUZZER);

}

ISR (INT1_vect)
{
	TCCR1B=0;
	PORTD &=~ (1<< BUZZER);
}

ISR (INT2_vect)
{
	TCCR1B = (1 << WGM12) | (1 << CS12) | (1 << CS10);

}
