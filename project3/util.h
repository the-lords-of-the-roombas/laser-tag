#ifndef RTOS_UTIL_INCLUDED
#define RTOS_UTIL_INCLUDED

#include <stdint.h>

#define SET_BIT(FIELD, BIT) (FIELD |= _BV(BIT))
#define CLEAR_BIT(FIELD, BIT) (FIELD &= ~_BV(BIT))

namespace robot_tag_game {

inline uint16_t bytes_to_int16(uint8_t h, uint8_t l)
{
    uint16_t result = 0;
    result |= h & 0xFF;
    result <<= 8;
    result |= l & 0xFF;
    return result;
}

inline uint16_t bytes_to_int16(uint8_t *bytes)
{
    return bytes_to_int16(bytes[0], bytes[1]);
}

template <typename T> inline
T sum(T *data, unsigned int count)
{
    T sum = 0;
    for(unsigned int i = 0; i < count; ++i)
        sum += data[i];
    return sum;
}

#define memory_barrier() asm volatile ("" ::: "memory")

}

#endif // RTOS_UTIL_INCLUDED
