#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void setup_timer()
{
    // Use 1/64 prescaler
    TCCR3B = (_BV(CS31) | _BV(CS30));
    // Time out after 2500 cycles = 10 ms
    OCR3A = TCNT3 + 2500U;
    // Clear timeout flag
    TIFR3 = _BV(OCF3A);
    // Enable interrupt
    TIMSK3 = _BV(OCIE3A);
}

SERVICE * volatile service;

static int16_t count = 5;

ISR(TIMER3_COMPA_vect)
{
    OCR3A += 2500U;

    SET_PIN8;
    Service_Publish(service, count);

    --count;
    if (!count)
        count = 5;
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    service = Service_Init();

    setup_timer();

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
