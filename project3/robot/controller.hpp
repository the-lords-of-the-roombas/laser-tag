#ifndef ROBOT_TAG_GAME_CONTROLLER_INCLUDED
#define ROBOT_TAG_GAME_CONTROLLER_INCLUDED

#include "../irobot/irobot.hpp"
#include "../rtos/os.h"
#include <stdint.h>

namespace robot_tag_game {

struct control_behavior
{
    enum types
    {
        wait,
        drive_forward,
        turn_right,
        turn_left,
        face_obstacle
    };

    types type;
    uint16_t until;
};

struct sensor_data
{
    bool bump;
    bool wheel_drop;
    uint16_t proximity[6];
};

struct control_info
{
};

class controller
{
public:
    controller(irobot *robot,
               control_behavior *behavior,
               control_info *info_dst,
               Service *info_service);
    // Must run in a periodic task:
    void run();
private:
    void acquire_sensors(sensor_data & d);
    void drive(int16_t velocity, int16_t radius);
    void drive_straight(int16_t velocity);
    void drive_stop();

    irobot *m_robot;
    control_behavior *m_behavior_source;
    control_info *m_info_dst;
    Service *m_info_service;

    sensor_data m_sensors;
};

}

#endif // ROBOT_TAG_GAME_CONTROLLER_INCLUDED
