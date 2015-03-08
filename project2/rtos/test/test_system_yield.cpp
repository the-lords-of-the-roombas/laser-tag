#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void yield()
{
    SET_PIN11;
    Task_Next();
    CLEAR_PIN11;
}

void task1()
{
    CLEAR_PIN11;
    for(;;)
    {
        SET_PIN8;
        _delay_ms(10);
        CLEAR_PIN8;
        yield();
    }
}

void task2()
{
    CLEAR_PIN11;
    for(;;)
    {
        SET_PIN9;
        _delay_ms(20);
        CLEAR_PIN9;
        yield();
    }
}

void task3()
{
    CLEAR_PIN11;
    for(;;)
    {
        SET_PIN10;
        _delay_ms(30);
        CLEAR_PIN10;
        yield();
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    SET_PIN10_OUT;
    SET_PIN11_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;
    CLEAR_PIN10;
    CLEAR_PIN11;

    _delay_ms(50);

    Task_Create_System(task1, 2);
    Task_Create_System(task2, 3);
    Task_Create_System(task3, 4);

    _delay_ms(50);

    return 0;
}
