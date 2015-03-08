#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void system_task()
{
    SET_PIN9;
    _delay_ms(1);
    CLEAR_PIN9;
}

void periodic_task()
{
    for(;;)
    {
        SET_PIN10;
        _delay_ms(1);
        CLEAR_PIN10;
        Task_Next();
    }
}

void rr_task()
{
    for(;;)
    {
        _delay_ms(20);

        SET_PIN8;
        Task_Create_System(system_task, 4);
        CLEAR_PIN8;
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    SET_PIN10_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;
    CLEAR_PIN10;

    Task_Create_RR(rr_task, 2);
    Task_Create_Periodic(periodic_task, 3, 1, 1, 5);
    Task_Periodic_Start();

    return 0;
}


