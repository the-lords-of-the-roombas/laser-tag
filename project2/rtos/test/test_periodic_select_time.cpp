#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void task()
{
    for(;;)
    {
        SET_PIN8;
        _delay_ms(1);
        CLEAR_PIN8;
        Task_Next();
    }
}

static const int n = 7;

int r_main()
{
    SET_PIN8_OUT;
    CLEAR_PIN8;

    for(int i = 0; i < n; ++i)
    {
        Task_Create_Periodic(task, 3, n + 1, 1, i);
    }

    Task_Periodic_Start();

    return 0;
}



