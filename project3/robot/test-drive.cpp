#include "../world.hpp"
#include "../arduino_config.h"
#include "../util.h"
#include "../rtos/os.h"
#include "../irobot/irobot.hpp"
#include "../radio/radio.h"
#include "../radio_packets/radio_packets.hpp"
#include "Arduino.h"

using namespace robot_tag_game;

enum navigation_state
{
    nav_go,
    nav_back_away,
    nav_turn_around,
};

irobot robot(&Serial1, arduino::pin_baud_rate_change);
Service * volatile report_service;
uint8_t volatile g_distance_bytes[2];
int16_t volatile g_distance_delta;
int16_t volatile g_distance;

int16_t volatile g_angle_delta;
int16_t volatile g_angle;


void drive(irobot & robot, int16_t velocity, int16_t radius)
{
    uint8_t data[4];
    data[0] = (velocity >> 8) & 0xFF;
    data[1] = velocity & 0xFF;
    data[2] = (radius >> 8) & 0xFF;
    data[3] = radius & 0xFF;
    robot.send(irobot::op_drive, data, 4);
}

void drive_straight(irobot & robot, int16_t velocity)
{
    uint8_t data[4];
    data[0] = (velocity >> 8) & 0xFF;
    data[1] = velocity & 0xFF;
    data[2] = 0x80;
    data[3] = 0x00;
    robot.send(irobot::op_drive, data, 4);
}

void navigate()
{
    int16_t travelled_m = 0;
    int16_t travelled_mm = 0;
    int16_t turned_deg = 0;

    navigation_state state = nav_go;

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

        {
            static const int sensor_count = 3;
            uint8_t data[sensor_count+1];
            data[0] = sensor_count;
            data[1] = irobot::sense_bumps_and_wheel_drops;
            data[2] = irobot::sense_distance;
            data[3] = irobot::sense_angle;

            robot.send(irobot::op_sensor_list, data, sensor_count + 1);
        }

        uint8_t bumps_and_wheel_drops;
        uint8_t distance_data[2];
        uint8_t angle_data[2];
        robot.receive(&bumps_and_wheel_drops, 1);
        robot.receive(distance_data, 2);
        robot.receive(angle_data, 2);

        bool wheel_drop = bumps_and_wheel_drops & (_BV(3) | _BV(2));
        bool bump = bumps_and_wheel_drops & (_BV(1) | _BV(0));

        if (wheel_drop)
        {
            robot.stop();
            OS_Abort();
        }

        //255 251 -5

        // Update travelled distance
        {
            int16_t distance_delta_mm = bytes_to_int16(distance_data[0], distance_data[1]);
            travelled_mm += distance_delta_mm;

            g_distance_bytes[0] = distance_data[0];
            g_distance_bytes[1] = distance_data[1];
            g_distance_delta = distance_delta_mm;
            g_distance = travelled_mm;
            Service_Publish(report_service, 0);
#if 0
            if (travelled_mm >= 1000)
            {
                travelled_m += travelled_mm / 1000;
                travelled_mm = travelled_mm % 1000;
            }
#endif
        }
        // Update turned angle
        {
            int16_t turned_delta = bytes_to_int16(angle_data[0], angle_data[1]);
            turned_deg += turned_delta;

            g_angle_delta = turned_delta;
            g_angle = turned_deg;
        }

        navigation_state new_state = state;

        // Possibly change state
        switch(state)
        {
        case nav_go:
        {
            if (bump)
            {
                new_state = nav_back_away;
            }
            //else if (travelled_m >= 1)
            else if (travelled_mm <= -100)
            {
                new_state = nav_turn_around;
            }
            break;
        }
        case nav_back_away:
        {
            //if (travelled_m < 0 || travelled_mm <= -100)
            if (travelled_mm > 10);
            {
                new_state = nav_turn_around;
            }
            break;
        }
        case nav_turn_around:
        {
            if (turned_deg >= 175)
            {
                new_state = nav_go;
            }
            break;
        }
        default:
            break;
        }

        if (new_state != state)
        {
            state = new_state;
            travelled_m = travelled_mm = 0;
            turned_deg = 0;
        }

        // Actuate state
        switch(state)
        {
        case nav_go:
        {
            drive_straight(robot, 100);
            break;
        }
        case nav_back_away:
        {
            drive_straight(robot, -100);
            break;
        }
        case nav_turn_around:
        {
            drive(robot, 100, 1);
            break;
        }
        default:
            OS_Abort();
        }

        Task_Next();
    }
}

void report()
{
    ServiceSubscription *report = Service_Subscribe(report_service);
    for(;;)
    {
        Service_Receive(report);
        //Serial.print(g_distance_bytes[0]);
        //Serial.write(' ');
        //Serial.print(g_distance_bytes[1]);
        //Serial.write(' ');
        Serial.print("D: ");
        Serial.print(g_distance_delta);
        Serial.write(' ');
        Serial.print(g_distance);
        Serial.println();

        Serial.print("A: ");
        Serial.print(g_angle_delta);
        Serial.write(' ');
        Serial.print(g_angle);
        Serial.println();
    }
}

int r_main()
{

    pinMode(13, OUTPUT);

    //Serial.begin(9600);

    report_service = Service_Init();

    robot.begin();

    robot.send(irobot::op_full_mode );

    {
        // Confirm mode
        uint8_t mode;
        robot.send(irobot::op_sensor, irobot::sense_oi_mode);
        robot.receive(&mode, 1);
        if (mode != 3)
            OS_Abort();
    }

    for(int i = 0; i < 3; ++i)
    {
        digitalWrite(13, HIGH);
        delay(300);
        digitalWrite(13, LOW);
        delay(300);
    }

    Task_Create_RR(report, 0);

    Task_Create_Periodic(navigate, 0, 50, 5, 0);

    Task_Periodic_Start();

    return 0;
}
