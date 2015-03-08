#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"

void blink_led()
{
    PORTB = _BV(PORTB7);
    _delay_ms(100);
    PORTB = 0;
}

void blinker()
{
    for(;;)
    {
        blink_led();
        Task_Next();
    }
}

int r_main()
{
    Task_Create_Periodic(blinker, 0, 200, 30, 0);
    Task_Create_Periodic(blinker, 0, 200, 30, 500);

    for(int i = 0; i < 10; ++i)
        _delay_ms(100);

    Task_Periodic_Start();

    return 0;
}
