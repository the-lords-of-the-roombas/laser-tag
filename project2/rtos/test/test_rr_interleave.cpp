#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

int volatile count = 4;

void work()
{
    _delay_ms(100);

    Disable_Interrupt();
    --count;

    if (!count)
        LED_ON;

    Enable_Interrupt();
}

int r_main()
{
    Task_Create_RR(work, 2);
    Task_Create_RR(work, 3);
    Task_Create_RR(work, 4);
    Task_Create_RR(work, 5);

    return 0;
}
