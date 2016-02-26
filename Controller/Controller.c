//Example ATmega2560 Project
//File: ATmega2560Project.c
//Author: Robert Howie
//Created: March 2015
//An example file for second year mechatronics project

//include this .c file's header file
#include "Controller.h"

//static function prototypes, functions only called in this file


//file scope variables
static char serial_string[200] = {0}; //declare and initialise string for serial printing
static char lcd_string[33] = {0}; //declare and initialise string for LCD

int main(void)
{
	//variable declarations
	static uint32_t current_ms = 0;
	static uint32_t last_send_ms = 0;
	
	static uint8_t serial_fsm_state = 0;
	static uint8_t serial_byte_in = 0;
	
	static uint8_t parameter_in_1 = 0;
	static uint8_t parameter_in_2 = 0;
	static uint8_t parameter_in_3 = 0;
	
	static uint8_t temp_parameter_in_1 = 0;
	static uint8_t temp_parameter_in_2 = 0;
	static uint8_t temp_parameter_in_3 = 0;
	
	//initialisation section, runs once
	serial0_init(); //initialise serial subsystem
	serial2_init(); //initialise serial subsystem
	_delay_ms(10); //short delay
	adc_init(); //initialse ADC
	lcd_init(); //initialise 
	milliseconds_init(); //initialise timer3 to track milliseconds
	_delay_ms(20);
	
	//main loop
	while(1)
	{
		current_ms = milliseconds;
		
		//sending section
		if(current_ms-last_send_ms >= 100) //sending rate controlled here
		{
			last_send_ms = current_ms;
			serial2_write_byte(255); //send start byte
			serial2_write_byte((uint32_t)adc_read(0)*253/1023); //send first parameter
			serial2_write_byte((uint32_t)adc_read(1)*253/1023); //send second parameter
			serial2_write_byte((uint32_t)adc_read(14)*253/1023); //send third parameter
			serial2_write_byte((uint32_t)adc_read(15)*253/1023); //send fourth parameter
			serial2_write_byte(0); //send fifth parameter
			serial2_write_byte(254); //send stop byte
		}
		
		//receiving section
		if(UCSR2A&(1<<RXC2)) //if new serial byte has arrived
		{
			serial_byte_in = UDR2; //move serial byte into variable
			switch(serial_fsm_state) //switch by the current state
			{
				case 0:
				//do nothing, if check after switch case will find start byte and set serial_fsm_state to 1
				break;
				case 1: //waiting for first parameter
				temp_parameter_in_1 = serial_byte_in;
				serial_fsm_state++;
				break;
				case 2: //waiting for second parameter
				temp_parameter_in_2 = serial_byte_in;
				serial_fsm_state++;
				break;
				case 3: //waiting for third parameter
				temp_parameter_in_3 = serial_byte_in;
				serial_fsm_state++;
				break;
				case 4: //waiting for stop byte
				if(serial_byte_in == 254) //stop byte
				{
					parameter_in_1 = temp_parameter_in_1;
					parameter_in_2 = temp_parameter_in_2;
					parameter_in_3 = temp_parameter_in_3;
					
					sprintf(serial_string, "1:%d, 2:%d, 3:%d\n", parameter_in_1, parameter_in_2, parameter_in_3);
					serial0_print_string(serial_string);
					
					//update controller based on new parameters here if you only want to run it after a new message
					
				} // if the stop byte is not received, there is an error, so no commands are implemented
				serial_fsm_state = 0; //do nothing next time except check for start byte (below)
				break;
			}
			if(serial_byte_in == 255) //if start byte is received
			{
				serial_fsm_state=1; //reset on 255
			}
		}
		
	}
	return(1);
} //end main