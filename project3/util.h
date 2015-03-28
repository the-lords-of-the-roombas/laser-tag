#ifndef RTOS_UTIL_INCLUDED
#define RTOS_UTIL_INCLUDED

#include <stdint.h>

#define SET_BIT(FIELD, BIT) (FIELD |= _BV(BIT))
#define CLEAR_BIT(FIELD, BIT) (FIELD &= ~_BV(BIT))

namespace robot_tag_game {

inline int16_t bytes_to_int16(uint8_t h, uint8_t l)
{
    uint16_t result = 0;
    result |= h & 0xFF;
    result <<= 8;
    result |= l & 0xFF;
    return result;
}

}

#endif // RTOS_UTIL_INCLUDED
