#include "scheduler.h"
#include "servo_control.h"

static bool joystick_was_pressed = false;
static task_t tasks[2];

void read_and_report_joystick_press()
{
    bool is_pressed = read_joystick_pressed();

    if (!joystick_was_pressed && is_pressed)
        Serial.println("BANG!");

    joystick_was_pressed = is_pressed;
}

// Just a dummy testing task:
#if 0
void print_something()
{
    Serial.println("TASK IS RUNNING");
}

void init_print_something(int task_idx)
{
    scheduler_task_init(tasks+task_idx, 0, 500, &print_something);
}
#endif

void setup()
{
    //Serial.begin(9600);

    joystick_init();

    scheduler_task_init(tasks+0, 0, 20, &read_joystick_position_and_control_servo);
    scheduler_task_init(tasks+1, 10, 40, &read_and_report_joystick_press);

    scheduler_init(tasks, 2);
}

void loop()
{
    milliseconds_t next_time = scheduler_run();
}
