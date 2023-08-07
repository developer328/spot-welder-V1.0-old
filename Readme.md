MicroWave TransFormer Spot Welder
============================================


# Version
- V1.0



# Description
 - The MicroWave TransFormer Spot Welder is a device that allows you to perform spot welding using a microwave transformer. This code provides the firmware for controlling the spot welder. It includes features such as temperature monitoring, pulse timing control, and user interface with an OLED display and rotary encoder.

# Updates
- 06/16/2023: Initial release of the code.
- 06/18/2023: Updated version with bug fixes and improvements.


# Requirements
- Microcontroller: AVR
- Development Environment: AVR-GCC
- Libraries: stdio.h, stdint.h, stdbool.h, avr/io.h, util/delay.h, avr/interrupt.h, string.h, u8g2.h, u8x8_avr.h



# Features
- Temperature monitoring: The code reads temperature values from NTC thermistors using ADC and converts them to Celsius.
- Pulse timing control: Allows the user to adjust pulse timings for spot welding using a rotary encoder.
- User interface: Utilizes an OLED display and rotary encoder for user interaction and menu navigation.
- Interrupt handling: Uses interrupts for button presses and encoder rotation detection.
- Error handling: Monitors temperature values and displays warnings if the temperature exceeds a certain threshold.


# Usage
- Set up the necessary hardware components, including the microcontroller, OLED display, rotary encoder, and thermistors.
Install the required libraries mentioned in the requirements section.
Compile the code using an AVR-GCC development environment.
Upload the compiled firmware to the microcontroller.
Connect the necessary peripherals and power supply to the spot welder.
Power on the spot welder and use the rotary encoder to navigate through the menu and adjust pulse timings.
Follow the warnings and temperature readings on the OLED display to ensure safe operation.


# Hardware details

## MCU
 - ATmega328p -U 


## Main parts
 - OLED 128x64 I2C
 - Rotary encoder with Switch
 - Weld Switch 
 - 2x NTC sensor
 - SSR40 DA


## Pins
 - OLED:  A4 SDA, A5 SCL
 - Rotary encoder A, B, SW:  9 PINB2, 10 PINB4, 3 INT1
 - Welding switch:  2 INT0
 - SSR NTC tempSens:  A1 ADC1
 - Transformer NTC tempSens:  A0 ADC0
 - SSR40 DA:  8 PB1


# SoftWare details

## Functions
- void show_temps(int temp0_val, int temp1_val);
- void delay_page(void);
- void weld_pulse_page(void);
- void turn_off_page(void);
- void warnings_page(void);
- void weld_page(void);
- void oled_setup(void);
- void IO_setup(void);
- void ext_isr_setup(void);
- void cut_off(void);
- void work(void);
- void encoder_rotary(void);
- void welding(void);
- void select_change(void);
- void select(void);
- void change_selected_val(void);
- void R_limiter(int min, int max);

- int ADC_read(int NTC_pin);
- int temp_convert(int ADC_val);


## Varibles

- #define SSD1306_ADDR  0x78
- #define ntc_R1 10000
- #define ntc_R2 10000

- volatile int R_count = 1;
- volatile bool A_state = 0;
- volatile bool B_state = 0;
- volatile bool int0_val = 0;
- volatile uint8_t btn_count = 0;
- volatile bool btn_state = 0;
- volatile bool safe_val = 1;
- volatile bool blink_val = 0;

- bool blink_stop = 0;
- bool equal_stop_val0 = 0;
- bool equal_stop_val1 = 0;

- uint16_t pulse0_time = 150;
- uint16_t pulse_delay_time = 80;
- uint16_t pulse1_time = 100;

- int space_1, space_2, space_3, cursor_Y = 15;

- uint8_t R_count_last;

- u8g2_t u8g2;


## ISR vectors
- INT0_vect
- INT1_vect
- PCINT0_vect



## UI

### Show values on display
 - Weld pulse
 - SSR temperature
 - Transformer temperature


### Show warnings on display
 - High temperature


### Changeable values on display
 - Weld pulse0 (ms)
 - delay (ms)
 - Weld pulse1 (ms)


### rotary encoder use
 - Change value : Turn encoder
 - Select value : Turn encoder
 - Allow to change value: Push encoder switch 

 - right first pin 9
 - left first pin 10



