#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include <stdio.h>
#include <avr/interrupt.h>
#include <inttypes.h>


#define MAXX 240
#define MAXY 320

#define BUFF_MAX 20
char buff[BUFF_MAX];
volatile uint8_t buff_ptr_h = 0;
volatile uint8_t buff_ptr_t = 0;

void init_uart(void)
{
	/* Configure 9600 baud, 8-bit, no parity and one stop bit */
	const int baud_rate = 9600;
	UBRR0H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR0L = (F_CPU/(baud_rate*16L)-1);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
}


ISR(USART0_RX_vect) 
{
	
	buff[buff_ptr_h] = UDR0;	
	buff_ptr_h++;
	if (buff_ptr_h == BUFF_MAX)
		buff_ptr_h = 0;

}

int main(void)
{
	
	init_lcd();
	init_uart();
	
	sei();
	
	DDRB = _BV(7);
	
	
	display_string("hello"); 
	char c;

	while(1)
	{

	
		while(buff_ptr_h == buff_ptr_t);

		cli();		
		c = buff[buff_ptr_t];
		buff_ptr_t++;
		if (buff_ptr_t == BUFF_MAX)
			buff_ptr_t = 0;
		sei();

		if (c == '\n')
		{
		
			rectangle b;
			b.top = display.y+8;
			b.bottom = display.y+24;
			b.left = 0;
			b.right = LCDWIDTH;
			fill_rectangle(b, BLACK);
			
			display.x = 0;
			display.y += 8;
			
		}
		else
		{
			if (display.x < LCDWIDTH-10)
				display_char(c);
			
		}
		
		if (display.y >= LCDHEIGHT)
			display.y=0;
	
	
	}

	return 0;
}