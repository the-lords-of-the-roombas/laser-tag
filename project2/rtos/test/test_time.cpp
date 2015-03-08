#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

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
                LED_ON;
            }
            else
            {
                timeout = 900;
                LED_OFF;
            }
            then = now;
        }
    }

    return 0;
}
