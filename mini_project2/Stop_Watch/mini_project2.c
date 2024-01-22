/*
 * mini_project2.c
 *
 *  Created on: Sep 13, 2023
 *      Author: Mahmoud Magdy Mahmoud Ali
 */
#include <avr/io.h>
#include <util/delay.h>
#include<avr/interrupt.h>

unsigned char pin[6]; // Six 7-Seg

unsigned char flag =0;

void INT0_Func(void) // interrupt 0 Function
{
	SREG |= (1<<7); // Enable I-Bit
	DDRD &= ~(1<<PD2); // Set PD2 as Input Pin For Push Button
	PORTD |= (1<<PD2); // Activate The Internal Pull-Up Resistor
	GICR |= (1<<INT0); // Enable The External Interrupt INT0
	MCUCR |= (1<<ISC01); // The falling edge of INT0 generates an interrupt request.
}

ISR(INT0_vect) // ISR For Interrupt 0
{
	int i;
	for(i=0;i<5;i++)
	{
		pin[i] = 0; // Reset All 7-Seg to 0
	}
	TCNT1=0; // Put The Initial Value For The Timer-1 Equal 0
}



void INT1_Func(void)  // Interrupt 1 Function
{
	SREG |= (1<<7); // Enable I-Bit
	DDRD &= ~(1<<PD3); // Set PD3 as Input Pin For Push Button
	MCUCR |= (1<<ISC10) | (1<<ISC11); // The Rising edge of INT1 generates an interrupt request.
	GICR |= (1<<INT1); // Enable The External Interrupt INT1
}

ISR(INT1_vect) // ISR For Interrupt 1
{
	// No clock source (Timer/Counter stopped).
	TCCR1B &=~(1<<CS10);
	TCCR1B &=~(1<<CS11);
	TCCR1B &=~(1<<CS12);
}



void INT2_Func(void)  // Interrupt 2 Function
{
	DDRB &= ~(1<<PB2);  // Set PB2 as Input Pin For Push Button
	PORTB |= (1<<PB2);  // Activate The Internal Pull-Up Resistor
	MCUCR &= ~(1<<ISC2); // The falling edge of INT2 generates an interrupt request.
	GICR |= (1<<INT2);  // Enable The External Interrupt INT2
}

ISR(INT2_vect) // ISR For Interrupt 2
{
	// Set clkI/O/1024 (From prescaler) to Resume the Stop Watch Time
	TCCR1B |= (1<<CS12) | (1<<CS10);
}




void Timer1_Func(void)
{
	TCCR1A |= (1<<FOC1A); // FOC1A bits are only active with a non-PWM mode.
	TCCR1B |= (1<<CS12) | (1<<CS10);  // clkI/O/1024 (From prescaler)
	TCCR1B |= (1<<WGM12);  // Enable CTC Mode
	TCNT1 = 0;  // Put Initial Value 0 For Timer-1
	OCR1A = 1000; // Put The Compare Value Equal 1000 Micro Seconds
	TIMSK |= (1<<OCIE1A);  // Output Compare A Match Interrupt Enable
}

ISR(TIMER1_COMPA_vect) // ISR For TIMER1
{
	flag = 1;  // Make The Flag equal 1
}




void Display_Digits_Func (void) // Function to Display All 7-SEG
{
	/* Loop on all The Output PINS In PORT A and ALL 7-SEG which Connected to Decoder at the same
	                to keep displaying all 7-SEG*/
	int j;
	for(j=0;j<6;j++)
	{
		PORTA = (1<<j);  // Set Pin j in PORT A as 1
		PORTC = pin[j];  // Set Pin j in PORT C as 1
		_delay_ms(2);  // Make a Delay 2 micro to show all the 6 7-SEG at the same Time
	}
}





void Calculate_Time_Func (void) // Function To Update The Time Every 1 Second
{
	/* Seconds-Controller */
	pin[0]++;  // First 7-SEG Will Start Count from 0 and will be incremented Every 1 Seconds
	if((pin[0]) == 10)  // Check if the first 7-SEG Reaches up  10
	{
		pin[1]++;  // Increament the Second 7-SEG every 10 Seconds
		pin[0] = 0; // Set First 7-SEG to 0 Every 10 Seconds
		if((pin[1]) == 6)  // Check if the second 7-SEG Reaches up 6
		{
			pin[2]++;  // Increament The Third 7-SEG every 60 Seconds
			pin[1] = 0;  // Set Second 7-SEG to 0 Every 60 Seconds
		}
	}

	/* Minute-Controller */
	if((pin[2] == 10))  // Check If the Third 7-SEG Reaches Up 10
	{
		pin[3]++;  // Increament The 4 7-SEG Every 10 Minutes
		pin[2] = 0;  // Set Third 7-SEG To 0 Every 10 Minutes
		if((pin[3]) == 6) // Check If Fourth 7-SEG Reaches up 6
		{
			pin[4]++; // Increament The Fifth 7-SEG Every 60 Minutes
			pin[3] = 0; // Set Fourth 7-SEG to 0 Every 60 Minutes
		}
	}

	/* Hours-Controller */
	if((pin[4]) == 10) // Check If Fifth 7-SEG Reaches up 10
	{
		pin[5]++;  // Increment The Sixth 7-SEG every 10 Hours
		pin[4] = 0; // Set The Sixth 7-SEG to 0 every 10 Hours
		if((pin[5]) == 6) // Check if The Sixth 7-SEG Reaches up 6
		{
			// Reset The Stop Watch Time
			pin[0]=0;
			pin[1]=0;
			pin[2]=0;
			pin[3]=0;
			pin[4]=0;
			pin[5]=0;
		}
	}
}
int main()
{
	DDRA = 0x3F; // 	Set First 6-Pins in PORTA as Output to connect to Six 7Seg
	PORTA = 0x01; // Make PA0 as the Initial Pin

	DDRC = 0x0F; // Set First 4-Pin in PORTC as Output to connect to 7447 Decoder and then to the Multiplexed 6 7-SEG
	PORTC = 0x00; // Make Initial Value = 0

	SREG |= (1<<7); // Enable I-Bit

	/* Call all Functions */

	INT0_Func();
	INT1_Func();
	INT2_Func();
	Timer1_Func();

	while(1)
	{
		Display_Digits_Func(); // Display All 7-SEG At The Same Time Always
		if(flag == 1)  // Check If Timer1_Interrupt Occurs
		{
			Calculate_Time_Func();  // Update The 7-SEG
			flag = 0;   // Reset The Flag
		}
	}
}


