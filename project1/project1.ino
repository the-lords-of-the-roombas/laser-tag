#include "scheduler.h"
#include "servo_control.h"
#include "gun.h"

static bool joystick_was_pressed = false;
static task_t tasks[3];

void read_and_report_joystick_press()
{
    int is_pressed = read_joystick_pressed();

    Serial.print("Pressed: ");
    Serial.println(is_pressed);

    if (!joystick_was_pressed && is_pressed)
        Serial.println("BANG!");

    joystick_was_pressed = is_pressed;
}

void read_joystick_press_and_shoot()
{
    bool is_pressed = read_joystick_pressed();

    if (!joystick_was_pressed && is_pressed)
    {
        Serial.println("BANG!");

        gun_trigger('A');
    }

    joystick_was_pressed = is_pressed;
}

// Just a dummy testing task:

void print_something()
{
    Serial.println("TASK IS RUNNING");
}

void setup()
{
    Serial.begin(9600);

    joystick_init();
    gun_init();

    //scheduler_task_init(tasks+0, 0, 500e3, &print_something);

    scheduler_task_init(tasks+0, 0, 20e3, &read_joystick_position_and_control_servo);
    scheduler_task_init(tasks+1, 10e3, 40e3, &read_joystick_press_and_shoot);
    scheduler_task_init(tasks+2, 500, 500, &gun_task);

    const int task_count = 3;

    scheduler_init(tasks, task_count);

    for (int i = 0; i < task_count; ++i)
    {
        Serial.print("Task ");
        Serial.print(i+1);
        Serial.println(" :");
        Serial.println(tasks[i].period);
        Serial.println(tasks[i].delay);
    }
}

void loop()
{
    microseconds_t delay = scheduler_run();
/*
    Serial.print("Next task after:");
    Serial.println(delay);

    Serial.println("Task 1: ");
    Serial.println(tasks[1].period);
    Serial.println(tasks[1].next_time);
*/
}
