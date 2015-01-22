#include "servo_control.h"
#include "modified_servo.h"
#include <Arduino.h>

Servo myservo;  // create servo object to control a servo 
const int SwButtonPin = 8;  //create joystick switch object on digital 8
int potpin = 0;  //create joystick analog position object on analog 0
int val;
int pos = 0;
 
void joystick_init() 
{ 
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object 
  pinMode(SwButtonPin, INPUT); //sets the SW pin as input
  digitalWrite(SwButtonPin, HIGH); //sets the SW button high
} 
 
void read_joystick_position_and_control_servo(){
    val = analogRead(potpin);
    int pos_step = map(val, 0, 360, -10, 10);
    pos = pos + pos_step;
    if(pos < 0) pos = 0;
    if(pos > 1000) pos = 1000;
    int servo_pos = pos * 0.18;
    myservo.write(servo_pos);
}

int read_joystick_pressed(){
    return digitalRead(SwButtonPin);
}
