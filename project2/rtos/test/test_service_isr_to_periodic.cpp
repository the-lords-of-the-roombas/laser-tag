#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

void setup_timer()
{
    // Set up Timer 1 to count ticks...
    // Use 1/64 prescaler
    TCCR3B = (_BV(CS31) | _BV(CS30));
    // Time out after 25000 cycles = 0.1 sec
    OCR3A = TCNT3 + 25000U;
    // Clear timeout flag
    TIFR3 = _BV(OCF3A);
    // Enable interrupt
    TIMSK3 = _BV(OCIE3A);
}

SERVICE * volatile service;

int16_t interrupt_count = 0;

ISR(TIMER3_COMPA_vect)
{
    OCR3A += 25000U;
    ++interrupt_count;
    Service_Publish(service, interrupt_count);
}

void subscriber()
{
    // OS should abort here:
    int16_t val;
    Service_Subscribe(service, &val);

    // The following line should never execute:
    LED_ON;
}

int r_main()
{
    service = Service_Init();

    setup_timer();

    Task_Create_Periodic(subscriber, 2, 100, 10, 0);
    Task_Periodic_Start();

    return 0;
}
