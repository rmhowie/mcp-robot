//Example ATmega2560 Project
//File: ATmega2560Project.h
//Author: Robert Howie

#ifndef ROBOT_H_ //double inclusion guard
#define ROBOT_H_

//include standard libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/delay.h>
#include <stdio.h>

//include header files
#include "../serial.h" //minimal serial lib
#include "../adc.h" //minimal adc lib
#include "../milliseconds.h" //milliseconds timekeeping lib

//constants
#define BUILD_DATE __TIME__ " " __DATE__"\n"

#define MOTOR_PWM_TOP (10000-1)
#define MOTOR_DRIVE_SCALING_LIMIT 123

#define MOTOR_IN1_DDR DDRA
#define MOTOR_IN1_PORT PORTA
#define MOTOR_IN1 0

#define MOTOR_IN2_DDR DDRA
#define MOTOR_IN2_PORT PORTA
#define MOTOR_IN2 1

#define MOTOR_IN3_DDR DDRA
#define MOTOR_IN3_PORT PORTA
#define MOTOR_IN3 2

#define MOTOR_IN4_DDR DDRA
#define MOTOR_IN4_PORT PORTA
#define MOTOR_IN4 3

#endif /* ATMEGA2560_H_ */