#include "sonar.hpp"
#include "../arduino_config.h"
#include "../rtos/os.h"

#include <avr/io.h>
#include <util/delay.h>
#include <Arduino.h>

namespace robot_tag_game {

Service * volatile g_echo_service = 0;
uint16_t volatile g_time = 0;

enum edge
{
    falling_edge = 0,
    rising_edge = 1
};

static int capture_edge()
{
    if(TCCR5B & _BV(ICES5))
        return rising_edge;
    else
        return falling_edge;
}

static void set_capture_edge( edge e)
{
    if (e)
        TCCR5B |= _BV(ICES5);
    else
        TCCR5B &= ~_BV(ICES5);
}

sonar::sonar()
{

}

int sonar::pin()
{
    return arduino::pin_sonar_io;
}

void sonar::init(Service *request_service, Service *reply_service)
{
    if (g_echo_service || m_request_service)
        OS_Abort();

    m_request_service = request_service;
    m_reply_service = reply_service;
    g_echo_service = Service_Init();

    if (!m_request_service || !m_reply_service || !g_echo_service)
        OS_Abort();

    m_request_sub = Service_Subscribe(m_request_service);
    m_echo_sub = Service_Subscribe(g_echo_service);

    TCCR5A = 0;
    TCCR5B = 0;
    TIMSK5 = 0;
}

void sonar::work()
{
    for(;;)
    {
        Service_Receive(m_request_sub);

        speak();

        int echo_duration = listen();

        Service_Publish(m_reply_service, echo_duration);
    }
}

void sonar::speak()
{
    // Disable input capture interrupt
    TIMSK5 = 0;
    pinMode(arduino::pin_sonar_io, OUTPUT);
    digitalWrite(arduino::pin_sonar_io, HIGH);
    delayMicroseconds(10);
    digitalWrite(arduino::pin_sonar_io, LOW);
}

int sonar::listen()
{
    g_time = 0;
    pinMode(arduino::pin_sonar_io, INPUT);
    digitalWrite(arduino::pin_sonar_io, LOW);

    // Use 1/256 prescaler. Provides maximum measure duration ~ 1sec.
    TCCR5B |= _BV(CS52);
    // Select rising edge to trigger input capture
    set_capture_edge(rising_edge);
    // Reset counter
    TCNT5 = 0;
    // Clear all interrupt flags
    TIFR5 = 0xFF;
    // Enable input capture interrupt
    TIMSK5 |= _BV(ICIE5);

    return Service_Receive(m_echo_sub);
}

#if 1
ISR(TIMER5_CAPT_vect)
{

    uint16_t current_time = ICR5;

    switch(capture_edge())
    {
    case rising_edge:
    {
        //digitalWrite(13, HIGH);

        set_capture_edge(falling_edge);

        g_time = current_time;

        break;
    }
    case falling_edge:
    {
        //digitalWrite(13, LOW);

        // Disable input capture interrupt
        TIMSK5 = 0;

        uint16_t duration = current_time - g_time;
        Service_Publish(g_echo_service, duration);

        break;
    }
    }
}
#endif
}
