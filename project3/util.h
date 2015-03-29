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
T sum(const T *data, unsigned int count)
{
    T sum = 0;
    for(unsigned int i = 0; i < count; ++i)
        sum += data[i];
    return sum;
}

template <typename T> inline
T array_max(const T *data, unsigned int count)
{
    if (count == 0)
        return 0;
    T m = data[0];
    for(unsigned int i = 1; i < count; ++i)
        if (data[i] > m)
            m = data[i];
    return m;
}

// Returns centroid of data, in range 0 - 100
// Undefined on negative data!
template <typename T> inline
int centroid(const T *data, unsigned int count, T sum)
{
    if (sum == 0)
        return 50;

    int weighted_idx = 0;
    for(unsigned int i = 0; i < count; ++i)
    {
        weighted_idx += i * data[i];
    }
    return (weighted_idx * 100) / (sum * (count - 1));
}

#define memory_barrier() asm volatile ("" ::: "memory")

}

#endif // RTOS_UTIL_INCLUDED
