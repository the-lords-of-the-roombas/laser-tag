#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void system_task()
{
    SET_PIN9;
    // Wait 10 ms = 2 ticks
    _delay_ms(10);
    CLEAR_PIN9;
}

void periodic_task()
{
    for(;;)
    {
        SET_PIN8;
        Task_Create_System(system_task, 3);
        CLEAR_PIN8;
        Task_Next();
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    // Periodic task has WCET 1 tick, but it is being preempted by
    // the system task for 2 ticks.
    // However, its allowed running time should be extended when preempted.
    Task_Create_Periodic(periodic_task, 2, 5, 1, 1);
    Task_Periodic_Start();

    return 0;
}
