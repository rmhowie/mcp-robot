//Example ATmega2560 Project
//File: ATmega2560Project.c
//Author: Robert Howie
//Created: March 2015
//An example file for second year mechatronics project

//include this .c file's header file
#include "Robot.h"

//static function prototypes, functions only called in this file
static void motors_init(void);
static void drive_motors(int16_t forwards_component, int16_t turn_right_component);

//file scope variables
static char serial_string[200] = {0}; //declare and initialise string for serial printing
static char lcd_string[33] = {0}; //declare and initialise string for LCD


int main(void)
{
	//variable declarations
	static uint32_t current_ms = 0;
	static uint8_t serial_fsm_state = 0;
	static uint8_t serial_byte_in = 0;
	
	static uint8_t parameter_in_1 = 0;
	static uint8_t parameter_in_2 = 0;
	static uint8_t parameter_in_3 = 0;
	static uint8_t parameter_in_4 = 0;
	static uint8_t parameter_in_5 = 0;
	
	static uint8_t temp_parameter_in_1 = 0;
	static uint8_t temp_parameter_in_2 = 0;
	static uint8_t temp_parameter_in_3 = 0;
	static uint8_t temp_parameter_in_4 = 0;
	static uint8_t temp_parameter_in_5 = 0;
	
	static uint8_t parameter_out_1 = 0;
	static uint8_t parameter_out_2 = 0;
	static uint8_t parameter_out_3 = 0;
	
	//initialisation section, runs once
	serial0_init(); //initialise serial0 subsystem
	serial1_init(); //initialise serial1 subsystem
	adc_init(); //initialse ADC
	milliseconds_init(); //initialise timer one to track milliseconds
	motors_init(); //initialise motors, DDRs & PWM
	_delay_ms(10); //short delay

	//main loop
	while(1)
	{
		current_ms = milliseconds;
		if(UCSR1A&(1<<RXC1)) //if new serial byte has arrived
		{
			serial_byte_in = UDR1; //move serial byte into variable
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
				case 4: //waiting for fourth parameter
				temp_parameter_in_4 = serial_byte_in;
				serial_fsm_state++;
				break;
				case 5: //waiting for fifth parameter
				temp_parameter_in_5 = serial_byte_in;
				serial_fsm_state++;
				break;
				case 6: //waiting for stop byte
				if(serial_byte_in == 254) //stop byte
				{
					parameter_in_1 = temp_parameter_in_1;
					parameter_in_2 = temp_parameter_in_2;
					parameter_in_3 = temp_parameter_in_3;
					parameter_in_4 = temp_parameter_in_4;
					parameter_in_5 = temp_parameter_in_5;
					
					//send back parameters after successful receive, this way data rate is controlled by the controller only
					serial1_write_byte(255); //send start byte
					serial1_write_byte((uint32_t)adc_read(0)*253/1023); //send first parameter
					serial1_write_byte((uint32_t)adc_read(1)*253/1023); //send second parameter
					serial1_write_byte((uint32_t)adc_read(2)*253/1023); //send third parameter
					serial1_write_byte(254); //send stop byte
					
					//sprintf(serial_string, "1:%d, 2:%d, 3:%d, 4:%d, 5:%d\n", parameter_in_1, parameter_in_2, parameter_in_3, parameter_in_4, parameter_in_5);
					//serial0_print_string(serial_string);
					
					//update robot based on new parameters here if you only want to run it after a new message
					drive_motors(253-parameter_in_3, parameter_in_4); //flip forwards component because joystick values are flipped
					
				} // if the stop byte is not received, there is an error, so no commands are implemented
				serial_fsm_state = 0; //do nothing next time except check for start byte (below)
				break;
			}
			if(serial_byte_in == 255) //if start byte is received
			{
				serial_fsm_state=1; //reset on 255
			}
		}

		//other robot code

	}
	return(1);
} //end main

//set up DDRs and PWM for motors
void motors_init(void)
{
	//put motor direction pins into output mode
	MOTOR_IN1_DDR |= (1<<MOTOR_IN1);
	MOTOR_IN2_DDR |= (1<<MOTOR_IN2);
	MOTOR_IN3_DDR |= (1<<MOTOR_IN3);
	MOTOR_IN4_DDR |= (1<<MOTOR_IN4);
	//set up timer1 for motor PWM
	DDRB |= (1<<DDB5)|(1<<DDB6); //set DDR bits to allow PWM output
	ICR1 = MOTOR_PWM_TOP; //set top value
	OCR1A = 0;
	OCR1B = 0;
	TCCR1A = (1<<COM1A1)|(1<<COM1B1); //enable PWM output, clear up mode
	TCCR1B = (1<<WGM13)|(1<<CS11); //16 bit phase and frequency correct mode, start timer, prescaler:8
} //end init_motors

//set motor direction bits and PWM compare values based on joystic inputs
void drive_motors(int16_t forwards_component, int16_t turn_right_component)
{
	int16_t left_motor; //-:reverse, 0:stopped, +:forwards
	int16_t right_motor; //-:reverse, 0:stopped, +:forwards
	
	left_motor = forwards_component+turn_right_component-253;
	right_motor = forwards_component-turn_right_component;
	
	//left motor output code
	if (left_motor >= 0) // if left motor is going forwards
	{
		MOTOR_IN1_PORT |= (1<<MOTOR_IN1); //set direction bits to forwards
		MOTOR_IN2_PORT &= ~(1<<MOTOR_IN2);
		if(left_motor >= MOTOR_DRIVE_SCALING_LIMIT)
		{
			OCR1A = MOTOR_PWM_TOP;
		}
		else
		{
			OCR1A = (int32_t)left_motor*MOTOR_PWM_TOP/MOTOR_DRIVE_SCALING_LIMIT;
		}
	}
	else
	{
		MOTOR_IN1_PORT &= ~(1<<MOTOR_IN1); //set direction bits to reverse
		MOTOR_IN2_PORT |= (1<<MOTOR_IN2);
		if(left_motor <= -MOTOR_DRIVE_SCALING_LIMIT)
		{
			OCR1A = MOTOR_PWM_TOP;
		}
		else
		{
			OCR1A = (int32_t)-left_motor*MOTOR_PWM_TOP/MOTOR_DRIVE_SCALING_LIMIT;
		}
	}
	
	//right motor output code
	if (right_motor >= 0) // if right motor is going forwards
	{
		MOTOR_IN3_PORT |= (1<<MOTOR_IN3); //set direction bits to forwards
		MOTOR_IN4_PORT &= ~(1<<MOTOR_IN4);
		if(right_motor >= MOTOR_DRIVE_SCALING_LIMIT)
		{
			OCR1B = MOTOR_PWM_TOP;
		}
		else
		{
			OCR1B = (int32_t)right_motor*MOTOR_PWM_TOP/MOTOR_DRIVE_SCALING_LIMIT;
		}
	}
	else
	{
		MOTOR_IN3_PORT &= ~(1<<MOTOR_IN3); //set direction bits to reverse
		MOTOR_IN4_PORT |= (1<<MOTOR_IN4);
		if(right_motor <= -MOTOR_DRIVE_SCALING_LIMIT)
		{
			OCR1B = MOTOR_PWM_TOP;
		}
		else
		{
			OCR1B = (int32_t)-right_motor*MOTOR_PWM_TOP/MOTOR_DRIVE_SCALING_LIMIT;
		}
	}
	
	sprintf(serial_string, "fc:%d, trc:%d, lm:%d rm:%d 1:%d 2:%d EnA_DC:%d 3:%d 4:%d EnB_DC:%d\n", forwards_component, turn_right_component, left_motor, right_motor, MOTOR_IN1_PORT&(1<<MOTOR_IN1), MOTOR_IN2_PORT&(1<<MOTOR_IN2), OCR1A, MOTOR_IN3_PORT&(1<<MOTOR_IN3), MOTOR_IN4_PORT&(1<<MOTOR_IN4), OCR1B);
	serial0_print_string(serial_string);
	
} //end drive_motors