#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void task1()
{
    SET_PIN8;
    for(;;)
    {
        _delay_ms(1);
        CLEAR_PIN8;
        Task_Next();
        SET_PIN8;
    }
}

void task2()
{
    SET_PIN9;
    for(;;)
    {
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;

    CLEAR_PIN8;
    CLEAR_PIN9;

    // Second task never yields, thus exceeding its WCET (2 ticks).
    // OS should abort at task 2 start + task 2 WCET = 2 ticks.
    Task_Create_Periodic(task1, 2, 5, 1, 0);
    Task_Create_Periodic(task2, 3, 5, 1, 1);

    _delay_ms(9);

    Task_Periodic_Start();

    return 0;
}
