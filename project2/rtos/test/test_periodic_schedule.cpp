#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

void set_led_on()
{
    for(;;)
    {
        LED_ON;
        Task_Next();
    }
}

void set_led_off()
{
    for(;;)
    {
        LED_OFF;
        Task_Next();
    }
}

int r_main()
{
    Task_Create_Periodic(set_led_on, 2, 200, 2, 0);
    Task_Create_Periodic(set_led_off, 3, 200, 2, 300);

    for(int i = 0; i < 10; ++i)
        _delay_ms(100);

    Task_Periodic_Start();

    return 0;
}
