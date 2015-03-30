#include "global.hpp"

enum local_packet_types
{
    debug_packet_type = global_packet_type_count
};

struct debug_payload
{
    int16_t test;

    int16_t ctl_behavior;
    int16_t obj_motion;
    int16_t obj_seek;
    int16_t radius;
    int16_t last_dir;

    uint16_t sonar_cm;
    //uint16_t proximity[6];
    //int16_t proximity_center;
    float proximities[3];
};
