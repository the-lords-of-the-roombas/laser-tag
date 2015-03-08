#ifndef RTOS_TEST_UTIL_INCLUDED
#define RTOS_TEST_UTIL_INCLUDED

#include  <avr/sfr_defs.h>

#define LED_ON (PORTB |= _BV(PB7))
#define LED_OFF (PORTB &= ~_BV(PB7))

#define Disable_Interrupt()     asm volatile ("cli"::)
#define Enable_Interrupt()     asm volatile ("sei"::)

#endif // RTOS_TEST_UTIL_INCLUDED
