#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"

extern int r_main();

void delay_30_ms()
{
    PORTB = (1 << PORTB7);
    _delay_ms(30);
    PORTB = 0;
}

void toggle_led()
{
    //DDRB = (1 << DDB7);

    bool on = true;

    for(;;)
    {
#if 0
        if (on)
            PORTB = (1 << PORTB7);
        else
            PORTB = 0;

        on = !on;
#endif
        Task_Create_System(delay_30_ms, 0);

        Task_Next();
    }
}

void dummy()
{
    for(;;)
    {
        Task_Next();
    }
}

void blink_led()
{
    DDRB = (1 << DDB7);

    uint8_t led_on = (1 << PORTB7);

    for(;;)
    {

        PORTB = led_on;

        _delay_ms(200);

        PORTB = 0;

        _delay_ms(200);
    }
}

int r_main()
{
    Task_Create_Periodic(toggle_led, 0, 100, 1, 0);
    Task_Create_Periodic(dummy, 0, 100, 5, 101);
    Task_Periodic_Start();

#if 0
    uint16_t then = Now();
    bool led_on = false;
    uint16_t timeout = 0;

    for(;;)
    {
        uint16_t now = Now();
        if (now - then >= timeout)
        {
            led_on = !led_on;
            if (led_on)
            {
                timeout = 100;
                PORTB = (1 << PORTB7);
            }
            else
            {
                timeout = 900;
                PORTB = 0;
            }
            then = now;
        }
    }
#endif
    return 0;
}
