#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

SERVICE *service1;
SERVICE *service2;
SERVICE *service3;

void subscriber1()
{
    SET_PIN9;
    int16_t v = 1;
    for(;;)
    {
        delay_ms(v);
        CLEAR_PIN9;
        Service_Subscribe(service1, &v);
        SET_PIN9;
    }
}

void subscriber2()
{
    SET_PIN10;
    int16_t v = 1;
    for(;;)
    {
        delay_ms(v);
        CLEAR_PIN10;
        Service_Subscribe(service2, &v);
        SET_PIN10;
    }
}

void subscriber3()
{
    SET_PIN11;
    int16_t v = 1;
    for(;;)
    {
        delay_ms(v);
        CLEAR_PIN11;
        Service_Subscribe(service3, &v);
        SET_PIN11;
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    SET_PIN10_OUT;
    SET_PIN11_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;
    CLEAR_PIN10;
    CLEAR_PIN11;

    service1 = Service_Init();
    service2 = Service_Init();
    service3 = Service_Init();

    Task_Create_System(subscriber1, 2);
    Task_Create_System(subscriber2, 3);
    Task_Create_System(subscriber3, 4);

    _delay_ms(3);

    for(;;)
    {
        Task_Next();

        SET_PIN8;
        Service_Publish(service1, 1);
        Service_Publish(service2, 2);
        Service_Publish(service3, 3);
        CLEAR_PIN8;
    }

    return 0;
}
