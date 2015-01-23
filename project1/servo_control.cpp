#include "servo_control.h"
#include "modified_servo.h"
#include <Arduino.h>

static Servo myservo;  // create servo object to control a servo
static const int sw_pin = 8;  //create joystick switch object on digital 8
static int x_pin = 0;  //create joystick analog position object on analog 0
static int y_pin = 1;

static int servo_pos = 0;

static int code_idx = 0;
static int previous_joystick_y_state = 0;
 
void joystick_init() 
{ 
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  pinMode(sw_pin, INPUT); //sets the SW pin as input
  digitalWrite(sw_pin, HIGH); //sets the SW button high
} 
 
void read_joystick_position_and_control_servo(){
    int val = analogRead(x_pin);
    int pos_step = map(val, 0, 360, -10, 10);
    servo_pos = servo_pos + pos_step;
    if(servo_pos < 0) servo_pos = 0;
    if(servo_pos > 1000) servo_pos = 1000;
    int servo_pos = servo_pos * 0.18;
    myservo.write(servo_pos);
}

int read_joystick_pressed(){
    return !digitalRead(sw_pin);
}

int read_gun_code()
{
    int y = analogRead(y_pin);
    int joystick_state;
    if (y < 90)
        joystick_state = -1;
    else if(y > 270)
        joystick_state = 1;
    else
        joystick_state = 0;

    if (joystick_state != previous_joystick_y_state && joystick_state)
    {
        code_idx = code_idx + joystick_state;
        if (code_idx >= 4)
            code_idx = 0;
        else if(code_idx < 0)
            code_idx = 3;
    }

    previous_joystick_y_state = joystick_state;

    return code_idx;
}
