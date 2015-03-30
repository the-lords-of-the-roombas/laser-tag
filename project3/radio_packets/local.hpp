#include "global.hpp"

enum local_packet_types
{
    debug_packet_type = global_packet_type_count
};

struct debug_payload
{
    int16_t test;
#if 0
    int16_t ctl_behavior;
    int16_t obj_motion;
    int16_t obj_seek;
    int16_t radius;
    int16_t last_dir;
#endif

#if 0
    uint16_t sonar_cm;
    float proximities[3];
#endif

    bool bump_left;
    bool bump_right;
    bool object_left;
    bool object_right;
};
