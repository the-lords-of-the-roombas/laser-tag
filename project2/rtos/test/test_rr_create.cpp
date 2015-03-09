#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "../arduino_pins.h"
#include "test_util.h"

void task()
{
    for(;;) {
        SET_PIN9;
        _delay_ms(2);
        CLEAR_PIN9;
        _delay_ms(2);
    }
}

int r_main()
{
    SET_PIN8_OUT;
    SET_PIN9_OUT;
    CLEAR_PIN8;
    CLEAR_PIN9;

    _delay_ms(5);

    SET_PIN8;
    Task_Create_RR(task, 2);
    CLEAR_PIN8;

    _delay_ms(5);

    return 0;
}

