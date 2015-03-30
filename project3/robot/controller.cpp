#include "controller.hpp"
#include "../util.h"
#include <util/atomic.h>
#include <Arduino.h>

namespace robot_tag_game {

controller::controller
(irobot *robot, input_t *input, output_t *output,
 Service *output_service):
    m_robot(robot),
    m_input_src(input),
    m_output_dst(output),
    m_output_service(output_service)
{}

void controller::run()
{
    int past_back_up = 0;
    //float past_prox_center = 0.0;
    int ghost_prox_left = 0;
    int past_target_found = 0;

    input_t input;
    output_t output;
    output.sonar_cm = 0;

    for(;;)
    {
        {
            static bool on = true;
            if (on)
                digitalWrite(13, HIGH);
            else
                digitalWrite(13, LOW);
            on = !on;
        }

        acquire_sensors(m_sensors);

        if (m_sensors.wheel_drop)
        {
            m_robot->stop();
            OS_Abort();
        }

        // Update past

        past_back_up = max(past_back_up - 1, 0);
        past_target_found = max(past_target_found - 1, 0);
        ghost_prox_left = max(ghost_prox_left - 1, 0);

        // Compute

        if(m_sensors.bump)
        {
            past_back_up = 10;
            ghost_prox_left = 40;
        }

        uint16_t prox_max_idx = 0;
        uint16_t prox_max = array_max(m_sensors.proximity, 6, &prox_max_idx);
        uint16_t prox_sum = sum(m_sensors.proximity, 6);
        //uint16_t prox_sum_left = sum(m_sensors.proximity, 3);
        //uint16_t prox_sum_right = sum(m_sensors.proximity + 3, 3);

        int16_t prox_weights[] = { -60, -26, -8, 26, 50, 102 };

        //float prox_left = proximity_to_target(-100, prox_weights, m_sensors.proximity, 6);
        //float prox_right = proximity_to_target(100, prox_weights, m_sensors.proximity, 6);
        //float prox_center = proximity_to_target(0, prox_weights, m_sensors.proximity, 6);
        //prox_left += ghost_prox_left;

        // Exchange data

        //g_sensors_derived.proximity_center = prox_center;
        //g_sensors_derived.proximities[0] = prox_left;
        //g_sensors_derived.proximities[1] = prox_center;
        //g_sensors_derived.proximities[2] = prox_right;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            input = *m_input_src;
            *m_output_dst = output;
        }

        output.sonar_cm = input.sonar_cm;
        output.behavior = input.behavior;

        Service_Publish(m_output_service, 0);


        // Control

        if (past_back_up)
        {
            drive_straight(-100);
        }
        else
        {
            switch(input.behavior)
            {
            case wait:
            {
                drive_stop();
                break;
            }
            case seek:
            {
                if (input.sonar_cm > input.sonar_cm_seek_threshold)
                {
                    turn(100, clockwise);
                }
                else
                {
                    drive_stop();
                }
                break;
            }
            case approach:
            {
                if (prox_max > 50)
                    drive_stop();
                else
                    drive_straight(300);
                break;
            }
            case drive_forward:
            {
                drive_stop();

                if (ghost_prox_left)
                {
                    drive(100, -1);
                }
#if 0
                int turn_direction = prox_left > prox_right ? -1 : 1;

                if (prox_left > 10 || prox_right > 10 || prox_center > 10)
                {
                    // Rotate away from obstacle
                    int velocity = 100;
                    drive(g_robot, velocity, turn_direction);
                }
                else if (prox_left > 0 || prox_right > 0 || prox_center > 0)
                {
                    // Veer away from obstacle
                    int velocity = 100;
                    int radius = turn_direction * 100;
                    drive(g_robot, velocity, radius);
                }
                else
                {
                    drive_straight(g_robot, 300);
                }
#endif
                break;
            }
            case face_obstacle:
            {
                if (prox_max < 15)
                {
                    drive_stop();
                }
                else
                {
                    if (prox_max_idx == 2)
                    {
                        drive_stop();
                    }
                    else
                    {
                        int turn_direction = prox_max_idx < 2 ? 1 : -1;
                        drive(100, turn_direction);
                    }
#if 0
                    int turn_direction = prox_left > prox_right ? 1 : -1;

                    if (past_target_found)
                    {
                        drive_stop(g_robot);
                    }
                    else if ( prox_center < prox_left || prox_center < prox_right )
                    {
                        drive(g_robot, 100, turn_direction);
                    }
                    else
                    {
                        // maximize
                        if (prox_center > past_prox_center)
                        {
                            drive(g_robot, 100, turn_direction);
                        }
                        else
                        {
                            past_target_found = 100;
                            drive_stop(g_robot);
                        }
                    }
#endif
                }
                break;
            }
            default:
                m_robot->stop();
                OS_Abort();
            }

            // Update past

            //past_prox_center = prox_center;
        }

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
    d.bump = raw_bumps_and_wheel_drops & (_BV(1) | _BV(0));
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
