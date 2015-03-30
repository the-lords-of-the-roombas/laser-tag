#ifndef ROBOT_TAG_GAME_CONTROLLER_INCLUDED
#define ROBOT_TAG_GAME_CONTROLLER_INCLUDED

#include "../irobot/irobot.hpp"
#include "../rtos/os.h"
#include <stdint.h>

namespace robot_tag_game {

class controller
{
public:
    enum behavior_t
    {
        wait,
        seek,
        approach,
        drive_forward,
        face_obstacle
    };

    struct input_t
    {
        controller::behavior_t behavior;
        uint16_t sonar_cm;
        uint16_t sonar_cm_seek_threshold;
    };

    struct output_t
    {
        int behavior;
        uint16_t sonar_cm;
    };

    controller(irobot *robot,
               input_t *input,
               output_t *output,
               Service *out_service);

    // Must run in a periodic task:
    void run();

private:
    struct sensor_data
    {
        bool bump;
        bool wheel_drop;
        uint16_t proximity[6];
    };

    enum turn_direction
    {
        clockwise,
        counter_clockwise
    };

    void acquire_sensors(sensor_data & d);
    void drive(int16_t velocity, int16_t radius);
    void drive_straight(int16_t velocity);
    void drive_stop();
    void turn(int16_t velocity, turn_direction );

    irobot *m_robot;
    input_t *m_input_src;
    output_t *m_output_dst;
    Service *m_output_service;

    sensor_data m_sensors;
};

}

#endif // ROBOT_TAG_GAME_CONTROLLER_INCLUDED
