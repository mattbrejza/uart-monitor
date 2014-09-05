#include <avr/io.h>
#include <util/delay.h>
#include "lcd.h"
#include <stdio.h>
#include <avr/interrupt.h>
#include <inttypes.h>


#define MAXX 240
#define MAXY 320

#define BUFF_MAX 40
char buff[BUFF_MAX];
volatile uint8_t buff_ptr_h = 0;
volatile uint8_t buff_ptr_t = 0;

char buff2[BUFF_MAX];
volatile uint8_t buff_ptr_h2 = 0;
volatile uint8_t buff_ptr_t2 = 0;

char gps_start1[7];
char gps_start2[7];

uint8_t gps_start1_ptr = 0;
uint8_t gps_start2_ptr = 0;

volatile uint8_t gps_prefix1_f = 0;
volatile uint8_t gps_prefix2_f = 0;

char gpsmode = 0;

char instring = 0;

uint8_t lcd_x1 = 0;
uint8_t lcd_x2 = 0;
uint8_t lcd_y1 = 0;
uint16_t lcd_y2 = 168;

void init_uart(void)
{
	/* Configure 9600 baud, 8-bit, no parity and one stop bit */
	const int baud_rate = 9600;
	UBRR0H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR0L = (F_CPU/(baud_rate*16L)-1);
	UCSR0B = _BV(RXEN0) | _BV(TXEN0) | _BV(RXCIE0);
	UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);
	
	UBRR1H = (F_CPU/(baud_rate*16L)-1) >> 8;
	UBRR1L = (F_CPU/(baud_rate*16L)-1);
	UCSR1B = _BV(RXEN1) | _BV(TXEN1) | _BV(RXCIE1);
	UCSR1C = _BV(UCSZ10) | _BV(UCSZ11);
}


ISR(USART0_RX_vect) 
{	

	buff[buff_ptr_h] = UDR0;	
	buff_ptr_h++;
	if (buff_ptr_h == BUFF_MAX)
		buff_ptr_h = 0;
}

ISR(USART1_RX_vect) 
{	
	buff2[buff_ptr_h2] = UDR1;	
	buff_ptr_h2++;
	if (buff_ptr_h2 == BUFF_MAX)
		buff_ptr_h2 = 0;
}

uint8_t checknmea(char in, uint8_t pos)
{
	const uint8_t text[] = "$GPGGA";
	
	if (text[pos] == in)
		return 1;
	else
		return 0;
	

}

void uart1(char c)
{
	PORTB=0xFF;
	display.y = lcd_y1;
	display.x = lcd_x1;
	

	

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

	if (display.y >= 152)
		display.y=0;
		
	lcd_y1 = display.y;
	lcd_x1 = display.x;
	PORTB=0x0;
}


void uart2(char c)
{
	display.y = lcd_y2;
	display.x = lcd_x2;

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
		display.y=168;
		
	lcd_y2 = display.y;
	lcd_x2 = display.x;
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
	
	
		if(buff_ptr_h != buff_ptr_t)
		{
			PORTB=0xFF;
			cli();		
			c = buff[buff_ptr_t];
			buff_ptr_t++;
			if (buff_ptr_t == BUFF_MAX)
				buff_ptr_t = 0;
			sei();
			
			if (c == '$'){
				gps_start1_ptr = 1;
			}else{
				if (gps_start1_ptr > 0){
					if(checknmea(c,gps_start1_ptr))		
						gps_start1_ptr++;
					else
						gps_start1_ptr = 0;
					
					if (gps_start1_ptr > 5){
						gps_prefix1_f = 1;
						gps_start1_ptr = 0;
						gpsmode = 1;
					}
				}
			}
			
			if (gpsmode==0 || gps_prefix1_f)
				uart1(c);
			
			if (c == '\n')
				gps_prefix1_f = 0;
			PORTB=0x0;
		}
		
		
		if(buff_ptr_h2 != buff_ptr_t2)
		{
			PORTB=0xFF;
			cli();		
			c = buff2[buff_ptr_t2];
			buff_ptr_t2++;
			if (buff_ptr_t2 == BUFF_MAX)
				buff_ptr_t2 = 0;
			sei();
			
			if (c == '$'){
				gps_start2_ptr = 1;
			}else{
				if (gps_start2_ptr > 0){
					if(checknmea(c,gps_start2_ptr))		
						gps_start2_ptr++;
					else
						gps_start2_ptr = 0;
					
					if (gps_start2_ptr > 5){
						gps_prefix2_f = 1;
						gps_start2_ptr = 0;
						gpsmode = 1;
					}
				}
			}
			
			if (gpsmode==0 || gps_prefix2_f)
				uart2(c);
			
			if (c == '\n')
				gps_prefix2_f = 0;
			PORTB=0x0;
		}
	
	
		/*
			if(buff_ptr_h != buff_ptr_t)
			{
				cli();		
				c = buff[buff_ptr_t];
				buff_ptr_t++;
				if (buff_ptr_t == BUFF_MAX)
					buff_ptr_t = 0;
				sei();
				uart1();
				
			}

			if(buff_ptr_h2 != buff_ptr_t2)
				uart2();	
		}*/
	}

	return 0;
}

	
	