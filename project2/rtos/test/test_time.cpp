#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    _delay_ms(4);

    for(;;)
    {
        for(int measure_duration = 3;
            measure_duration < 15;
            measure_duration += 3)
        {
            SET_PIN8;
            uint16_t measure_start = Now();
            delay_ms(measure_duration);
            uint16_t measure_end = Now();
            CLEAR_PIN8;

            uint16_t duration = measure_end - measure_start;

            SET_PIN9;
            delay_ms(duration);
            CLEAR_PIN9;
        }
    }

    return 0;
}
