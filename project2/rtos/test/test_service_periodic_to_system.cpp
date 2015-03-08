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
        Service_Publish(service, count);
        Task_Next();

        ++count;
    }
}

int r_main()
{
    service = Service_Init();

    Task_Create_Periodic(publisher, 2, 20, 3, 0);
    Task_Periodic_Start();

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
