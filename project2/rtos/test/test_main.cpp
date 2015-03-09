#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

int r_main()
{
    SET_PIN8_OUT;

    for(;;)
    {
        SET_PIN8;
        _delay_ms(5);
        CLEAR_PIN8;
        _delay_ms(5);
    }

    return 0;
}
