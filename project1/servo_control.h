$ servo_control.h
#ifndef UVIC_RTSYS_SERVO_CONTROL_INCLUDED
#define UVIC_RTSYS_SERVO_CONTROL_INCLUDED

#include <Servo.h>

void joystick_init();

void read_joystick_position();

int read_joystick_pressed();

#ednif //UVIC_RTSYS_SERVO_CONTROL_INCLUDED