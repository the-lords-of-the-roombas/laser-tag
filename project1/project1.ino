#include "scheduler.h"
#include "servo_control.h"
#include "gun.h"

static bool joystick_was_pressed = false;
static const int task_count = 4;
static task_t tasks[task_count];
static char codes[4] = { 'A', 'B', 'C', 'D' };
static int code_idx = 3;

static int code_indicator_lo_pin = 2;
static int code_indicator_hi_pin = 3;

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
        char code = codes[code_idx];
        Serial.print("BANG: ");
        Serial.println(code);

        gun_trigger(code);
    }

    joystick_was_pressed = is_pressed;
}

void read_and_store_current_code()
{
    int new_code_idx = read_gun_code();

    if (code_idx != new_code_idx)
    {
        Serial.println("NEW CODE: ");
        Serial.println(new_code_idx);
        Serial.println(codes[new_code_idx]);
    }

    code_idx = new_code_idx;

    int code_low_bit = (code_idx >> 0) & 0x1;
    int code_hi_bit = (code_idx >> 1) & 0x1;

    digitalWrite(code_indicator_lo_pin, code_low_bit ? HIGH : LOW);
    digitalWrite(code_indicator_hi_pin, code_hi_bit ? HIGH : LOW);
}

// Just a dummy testing task:

void print_something()
{
    Serial.println("TASK IS RUNNING");
}

void setup()
{
    Serial.begin(9600);

    pinMode(code_indicator_lo_pin, OUTPUT);
    pinMode(code_indicator_hi_pin, OUTPUT);

    joystick_init();
    gun_init();

    //scheduler_task_init(tasks+0, 0, 500e3, &print_something);

    scheduler_task_init(tasks+0, 0, 20e3, &read_joystick_position_and_control_servo);
    scheduler_task_init(tasks+1, 10e3, 40e3, &read_joystick_press_and_shoot);
    scheduler_task_init(tasks+2, 20e3, 40e3, &read_and_store_current_code);
    scheduler_task_init(tasks+3, 500, 500, &gun_task);

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
