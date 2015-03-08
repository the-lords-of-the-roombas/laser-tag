#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"


SERVICE * volatile service;

void subscriber()
{
    // OS should abort here:
    int16_t val;
    Service_Subscribe(service, &val);
}

int r_main()
{
    service = Service_Init();

    // OS should abort at start of periodic task = 3 ticks
    Task_Create_Periodic(subscriber, 2, 1, 1, 3);
    Task_Periodic_Start();

    return 0;
}
