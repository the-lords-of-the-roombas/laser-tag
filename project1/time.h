#ifndef UVIC_RTSYS_TIME_INCLUDED
#define UVIC_RTSYS_TIME_INCLUDED

#include <stdint.h>

#ifdef AVR

typedef uint32_t milliseconds_t;
typedef unsigned long microseconds_t;

#include "Arduino.h"

inline milliseconds_t current_time_ms()
{
    return millis();
}

inline microseconds_t current_time_micros()
{
    return micros();
}


#else // not AVR

#include <chrono>

typedef uint64_t milliseconds_t;
typedef uint64_t microseconds_t;

inline milliseconds_t current_time_ms()
{
    using namespace std::chrono;
    using ms_duration = duration<milliseconds_t,std::milli>;

    auto now = high_resolution_clock::now();
    auto ms = duration_cast<ms_duration>( now.time_since_epoch() );
    return ms.count();
}

#endif // system type

#endif // UVIC_RTSYS_TIME_INCLUDED
