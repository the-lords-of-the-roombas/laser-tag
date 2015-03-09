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
        _delay_ms(1);
        CLEAR_PIN9;
        Task_Next();
        SET_PIN9;
    }
}

void task3()
{
    SET_PIN10;
    for(;;)
    {
        _delay_ms(1);
        CLEAR_PIN10;
        Task_Next();
        SET_PIN10;
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

    Task_Create_Periodic(task1, 2, 2, 1, 0);
    Task_Create_Periodic(task2, 3, 4, 1, 1);
    Task_Create_Periodic(task3, 4, 4, 1, 3);

    _delay_ms(4);

    Task_Periodic_Start();

    return 0;
}
