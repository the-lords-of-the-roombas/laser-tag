#include "joystick.h"
#include "modified_servo.h"
#include <Arduino.h>

static const int servo_control_pin = 9;
static const int sw_pin = 8;  //create joystick switch object on digital 8
static const int x_pin = 0;  //create joystick analog position object on analog 0
static const int y_pin = 1;

Servo servo;

void joystick_init(joystick_state *jstick)
{ 
    filter_init(&jstick->filter);

    jstick->servo_control = 0;
    jstick->code_idx = 0;
    jstick->previous_joystick_y_state = 0;
    jstick->is_pressed = false;

    servo.attach(servo_control_pin);

    pinMode(sw_pin, INPUT);
    digitalWrite(sw_pin, HIGH);
} 
 
void joystick_read_position_and_control_servo(void*object)
{
    joystick_state* jstick = (joystick_state*)object;

    // Lo-pass filter the input
    int in_raw = analogRead(x_pin);
    int in = filter_process(&jstick->filter, in_raw);

    // Map input to range -100 to 100
    int step = map(in, 0, 380, -100, 100);

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
    int servo_ctl = jstick->servo_control + step;
    int range = 3000;
    if(servo_ctl < 0) servo_ctl = 0;
    if(servo_ctl > range) servo_ctl = range;
    jstick->servo_control = servo_ctl;

    // Map control variable to servo degrees and output that to servo
    int servo_degrees = map(servo_ctl, 0, range, 0, 180);
    servo.write(servo_degrees);
}

void joystick_read_pressed(void *object)
{
    joystick_state* jstick = (joystick_state*)object;
    jstick->is_pressed = !digitalRead(sw_pin);
}

void joystick_update_code_selection(void *object)
{
    joystick_state* jstick = (joystick_state*)object;

    int y = analogRead(y_pin);
    int joystick_state;
    if (y < 90)
        joystick_state = -1;
    else if(y > 270)
        joystick_state = 1;
    else
        joystick_state = 0;

    if (joystick_state != jstick->previous_joystick_y_state && joystick_state)
    {
        int code_idx = jstick->code_idx + joystick_state;
        if (code_idx >= 4)
            code_idx = 0;
        else if(code_idx < 0)
            code_idx = 3;
        jstick->code_idx = code_idx;
    }

    jstick->previous_joystick_y_state = joystick_state;
}
