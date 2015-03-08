#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

volatile int count = 0;

void task()
{
    for(int i = 0; i < 10; ++i)
    {
        _delay_ms(100);
    }

    for(int i = 0; i < 3; ++i)
    {
        LED_ON;
        _delay_ms(200);
        LED_OFF;
        _delay_ms(200);
    }

}

int r_main()
{
    Task_Create_RR(task, 2);

    for(int i = 0; i < 10; ++i)
    {
        _delay_ms(100);
    }

    return 0;
}

