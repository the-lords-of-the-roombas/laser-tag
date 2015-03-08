#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

SERVICE *service;

void publisher()
{
    for(;;)
    {
        SET_PIN9;
        SET_PIN8;
        Service_Publish(service, 0);
        _delay_ms(1);
        CLEAR_PIN9;
        _delay_ms(1);
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    service = Service_Init();

    Task_Create_RR(publisher, 2);

    _delay_ms(5);

    for(;;)
    {
        int16_t val;
        Service_Subscribe(service, &val);
        CLEAR_PIN8;
    }

    return 0;
}
