#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"

SERVICE *service;

void receive(SERVICE *service)
{
    int16_t value;

    Service_Subscribe(service, &value);

    PORTB = _BV(PORTB7);
    for (int i = 0; i < value; ++i)
        _delay_ms(100);
    PORTB = 0;
}

void publisher()
{
    for(int i = 1; i < 4; ++i)
    {
        Service_Publish(service, i);
        Task_Next();
    }
}

int r_main()
{
    service = Service_Init();

    Task_Create_Periodic(publisher, 0, 200, 1, 200);
    Task_Periodic_Start();

    // All 3 should complete
    receive(service);
    receive(service);
    receive(service);
    // Last time should never complete
    receive(service);

    return 0;
}
