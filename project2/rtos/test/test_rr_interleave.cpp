#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void task1()
{
    for(;;)
    {
        SET_PIN8;
        _delay_ms(50);
        CLEAR_PIN8;
        _delay_ms(50);
    }
}
void task2()
{
    for(;;)
    {
        SET_PIN9;
        _delay_ms(50);
        CLEAR_PIN9;
        _delay_ms(50);
    }
}
void task3()
{
    for(;;)
    {
        SET_PIN10;
        _delay_ms(50);
        CLEAR_PIN10;
        _delay_ms(50);
    }
}
void task4()
{
    for(;;)
    {
        SET_PIN11;
        _delay_ms(50);
        CLEAR_PIN11;
        _delay_ms(50);
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

    Task_Create_RR(task1, 2);
    Task_Create_RR(task2, 3);
    Task_Create_RR(task3, 4);
    Task_Create_RR(task4, 5);

    return 0;
}
