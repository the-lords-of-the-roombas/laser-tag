#include "global.hpp"

enum local_packet_types
{
    debug_packet_type = global_packet_type_count
};

struct debug_payload
{
    int16_t test;
    uint16_t proximity[6];
    int16_t proximity_center;
};
