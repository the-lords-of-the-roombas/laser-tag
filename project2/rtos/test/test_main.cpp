#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

int r_main()
{
    for(int i = 0; i < 10; ++i)
    {
        _delay_ms(100);
    }

    for(int i = 0; i < 3; ++i)
    {
        LED_ON;
        _delay_ms(200);
        LED_OFF;
        _delay_ms(200);
    }

    return 0;
}
