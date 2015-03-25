#include "gun.hpp"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

// 38kHz fequency = 0.00002631578947368421 second period
// at 16Mhz and no prescaler, that is about 421 cycles.
#define GUN_TIMER_TOP 421
#define GUN_TIMER_HALF 210
#define GUN_CYCLES_PER_BIT 250

static robot_tag_game::gun *g = 0;

namespace robot_tag_game {


char *sending = "abcdef";
int current_char_index = 0;

gun::gun()
{
    current_bit_index = 0;
    current_bit_phase = 0;
    byte_to_send = 'z';
}

void gun::init()
{
    g = this;

    pinMode(3, OUTPUT);    

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
    OCR4A = TCNT4 + GUN_CYCLES_PER_BIT;
    // Clear interrupt flag
    TIFR4 = _BV(OCF4A);
    // Enable interrupt
    TIMSK4 |= _BV(OCIE4A);
}

void gun::send_hi()
{
    // Enable output C:
    TCCR3A |= (1<<COM3C1);
    digitalWrite(10, HIGH);
}

void gun::send_lo()
{
    // Disable output C:
    TCCR3A &= ~(1<<COM3C1);
    digitalWrite(10, LOW);
}

void gun::next_tick()
{
    byte_to_send = sending[current_char_index];

    if(current_bit_index < 0){
        if(current_bit_index < -400)
            digitalWrite(13, HIGH);
        else
            digitalWrite(13, LOW);
        current_bit_index++;
        return;
    }

    int value = 0;
    int hi = 0;
    value = byte_to_send &(1<<(7-current_bit_index));
    
    if(value)
    {
        hi=current_bit_phase<3;
    }else{
        hi=current_bit_phase<1; 
    }
    
    current_bit_phase++;
    
    if(current_bit_phase > 3)
    {
        current_bit_phase = 0;
        current_bit_index++;
    }

    if(current_bit_index > 7)
    {
        current_bit_index = -500;
        current_char_index = (current_char_index+1) % 6;
    }

    if(hi)
        send_hi();
    else
        send_lo();
}

void gun::run()
{

}

}

ISR(TIMER4_COMPA_vect)
{
    if(!g) return;
    OCR4A += GUN_CYCLES_PER_BIT;
    g->next_tick();
}


