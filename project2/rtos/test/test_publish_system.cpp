#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"

SERVICE *service;

void receive(SERVICE *service)
{
    int16_t value;

    Service_Subscribe(service, &value);

    PORTB = _BV(PORTB7);
    for(int t = 0; t < 10; ++t)
        _delay_ms(100);
    PORTB = 0;
    _delay_ms(200);

    while(value--)
    {
        PORTB = _BV(PORTB7);
        _delay_ms(200);
        PORTB = 0;
        _delay_ms(200);
    }
}

void publisher()
{
    {
        int t = 10;
        while(t--)
        {
            _delay_ms(100);
        }

        Service_Publish(service, 5);
    }
}

int r_main()
{
    service = Service_Init();

    for(;;)
    {
        Task_Create_System(publisher, 1);
        receive(service);
    }

    return 0;
}
