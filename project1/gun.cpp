#include "gun.h"

#include "Arduino.h"

// 38kHz fequency = 0.00002631578947368421 second period
// at 16Mhz and no prescaler, that is about 421 cycles.
#define GUN_TIMER_TOP 421
#define GUN_TIMER_HALF 210

static void gun_send_hi()
{
    // Enable output C:
    TCCR1A |= (1<<COM1C1);

    //OCR1C = GUN_TIMER_HALF;
}

static void gun_send_lo()
{
    // Disable output C:
    TCCR1A &= ~(1<<COM1C1);

    //OCR1C = 0;
}

void gun_init(gun_state * gun)
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

    //No prescaler
    TCCR1B |= (1<<CS10);
    OCR1A = GUN_TIMER_TOP;  // Set TOP
    OCR1C = GUN_TIMER_HALF;  // Set TARGET for 50 % pulse width

    // State
    gun->enabled = false;
    gun->code = 0;
    gun->current_bit = 0;

    gun_send_lo();
}

void gun_trigger(gun_state * gun, uint8_t code)
{
    if (gun->enabled)
        return;

    gun->enabled = true;
    gun->code = 0;
    gun->code |= 0x01;
    gun->code |= (code << 2);
    gun->current_bit = 0;
}

void gun_transmit(void *object)
{
    gun_state *gun = (gun_state*)object;
    digitalWrite(4, HIGH);

    if (gun->enabled)
    {

        if((gun->code >> gun->current_bit) & 0x1)
            gun_send_hi();
        else
            gun_send_lo();

        ++gun->current_bit;

        if (gun->current_bit >= 10)
            gun->enabled = false;
    }

    digitalWrite(4, LOW);
}
