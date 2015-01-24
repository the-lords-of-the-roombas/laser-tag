#include "servo_control.h"
#include "modified_servo.h"
#include <Arduino.h>

static Servo myservo;  // create servo object to control a servo
static const int servo_control_pin = 9;
static const int sw_pin = 8;  //create joystick switch object on digital 8
static int x_pin = 0;  //create joystick analog position object on analog 0
static int y_pin = 1;

static int servo_pos = 0;

static int code_idx = 0;
static int previous_joystick_y_state = 0;
 
void joystick_init() 
{ 
  myservo.attach(servo_control_pin);
  pinMode(sw_pin, INPUT);
  digitalWrite(sw_pin, HIGH);
} 
 
void read_joystick_position_and_control_servo()
{
    static int in_1 = 0, out_1 = 0;

    // Lo-pass filter the input
    int in_0 = analogRead(x_pin);
    int v = (0.2 * in_0 + 0.8 * out_1);
    out_1 = v;
    in_1 = in_0;

    // Map input to range -100 to 100
    int step = map(v, 0, 380, -100, 100);

    // Don't do anything if inside the range -10 to 10
    if (step >= 10)
        step -= 10;
    else if (step <= -10)
        step += 10;
    else
        return;

    // Scale further down to range -10 to 10
    step = map(step, -100, 100, -10, 10);

    // Update control variable and keep it in range 0 to 3000
    servo_pos = servo_pos + step;
    int range = 3000;
    if(servo_pos < 0) servo_pos = 0;
    if(servo_pos > range) servo_pos = range;

    // Map control variable to servo degrees and output that to servo
    int servo_degrees = map(servo_pos, 0, range, 0, 180);
    myservo.write(servo_degrees);
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
