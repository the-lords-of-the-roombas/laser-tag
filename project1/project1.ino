#include "scheduler.h"
#include "joystick.h"
#include "gun.h"

static const int task_count = 4;
static task_t tasks[task_count];

static const char codes[4] = { 'A', 'B', 'C', 'D' };

static const int code_indicator_lo_pin = 2;
static const int code_indicator_hi_pin = 3;
static const int task_duration_pin = 4;

static gun_state gun;
static joystick_state joystick;

void detect_joystick_press_and_shoot(void*)
{
    bool was_pressed = joystick_is_pressed(&joystick);

    joystick_read_pressed(&joystick);

    bool is_pressed = joystick_is_pressed(&joystick);

    if (!was_pressed && is_pressed)
    {
        int code_idx = joystick_code_index(&joystick);

        char code = codes[code_idx];
        Serial.print("BANG: ");
        Serial.println(code);

        gun_trigger(&gun, code);
    }
}

void update_current_code(void*)
{
    int old_code_idx = joystick_code_index(&joystick);

    joystick_update_code_selection(&joystick);

    int code_idx = joystick_code_index(&joystick);

    if (code_idx != old_code_idx)
    {
        Serial.println("NEW CODE: ");
        Serial.println(code_idx);
        Serial.println(codes[code_idx]);
    }

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
    pinMode(task_duration_pin, OUTPUT);

    joystick_init(&joystick);
    gun_init(&gun);

    //scheduler_task_init(tasks+0, 0, 500e3, &print_something);

    scheduler_task_init(tasks+0, 0, 20e3, &joystick_read_position_and_control_servo, &joystick);
    scheduler_task_init(tasks+1, 5e3, 40e3, &detect_joystick_press_and_shoot, 0);
    scheduler_task_init(tasks+2, 10e3, 40e3, &update_current_code, 0);
    scheduler_task_init(tasks+3, 250, 500, &gun_transmit, &gun);

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
