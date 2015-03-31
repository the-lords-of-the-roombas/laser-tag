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
        move,
    };

    enum direction_t
    {
        straight = 0,
        left,
        right
    };

    enum speed_t
    {
        // unit = mm/s
        still = 0,
        super_slow = 100,
        slow = 200,
        fast = 300,
        super_fast = 400,
        deadly_fast = 500
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
        uint16_t distance; // see "mm_to_distance_unit" below
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
        uint16_t remaining_distance;
    };

    controller(irobot *robot,
               gun *,
               input_t *input,
               output_t *output,
               Service *out_service,
               uint16_t period_ms);

    // Must run in a periodic task:
    void run();

    uint16_t mm_to_distance_units(uint16_t mm)
    {
        /*
        slowest_speed = 100 mm/sec = 100/1000 mm/ms
        mm-per-unit = period_ms * 100 / 1000
        units = mm / mm-per-unit = mm * 1000 / (period_ms * 100) = mm * 10 / period_ms
        */
        return (mm * 10ul) / m_period_ms;
    }

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
