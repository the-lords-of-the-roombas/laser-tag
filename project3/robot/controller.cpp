#include "controller.hpp"
#include "../util.h"
#include <util/atomic.h>
#include <Arduino.h>

namespace robot_tag_game {

controller::controller
(irobot *robot, gun *g, input_t *input, output_t *output,
 Service *output_service, uint16_t period_ms):
    m_robot(robot),
    m_gun(g),
    m_input_src(input),
    m_output_dst(output),
    m_output_service(output_service),
    m_period_ms(period_ms)
{}

static int trail_filter(int v)
{
    if (v > 0)
        return v - 1;
    else if (v < 0)
        return v + 1;
    else
        return v;
}

void controller::run()
{
    int back_up_time = 0;
    int ban_travel_time = 0;

    int last_direction = 0;
    int object_motion_trail = 0;
    int object_seek_trail = 0;
    bool object_last_seen = false;
    bool was_shooting = false;
    int shooting_phase = 0;

    input_t & input = *m_input_src;
    output_t & output = *m_output_dst;

    static const int object_seek_len = 2000 / m_period_ms;

    for(;;)
    {
        // Get input

        // Sync shared memory (input, output):
        memory_barrier();

#if 0
        {
            static bool on = true;
            if (on)
                digitalWrite(13, HIGH);
            else
                digitalWrite(13, LOW);
            on = !on;
        }
#endif
        // Read sensors

        acquire_sensors(m_sensors);

        if (m_sensors.wheel_drop)
        {
            m_robot->stop();
            OS_Abort();
        }

        // Compute environment

        bool bumped = m_sensors.bump_left || m_sensors.bump_right;

        back_up_time = bumped
                ? periods_from_ms(250)
                : trail_filter(back_up_time);

        if (bumped)
            ban_travel_time =
                    (m_sensors.bump_left && !m_sensors.bump_right)
                    ? (-periods_from_ms(1500)) : periods_from_ms(1500);
        else
            ban_travel_time = trail_filter(ban_travel_time);

        bool object_seen = input.sonar_cm <= input.sonar_cm_seek_threshold;
        bool object_disappeared = !object_seen && object_last_seen;
        object_last_seen = object_seen;

        if (object_seen && last_direction != 0)
            object_motion_trail = last_direction * 150;
        else
            object_motion_trail = trail_filter(object_motion_trail);

        if (object_disappeared && object_motion_trail != 0)
        {
            if (object_motion_trail > 0)
                object_seek_trail = object_seek_len;
            else
                object_seek_trail = -object_seek_len;
        }
        else
        {
            object_seek_trail = trail_filter(object_seek_trail);
        }
#if 0
        int expected_object_direction = 0;
        if (object_seek_trail > 3 * object_seek_len / 4 )
            expected_object_direction = 1;
        else if (object_seek_trail > 0)
            expected_object_direction = -1;
        else if (object_seek_trail < -3 * object_seek_len / 4)
            expected_object_direction = -1;
        else if (object_seek_trail < 0)
            expected_object_direction = 1;
#endif
        uint16_t prox_max_idx = 0;
        uint16_t prox_max = array_max(m_sensors.proximity, 6, &prox_max_idx);
        //int16_t prox_weights[] = { -60, -26, -8, 26, 50, 102 };

        bool object_centered = false;

        if (!was_shooting && input.behavior == shoot)
            shooting_phase = 5;
        else
            shooting_phase = trail_filter(shooting_phase);

        // Compute controls

        int16_t velocity = 0;
        int16_t radius = 0;

        switch(input.behavior)
        {
        case wait:
        {
            break;
        }
        case go:
        {
            if (input.direction != straight)
            {
                velocity = input.speed;
                radius = input.direction == left ? 1 : -1;
            }
            else if (prox_max < 50)
            {
                velocity = input.speed;
            }
            else
            {
                // We are not turning,
                // and we are in close proximity of an object.
                // Hence, stop.
                velocity = 0;
            }
#if 0
            if (object_seen)
            {
                // stop
            }
            else if (expected_object_direction)
            {
                velocity = 200;
                radius = expected_object_direction > 0 ? 1 : -1;
            }
            else
            {
                velocity = 100;
                radius = 1;
            }
#endif
            break;
        }
        case chase:
        {
            if (prox_max > 50)
            {
                // We are in close proximity of object
                if (prox_max_idx == 2)
                {
                    object_centered = true;
                    // We have aimed right into the object
                }
                else
                {
                    // Turn towards the object
                    velocity = 50;
                    radius =  prox_max_idx < 2 ? 1 : -1;
                }
            }
            else
            {
                velocity = input.speed;
            }
            break;
        }
        case shoot:
        {
            if (shooting_phase == 5)
            {
                //digitalWrite(13, HIGH);
                m_gun->send('x');
            }
            else
            {
                //digitalWrite(13, LOW);
            }
            break;
        }
        case move:
        {
            if (input.distance)
            {
                velocity = input.speed;

                switch(input.direction)
                {
                case straight:
                    radius = 0; break;
                case left:
                    radius = 1; break;
                case right:
                    radius = -1; break;
                }
            }

            input.distance = trail_filter(input.distance);

            break;
        }
        default:
            m_robot->stop();
            OS_Abort();
        }

        // Override

        if(back_up_time)
        {
            radius = 0;
            velocity = -100;
        }
        else if (ban_travel_time || prox_max > 50)
        {
            if (radius != 1 && radius != -1)
                velocity = 0;
        }

#if 0
        // Kill movement for debugging purposes:
        velocity = 0;
        radius = 0;
#endif
        // Apply

        if (radius == 0)
            drive_straight(velocity);
        else
            drive(velocity, radius);

        // Output
/*
        output.sonar_cm = input.sonar_cm;
        output.behavior = input.behavior;
        output.obj_motion_trail = object_motion_trail;
        output.obj_seek_trail = object_seek_trail;
        output.radius = radius;
        output.last_direction = last_direction;
*/
        output.bump_left = ban_travel_time < 0;
        output.bump_right = ban_travel_time > 0;
        output.object_left =
                m_sensors.proximity[0] > 50 ||
                m_sensors.proximity[1] > 50 ||
                m_sensors.proximity[2] > 50;
        output.object_right =
                m_sensors.proximity[3] > 50 ||
                m_sensors.proximity[4] > 50 ||
                m_sensors.proximity[5] > 50;
        output.object_centered = object_centered;
        output.done_shooting = input.behavior == shoot && shooting_phase == 0;
        output.remaining_distance = input.distance;

        // Update past

        if (radius > 0)
            last_direction = 1;
        else if (radius < 0)
            last_direction = -1;
        else
            last_direction = 0;

        was_shooting = input.behavior == shoot;

        // Provide output

        // Sync shared memory (input, output):
        memory_barrier();

        Service_Publish(m_output_service, 0);

        Task_Next();
    }
}



void controller::acquire_sensors(sensor_data & d)
{
    {
        static const int sensor_count = 7;
        uint8_t data[] = {
            sensor_count,
            irobot::sense_bumps_and_wheel_drops,
            irobot::sense_light_bump_left_signal,
            irobot::sense_light_bump_front_left_signal,
            irobot::sense_light_bump_center_left_signal,
            irobot::sense_light_bump_center_right_signal,
            irobot::sense_light_bump_front_right_signal,
            irobot::sense_light_bump_right_signal,
        };
#if 0
        data[0] = sensor_count;
        data[1] = irobot::sense_bumps_and_wheel_drops;
        data[2] = irobot::sense_light_bump_left_signal;
        data[3] = irobot::sense_light_bump_front_left_signal;
        data[4] = irobot::sense_light_bump_center_left_signal;
        data[5] = irobot::sense_light_bump_center_right_signal;
        data[6] = irobot::sense_light_bump_front_right_signal;
        data[7] = irobot::sense_light_bump_right_signal;
#endif
        m_robot->send(irobot::op_sensor_list, data, sensor_count + 1);
    }

    uint8_t raw_bumps_and_wheel_drops;
    //uint8_t raw_distance_data[2];
    //uint8_t raw_angle_data[2];
    uint8_t raw_proxim_l[2];
    uint8_t raw_proxim_lf[2];
    uint8_t raw_proxim_lc[2];
    uint8_t raw_proxim_rc[2];
    uint8_t raw_proxim_rf[2];
    uint8_t raw_proxim_r[2];
    m_robot->receive(&raw_bumps_and_wheel_drops, 1);
    //robot.receive(raw_distance_data, 2);
    //robot.receive(raw_angle_data, 2);
    m_robot->receive(raw_proxim_l, 2);
    m_robot->receive(raw_proxim_lf, 2);
    m_robot->receive(raw_proxim_lc, 2);
    m_robot->receive(raw_proxim_rc, 2);
    m_robot->receive(raw_proxim_rf, 2);
    m_robot->receive(raw_proxim_r, 2);

    d.wheel_drop = raw_bumps_and_wheel_drops & (_BV(3) | _BV(2));
    d.bump_left = raw_bumps_and_wheel_drops & _BV(1);
    d.bump_right = raw_bumps_and_wheel_drops & _BV(0);

    d.proximity[0] = bytes_to_int16(raw_proxim_l);
    d.proximity[1] = bytes_to_int16(raw_proxim_lf);
    d.proximity[2] = bytes_to_int16(raw_proxim_lc);
    d.proximity[3] = bytes_to_int16(raw_proxim_rc);
    d.proximity[4] = bytes_to_int16(raw_proxim_rf);
    d.proximity[5] = bytes_to_int16(raw_proxim_r);
}

#if 0

float proximity_to_target( int16_t target,
                           int16_t *positions,
                           uint16_t *proximities,
                           unsigned int count)
{
    float result = 0;
    for(unsigned int i = 0; i < count; ++i)
    {
        int16_t position = positions[i];
        int16_t proximity = proximities[i];
        proximity = max(proximity - 10, 0);

        float f_proximity = proximity / 1000.f;

        int16_t distance = target - position;
        if (distance < 0)
            distance = -distance;
        if (distance > 100)
            distance = 100;
        int16_t closeness = 100 - distance;

        result += f_proximity * closeness;
    }
    return result;
}
#endif


void controller::drive(int16_t velocity, int16_t radius)
{
    uint8_t data[4];
    data[0] = (velocity >> 8) & 0xFF;
    data[1] = velocity & 0xFF;
    data[2] = (radius >> 8) & 0xFF;
    data[3] = radius & 0xFF;
    m_robot->send(irobot::op_drive, data, 4);
}

void controller::drive_straight(int16_t velocity)
{
    uint8_t data[4];
    data[0] = (velocity >> 8) & 0xFF;
    data[1] = velocity & 0xFF;
    data[2] = 0x80;
    data[3] = 0x00;
    m_robot->send(irobot::op_drive, data, 4);
}

void controller::drive_stop()
{
    drive_straight(0);
}

void controller::turn(int16_t velocity, turn_direction dir )
{
    drive(velocity, dir == clockwise ? 1 : -1);
}

}
