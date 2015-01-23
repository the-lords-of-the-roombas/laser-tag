
#ifndef UVIC_RTSYS_SERVO_CONTROL_INCLUDED
#define UVIC_RTSYS_SERVO_CONTROL_INCLUDED

void joystick_init();

void read_joystick_position_and_control_servo();

int read_joystick_pressed();

int read_gun_code();

#endif //UVIC_RTSYS_SERVO_CONTROL_INCLUDED
