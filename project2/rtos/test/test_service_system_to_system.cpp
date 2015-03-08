#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

SERVICE *service;

void publisher()
{
    int16_t count = 0;

    for(;;)
    {
        _delay_ms(100);

        Service_Publish(service, count);

        ++count;
    }
}

int r_main()
{
    service = Service_Init();

    Task_Create_System(publisher, 2);

    bool on = false;

    for(;;)
    {
        int16_t val;
        Service_Subscribe(service, &val);

        if (val % 5 == 0)
            on = !on;

        if (on)
            LED_ON;
        else
            LED_OFF;
    }

    return 0;
}
