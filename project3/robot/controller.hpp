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
        move,
    };

    enum direction_t
    {
        forward,
        backward,
        leftward,
        rightward
    };

    enum speed_t
    {
        // unit = dm/s
        still = 0,
        super_slow = 1,
        slow = 2,
        fast = 3,
        super_fast = 4,
        deadly_fast = 5
    };

    struct input_t
    {
        input_t():
            behavior(wait),
            direction(forward),
            radius(0)
        {}

        controller::behavior_t behavior;
        direction_t direction;
        speed_t speed;
        uint16_t radius;
        uint16_t distance; // see "mm_to_distance_unit" below
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
               input_t *input,
               output_t *output,
               Service *shot_service,
               uint16_t period_ms);

    // Must run in a periodic task:
    void run();

    uint16_t mm_to_distance(uint16_t mm)
    {
        return mm_to_distance(mm, m_period_ms);
    }

    static uint16_t mm_to_distance(uint16_t mm, uint16_t ms_per_period)
    {
        /*
        slowest_speed = 100 mm/sec = 100/1000 mm/ms
        mm-per-unit = ms_per_period * 100 / 1000
        units = mm / mm-per-unit = mm * 1000 / (period_ms * 100) = mm * 10 / ms_per_period
        */
        return (mm * 10ul) / ms_per_period;
    }

private:
    struct sensor_data
    {
        bool bump_left;
        bool bump_right;
        bool wheel_drop;
        uint16_t proximity[6];
        uint8_t ir;
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
    input_t *m_input_src;
    output_t *m_output_dst;
    Service *m_shot_service;
    uint16_t m_period_ms;

    sensor_data m_sensors;
};

}

#endif // ROBOT_TAG_GAME_CONTROLLER_INCLUDED
