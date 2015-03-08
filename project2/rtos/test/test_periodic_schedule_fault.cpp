#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

void blink_led()
{
    for(;;)
    {
        LED_ON;
        _delay_ms(20);
        LED_OFF;
        Task_Next();
    }
}

int r_main()
{
    // WCET of second task makes it overlap with the first task
    // OS should abort at start of second task.
    Task_Create_Periodic(blink_led, 2, 200, 10, 0);
    Task_Create_Periodic(blink_led, 3, 200, 150, 300);

    for(int i = 0; i < 10; ++i)
        _delay_ms(100);

    Task_Periodic_Start();

    return 0;
}
