#ifndef RTOS_TEST_UTIL_INCLUDED
#define RTOS_TEST_UTIL_INCLUDED

#include <avr/sfr_defs.h>
#include <util/delay.h>

#define LED_ON (PORTB |= _BV(PB7))
#define LED_OFF (PORTB &= ~_BV(PB7))

#define Disable_Interrupt()     asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

inline void delay_ms(int count)
{
    for(int i = 0; i < count; ++i)
        _delay_ms(1);
}

#endif // RTOS_TEST_UTIL_INCLUDED
