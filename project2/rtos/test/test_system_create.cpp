#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

static int task_id = 2;

void task()
{
    ++task_id;
    if (task_id > 5)
        task_id = 2;

    _delay_ms(1);

    SET_PIN8;
    Task_Create_System(task, task_id);
    CLEAR_PIN8;

    _delay_ms(1);
}

int r_main()
{
    SET_PIN8_OUT;
    CLEAR_PIN8;

    _delay_ms(1);

    SET_PIN8;
    Task_Create_System(task, task_id);
    CLEAR_PIN8;

    _delay_ms(1);

    return 0;
}


