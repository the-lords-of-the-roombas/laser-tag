#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

SERVICE *service;

void publisher()
{
    int16_t count = 0;

    for(;;)
    {
        count = 5;
        while(count)
        {
            SET_PIN8;
            Service_Publish(service, count);
            count--;
        }
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    service = Service_Init();

    Task_Create_System(publisher, 2);

    _delay_ms(20);

    for(;;)
    {
        int16_t val;
        Service_Subscribe(service, &val);        
        CLEAR_PIN8;

        SET_PIN9;
        for(int i = 0; i < val; ++i)
            _delay_ms(1);
        CLEAR_PIN9;
    }

    return 0;
}
