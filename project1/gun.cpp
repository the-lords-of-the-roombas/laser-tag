#include "gun.h"

#include "Arduino.h"

bool g_enabled;
int g_current_bit;
uint16_t g_code;

#define GUN_TIMER_TOP 421
#define GUN_TIMER_HALF 210

static void gun_send_hi()
{
    OCR1C = GUN_TIMER_HALF;
}

static void gun_send_lo()
{
    OCR1C = 0;
}

void gun_init()
{
    pinMode(13, OUTPUT);

    // Clear control registers
    TCCR1A = 0;
    TCCR1B = 0;

    // Enable interrupt?
    TIMSK1 &= ~(1<<OCIE1C);

    //Set to Fast PWM (mode 15)
    TCCR1A |= (1<<WGM10) | (1<<WGM11);
    TCCR1B |= (1<<WGM12) | (1<<WGM13);

    //Enable output C.
    TCCR1A |= (1<<COM1C1);

    // 38kHz fequency = 0.00002631578947368421 second period
    // at 16Mhz and no prescaler, that is about 421 cycles.

    //No prescaler
    TCCR1B |= (1<<CS10);
    OCR1A = 421;  // Set TOP for 50 us period
    OCR1C = 0;  // Set TARGET for 50 % pulse width

    // State
    g_enabled = false;
    g_current_bit = 0;
    g_code = 0;
}

void gun_trigger(uint8_t code)
{
    if (g_enabled)
        return;

    g_enabled = true;
    g_code = 0;
    g_code |= code;
    g_code |= (0x10 << 8);
    g_current_bit = 0;
}

void gun_task()
{
    if (!g_enabled)
        return;

    if((g_code >> g_current_bit) & 0x1)
        gun_send_hi();
    else
        gun_send_lo();

    ++g_current_bit;

    if (g_current_bit >= 10)
        g_enabled = false;
}
