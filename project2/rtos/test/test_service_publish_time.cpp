#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

SERVICE *service;

void task()
{
    for(;;)
    {
        int16_t v;
        Service_Subscribe(service, &v);
        CLEAR_PIN8;
        SET_PIN9;
        _delay_ms(1);
        CLEAR_PIN9;
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    service = Service_Init();

    for(int t = 0; t < 10; ++t)
    {
        Task_Create_System(task, 2);

        for(int i = 0; i < 5; ++i)
        {
            Task_Next();

            _delay_ms(1);
            SET_PIN8;
            Service_Publish(service, 0);
        }
    }

    return 0;
}


