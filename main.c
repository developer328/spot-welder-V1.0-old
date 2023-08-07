/*
MicroWave TransFormer Spot Welder

V1.0

updates before github:
06/16/2023 - base code
06/18/2023 - optimized varibles
07/08/2023 - reduced noises
*/


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>
#include <u8g2.h>
#include <u8x8_avr.h>

//Display
void show_temps(int temp0_val, int temp1_val);
void delay_page(void);
void weld_pulse_page(void);
void warnings_page(void);
void weld_page(void);
//configurations
void oled_setup(void);
void IO_setup(void);
void ext_isr_setup(void);
//logic
void cut_off(void);
void work(void);
void encoder_rotary(void);
void welding(void);
void select_and_change(void);
void select_val(void);
void change_selected_val(void);
void R_limiter(int min, int max);
//Functions with returns
int ADC_read(int NTC_pin);
int temp_convert(int ADC_val);


#define F_CPU 16000000
#define SSD1306_ADDR  0x78
#define ntc_R1 10000
#define ntc_R2 10000

//Rotary encoder
volatile int R_count = 1;
volatile bool A_state = 0;
volatile bool B_state = 0;
//INT
volatile bool int0_val = 0;
volatile uint8_t btn_count = 0;
volatile bool btn_state = 0;
volatile bool safe_val = 1;
volatile bool blink_val = 0;

bool blink_stop = 0;
bool equal_stop_val0 = 0;
bool equal_stop_val1 = 0;

uint16_t pulse0_time = 150;
uint16_t pulse_delay_time = 80;
uint16_t pulse1_time = 100;
int space_1, space_2, space_3, cursor_Y = 15;
uint8_t R_count_last;

u8g2_t u8g2;


int main(void)
{ 
	IO_setup();
	ext_isr_setup();
	oled_setup();
	sei();
	
	u8g2_SetFontRefHeightText(&u8g2);
	u8g2_SetFontPosTop(&u8g2);
	
	delay_page();
	_delay_ms(2000);
	
	while(1)
	{
		work();
		_delay_ms(1);
	}

	return 0;
}


ISR(INT0_vect)
{
	_delay_ms(50);
	if ((safe_val == 0) && (btn_count == 0) && (R_count == 4))
	{
		if (PIND & 0x04)
		{
			int0_val = 1;
		}
		else
		{
			int0_val = 0;
		}
	}
}


ISR(INT1_vect)
{
	if ((btn_state == 0) && (int0_val == 0))
	{
		btn_count++;
		btn_state = 1;
	}
	_delay_ms(70);	
}


ISR(PCINT0_vect)
{
	if (btn_state == 0)
	{
		blink_stop = 1;
		encoder_rotary();
		_delay_ms(1);
	}
}


void IO_setup(void)
{
	DDRB |= ~0x07;//pin 8, 9, 10 
	DDRD |= ~0x0C;//pin 2, 3
}


void ext_isr_setup(void)
{
	EICRA |= 0x0B;//INT0 falling and INT1 rising 
	EIMSK |= 0x03;//INT0 and INT1
	PCICR |= 0x01;//PCINT0
	PCMSK0 |= 0x06;//PCINT9 0x02, PCINT10 0x04
}


void oled_setup(void)
{
	u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_avr_hw_i2c, u8x8_avr_delay);
	u8g2_SetI2CAddress(&u8g2, SSD1306_ADDR);
	u8g2_InitDisplay(&u8g2);
	u8g2_SetPowerSave(&u8g2, 0);
}


void show_temps(int temp0_val, int temp1_val)
{   	
	char str_temp0_val[15];
	char str_temp1_val[15];
	sprintf(str_temp0_val, "T=%dC", temp0_val);    
	sprintf(str_temp1_val, "S=%dC", temp1_val);
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	u8g2_DrawStr(&u8g2, 0, 0, str_temp0_val);//temp0	16
	u8g2_DrawStr(&u8g2, 65, 0, str_temp1_val);//temp0	16
}


void weld_pulse_page(void)
{
	char str_Pulse0_time[10];
	char str_delay_time[10];
	char str_Pulse1_time[10];
	
	sprintf(str_Pulse0_time, "%d ms", pulse0_time);
	sprintf(str_delay_time, "%d ms", pulse_delay_time);
	sprintf(str_Pulse1_time, "%d ms", pulse1_time);
	
	u8g2_ClearBuffer(&u8g2);
	
	show_temps(temp_convert(ADC_read(0)), temp_convert(ADC_read(1)));
	
	select_and_change();
	
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	u8g2_DrawStr(&u8g2, 0, cursor_Y, "->");
	
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	
	u8g2_DrawStr(&u8g2, (0+space_1), 15, "Pulse0");
	u8g2_DrawStr(&u8g2, (0+space_2), 30, "Delay");	
	u8g2_DrawStr(&u8g2, (0+space_3), 45, "Pulse1");	
	
	u8g2_DrawStr(&u8g2, (55+space_1), 15, ":");
	u8g2_DrawStr(&u8g2, (55+space_2), 30, ":");
	u8g2_DrawStr(&u8g2, (55+space_3), 45, ":");
	
	u8g2_DrawStr(&u8g2, (62+space_1), 15, str_Pulse0_time);
	u8g2_DrawStr(&u8g2, (62+space_2), 30, str_delay_time);
	u8g2_DrawStr(&u8g2, (62+space_3), 45, str_Pulse1_time);
	
	u8g2_SendBuffer(&u8g2);
}


void weld_page(void)
{
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	u8g2_DrawStr(&u8g2, 20, 27, "Welding...");
	u8g2_SendBuffer(&u8g2);
}


void select_and_change(void)
{
	if (btn_count == 0)//selecting mode
	{  	
		if (blink_stop == 0)//for fast navigation
		{
			_delay_ms(100);
			blink_val = !blink_val;	
		}
		else
		{
			blink_val = 0;
		}
		
		equal_stop_val0 = 1;
		
		select_val();

		if (blink_val)
		{
			cursor_Y = -50;
		}
		
		blink_stop = 0;//allow cursor blinking
		btn_state = 0;//resets ISR1 allow val
	}
	else if (btn_count == 1)//selected value change mode
	{
		equal_stop_val1 = 1;
		
		change_selected_val();
		btn_state = 0;//resets ISR1 allow val
	}
	else if (btn_count >= 2)
	{
		btn_count = 0;
	}
}


void select_val(void)
{
	if (equal_stop_val1)
	{
		R_count = R_count_last;//remain last selected item
		equal_stop_val1 = 0;
	}
	R_limiter(1, 4);// limits menu can select only from 3 items.
	if (R_count == 1)
	{
		safe_val = 1;
		space_1 = 13;
		space_2 = 0;
		space_3 = 0;
		cursor_Y = 15;
		R_count_last = R_count;
	}
	else if (R_count == 2)
	{
		safe_val = 1;
		space_1 = 0;
		space_2 = 13;
		space_3 = 0;
		cursor_Y = 30;
		R_count_last = R_count;
	}
	else if (R_count == 3)
	{
		safe_val = 1;
		space_1 = 0;
		space_2 = 0;
		space_3 = 13;
		cursor_Y = 45;
		R_count_last = R_count;
	}
	else if (R_count == 4)
	{
		safe_val = 0;
		space_1 = 0;
		space_2 = 0;
		space_3 = 0;
		cursor_Y = -50;
		R_count_last = R_count;
	}
}


void change_selected_val(void)
{
	if (R_count_last == 1)
	{
		cursor_Y = 15;
		if (equal_stop_val0)
		{
			R_count = pulse0_time;//for start de.. or in.. crease from last value 
			equal_stop_val0 = 0;//for equation happens only once 
		}
		R_limiter(0, 500);
		pulse0_time = R_count;
	}
	else if (R_count_last == 2)
	{
		cursor_Y = 30;
		if (equal_stop_val0)
		{
			R_count = pulse_delay_time;//for start de.. or in.. crease from last value 
			equal_stop_val0 = 0;//for equation happens only once 
		}
		R_limiter(0, 500);
		pulse_delay_time = R_count;
	}
	else if (R_count_last == 3)
	{
		cursor_Y = 45;
		if (equal_stop_val0)
		{
			R_count = pulse1_time;//for start de.. or in.. crease from last value 
			equal_stop_val0 = 0;//for equation happens only once 
		}
		R_limiter(0, 500);
		pulse1_time = R_count;
	}
	else if (R_count_last == 4)
	{
		btn_count = 0;
	}
}


// limits rotary encoder max and min values
void R_limiter(int min, int max) 
{	
	if (R_count < min)
	{
		R_count = max;
	}
	else if (R_count > max)
	{
		R_count = min;
	}
}


void warnings_page(void)
{
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	u8g2_DrawStr(&u8g2, 0, 0, "Cooling...");
	u8g2_DrawStr(&u8g2, 0, 15, "OverHeated !!!");
	u8g2_DrawStr(&u8g2, 0, 27, "Wait until it");
	u8g2_DrawStr(&u8g2, 0, 37, "cools down.");
	u8g2_SendBuffer(&u8g2);
}


void delay_page(void)
{
	u8g2_ClearBuffer(&u8g2);
	u8g2_SetFont(&u8g2, u8g2_font_smart_patrol_nbp_tr);
	u8g2_DrawStr(&u8g2, 25, 27, "Wait...");
	u8g2_SendBuffer(&u8g2);
}


void encoder_rotary(void)
{
  if (!(PINB & 0x02) && (PINB & 0x04) &&  !B_state) {  // detect direction
	  B_state = 0;
	  A_state = 1;
  }

  if (!(PINB & 0x04) && (PINB & 0x02) && !A_state) {  // detect direction
	  A_state = 0;
	  B_state = 1;
  }

  if ((PINB & 0x04) && (PINB & 0x02)) {  //first or last state both are zero - reset
	  A_state = 0;
	  B_state = 0;
  }


  if (A_state) {
	  if (!(PINB & 0x04) && !(PINB & 0x02)) {  //when direction is recognized both need equal 1 for ++
		  if (R_count < 1000)
		  {
			   R_count++;
		  }
		  A_state = 0;  //reset
	  }
  }

  if (B_state) {
	  if (!(PINB & 0x04) && !(PINB & 0x02)) {  //when direction is recognized both need equal 1 for --
		  if (R_count > -1000)
		  {
			  R_count--;			  
		  }
		  B_state = 0;  // reset
	  }
  }
}


void cut_off(void)
{
	int0_val = 0;
	A_state = 0;
	B_state = 0;
}


void work(void)
{
	if ((temp_convert(ADC_read(0x00)) > 45) || (temp_convert(ADC_read(0x01)) > 40))
	{
		cut_off();
		warnings_page();
	}
	else
	{   
		weld_pulse_page();
		
		if (safe_val == 0)
		{
			if (int0_val == 1)
			{
				_delay_ms(10);
				weld_page();
				welding();
				delay_page();
				_delay_ms(500);
				int0_val = 0;
			}
		}
		else
		{
			int0_val = 0;
		}
	}
}
 

void welding(void)
{
	PORTB |= 0x01;
	for (uint16_t i = 0; i < pulse0_time; i++)
	{
		_delay_ms(1);
	}

	PORTB &= ~0x01;
	for (uint16_t i = 0; i < pulse_delay_time; i++)
	{
		_delay_ms(1);
	}
		
	PORTB |= 0x01;
	for (uint16_t i = 0; i < pulse1_time; i++)
	{
		_delay_ms(1);
	}
	PORTB &= ~0x01;
}


int ADC_read(int NTC_pin) {
	
	int ADC_val;
	
	ADMUX |= NTC_pin;
	ADMUX |= 0x40; 
	ADCSRA |= 0xC0; // Enable ADC and start conversion
	
	while (ADCSRA & (1 << ADSC));
	
	ADC_val = ADCL | (ADCH << 8);
	
	ADMUX &= 0x00;
	
	return ADC_val;
}


int temp_convert(int ADC_val)
{
  float Temp;
  //Temp = log(10000.0 / (1023.0 / ADC_val - 1));
  Temp = log(10000.0 * ((1024.0 / ADC_val) - 1)); // for pull-up configuration
  Temp = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 * Temp * Temp)) * Temp);
  Temp = Temp - 273.15; // Convert Kelvin to Celsius
  // Temp = (Temp * 9.0) / 5.0 + 32.0; // Convert Celsius to Fahrenheit
  _delay_ms(1);
  return (int)Temp;
}
