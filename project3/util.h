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
T array_max(const T *data, unsigned int count, unsigned int * index)
{
    if (count == 0)
    {
        *index = 0;
        return 0;
    }
    T m = data[0];
    for(unsigned int i = 1; i < count; ++i)
    {
        if (data[i] > m)
        {
            *index = i;
            m = data[i];
        }
    }
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

template<typename T>
T abs_dif(T a, T b)
{
    if (a > b)
        return a - b;
    else
        return b - a;
}

inline bool coin_flip(uint16_t clock)
{
    return clock & 0x1;
}

inline uint8_t random_uint8(uint16_t clock)
{
    return clock & 0xFF;
}


template<typename T, unsigned int N>
class variance_filter
{
public:
    variance_filter()
    {
        for(int i = 0; i < N; ++i)
            m_prev[i] = 0;
        m_last = 0;
        m_last_good = 0;
        m_delay_idx = 0;
    }

    T operator()(T in, T & out_avg_dif, T & out_in_dif)
    {
        T sum = 0;
        for (unsigned int i = 0; i < N-1; ++i)
            sum += m_prev[i];
        sum += m_last;
        T avg = sum / N;

        T avg_dif = 0;
        {
            unsigned int i, k;
            unsigned int k_sum = 0;
            for (i = m_delay_idx, k = 1; i < N-1; ++i, ++k)
            {
                avg_dif += abs_dif(m_prev[i], avg) * k;
                k_sum += k;
            }
            for (i = 0; i < m_delay_idx; ++i, ++k)
            {
                avg_dif += abs_dif(m_prev[i], avg) * k;
                k_sum += k;
            }
            avg_dif += abs_dif(m_last, avg) * k;
            avg_dif /= k_sum;
        }

        T in_dif = abs_dif(in, avg);

        T out;
        if (in_dif <= 3 * avg_dif)
            out = m_last_good = in;
        else
            out = m_last_good;

        m_prev[m_delay_idx] = m_last;
        m_last = in;
        ++m_delay_idx;
        if (m_delay_idx >= N-1)
            m_delay_idx = 0;

        out_avg_dif = avg_dif;
        out_in_dif = in_dif;

        return out;
    }
private:
    T m_prev[N-1];
    T m_last;
    T m_last_good;
    uint8_t m_delay_idx;
};

#define memory_barrier() asm volatile ("" ::: "memory")

}

#endif // RTOS_UTIL_INCLUDED
