#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

volatile int count = 0;

void task()
{
    bool on = false;

    for(int i = 0; i < 4; ++i)
    {
        on = !on;
        if(on)
            LED_ON;
        else
            LED_OFF;
        Task_Next();
    }
}

int r_main()
{
    Task_Create_Periodic(task, 2, 100, 2, 100);

    LED_ON;
    for(int i = 0; i < 10; ++i)
    {
        _delay_ms(100);
    }
    LED_OFF;

    Task_Periodic_Start();

    return 0;
}



