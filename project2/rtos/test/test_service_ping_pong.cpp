#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

SERVICE *ping_to_pong;
SERVICE *pong_to_ping;

// NOTE: "Service_Publish" yields, so we have to
// yield before publishing to give other opportunity to subscribe

void ping()
{
    int16_t i = 0;
    for(;;)
    {
        Task_Next();

        Service_Publish(ping_to_pong, i);

        Service_Subscribe(pong_to_ping, &i);

        SET_PIN8;
        delay_ms(i + 1);
        CLEAR_PIN8;

        i = (i + 1) % 5;
    }
}

void pong()
{
    int16_t i = 0;
    for(;;)
    {
        Service_Subscribe(ping_to_pong, &i);

        SET_PIN9;
        delay_ms(i + 1);
        CLEAR_PIN9;

        i = (i + 1) % 5;

        Task_Next();

        Service_Publish(pong_to_ping, i);
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    ping_to_pong = Service_Init();
    pong_to_ping = Service_Init();

    _delay_ms(10);

    Task_Create_System(ping, 2);
    Task_Create_System(pong, 3);

    return 0;
}
