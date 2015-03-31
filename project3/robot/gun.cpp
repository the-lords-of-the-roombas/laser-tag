#include "gun.hpp"
#include "../rtos/os.h"
#include "../arduino_config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <Arduino.h>

// 38kHz fequency = 0.00002631578947368421 second period
// at 16Mhz and no prescaler, that is about 421 cycles.
#define GUN_TIMER_TOP 421
#define GUN_TIMER_HALF 210
#define GUN_CYCLES_PER_TICK 250

static robot_tag_game::gun * volatile g = 0;

namespace robot_tag_game {

// debug cruft:
//char *sending = "abcdef";
//int current_char_index = 0;

gun::gun()
{
    // Prevent sending anything:
    m_current_bit_index = 8;
}

void gun::init()
{
    uint8_t sreg = SREG;
    cli();

    if (g)
        OS_Abort();

    g = this;

    pinMode(arduino::pin_ir_emit, OUTPUT);

    // Clear control registers
    TCCR3A = 0;
    TCCR3B = 0;

    // Enable interrupt?
    TIMSK3 &= ~(1<<OCIE3C);

    //Set to Fast PWM (mode 15)
    TCCR3A |= (1<<WGM30) | (1<<WGM31);
    TCCR3B |= (1<<WGM32) | (1<<WGM33);

    //Enable output C.
    TCCR3A |= (1<<COM3C1);

    //No prescaler
    TCCR3B |= (1<<CS30);
    OCR3A = GUN_TIMER_TOP; // Set TOP
    OCR3C = GUN_TIMER_HALF; // Set TARGET for 50 % pulse width

//Set up bit transmission timer 
    // First, clear everything..
    TIMSK4 = 0;
    TCCR4A = 0;
    TCCR4B = 0;

    // Use 1/64 prescaler
    TCCR4B |= (_BV(CS41));
    TCCR4B |= (_BV(CS40));
    // Time out after 1 tick
    OCR4A = TCNT4 + GUN_CYCLES_PER_TICK;
    // Clear interrupt flag
    TIFR4 = _BV(OCF4A);
    // Enable interrupt
    TIMSK4 |= _BV(OCIE4A);

    sei();
    SREG = sreg;
}

bool gun::send(char byte)
{
    bool ok = false;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        // Don't proceed if still transmitting.
        if (m_current_bit_index > 7)
        {
            m_byte_to_send = byte;
            m_current_bit_index = -1;
            m_current_bit_phase = 0;

            ok = true;
        }
    }

    return ok;
}

void gun::send_hi()
{
    // Enable output C:
    TCCR3A |= (1<<COM3C1);
}

void gun::send_lo()
{
    // Disable output C:
    TCCR3A &= ~(1<<COM3C1);
}

void gun::next_tick()
{
    char byte_to_send;
    char bit_idx;
    char bit_phase;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        byte_to_send = m_byte_to_send;
        bit_idx = m_current_bit_index;
        bit_phase = m_current_bit_phase;
    }

    if (bit_idx > 7)
    {
        //digitalWrite(13, LOW);
        return;
    }

    //digitalWrite(13, HIGH);

    if (bit_idx < 0)
    {
        send_lo();
    }
    else
    {
        int value = 0;
        value = byte_to_send &(1<<(7-bit_idx));

        bool hi;
        if (value)
            hi = bit_phase<3;
        else
            hi = bit_phase<1;

        if (hi)
            send_hi();
        else
            send_lo();
    }

    bit_phase++;

    if (bit_phase > 3)
    {
        bit_phase = 0;
        bit_idx++;
    }

    m_current_bit_index = bit_idx;
    m_current_bit_phase = bit_phase;
}

void gun::run()
{

}

}

ISR(TIMER4_COMPA_vect)
{
    if(!g) return;
    OCR4A += GUN_CYCLES_PER_TICK;
    g->next_tick();
}


