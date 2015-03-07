#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"

int r_main()
{
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

    return 0;
}
