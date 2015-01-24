
#ifndef UVIC_RTSYS_SERVO_CONTROL_INCLUDED
#define UVIC_RTSYS_SERVO_CONTROL_INCLUDED

#include "filter.h"

struct joystick_state
{
    filter_state filter;
    int servo_control;
    int code_idx;
    int previous_joystick_y_state;
    bool is_pressed;
};

inline bool joystick_is_pressed(joystick_state * j)
{
    return j->is_pressed;
}

inline int joystick_code_index(joystick_state * j)
{
    return j->code_idx;
}

void joystick_init(joystick_state*);

void joystick_read_position_and_control_servo(void *);

void joystick_read_pressed(void*);

void joystick_update_code_selection(void*);

#endif //UVIC_RTSYS_SERVO_CONTROL_INCLUDED
