#include "global.hpp"

enum local_packet_types
{
    debug_packet_type = global_packet_type_count
};

struct debug_payload
{
    int16_t test;
    int16_t ctl_behavior;
    uint16_t sonar_cm;
    //uint16_t proximity[6];
    //int16_t proximity_center;
    float proximities[3];
};
