#ifndef ROBOT_TAG_GAME_CONTROLLER_INCLUDED
#define ROBOT_TAG_GAME_CONTROLLER_INCLUDED

#include "gun.hpp"
#include "../irobot/irobot.hpp"
#include "../rtos/os.h"
#include <stdint.h>

namespace robot_tag_game {

/*
  Any task that shares input_t and output_t instances must be
  of lower priority!
*/

class controller
{
public:
    enum behavior_t
    {
        wait,
        go,
        chase,
        shoot,
        drive_forward,
        face_obstacle
    };

    enum direction_t
    {
        straight = 0,
        left,
        right
    };

    enum speed_t
    {
        super_fast,
        fast,
        slow,
        super_slow
    };

    struct input_t
    {
        input_t():
            behavior(wait),
            direction(straight),
            sonar_cm(0),
            sonar_cm_seek_threshold(0)
        {}

        controller::behavior_t behavior;
        direction_t direction;
        speed_t speed;
        uint16_t sonar_cm;
        uint16_t sonar_cm_seek_threshold;
    };

    struct output_t
    {
        output_t():
            bump_left(false),
            bump_right(false),
            object_left(false),
            object_right(false),
            object_centered(false)
        {}

        //int behavior;
        //uint16_t sonar_cm;
        //int obj_motion_trail;
        //int obj_seek_trail;
        //int radius;
        //int last_direction;
        bool bump_left;
        bool bump_right;
        bool object_left;
        bool object_right;
        bool object_centered;
        bool done_shooting;
    };

    controller(irobot *robot,
               gun *,
               input_t *input,
               output_t *output,
               Service *out_service,
               uint16_t period_ms);

    // Must run in a periodic task:
    void run();

private:
    struct sensor_data
    {
        bool bump_left;
        bool bump_right;
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

    int periods_from_ms(int ms) { return ms / m_period_ms; }

    /*int16 mm_per_period( int16_t speed ) {
        return speed
        100 mm / 1000 ms;

        // (100 / 1000) mm/ms == (100 * period_ms) / 1000

    }*/

    irobot *m_robot;
    gun *m_gun;
    input_t *m_input_src;
    output_t *m_output_dst;
    Service *m_output_service;
    uint16_t m_period_ms;

    sensor_data m_sensors;
};

}

#endif // ROBOT_TAG_GAME_CONTROLLER_INCLUDED
