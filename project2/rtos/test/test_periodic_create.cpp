#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void task()
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

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;

    CLEAR_PIN8;
    CLEAR_PIN9;

    _delay_ms(20);

    SET_PIN8;
    Task_Create_Periodic(task, 2, 1, 1, 0);
    CLEAR_PIN8;

    _delay_ms(20);

    Task_Periodic_Start();

    return 0;
}



