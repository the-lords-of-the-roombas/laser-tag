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

// Radio

static uint8_t radio_base_address[5] = BASE_RADIO_ADDRESS;
static uint8_t radio_robot0_address[5] = ROBOT_0_RADIO_ADDRESS;

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
}
}

// Control

irobot g_robot(&Serial1, arduino::pin_baud_rate_change);
Service * volatile g_report_service;
uint8_t volatile g_distance_bytes[2];
int16_t volatile g_distance_delta;
int16_t volatile g_distance;

int16_t volatile g_angle_delta;
int16_t volatile g_angle;

struct sensor_data
{
    bool bump;
    bool wheel_drop;
    uint16_t proximity[6];
};

static sensor_data g_sensors;

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

void acquire_sensors(sensor_data & d)
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
        g_robot.send(irobot::op_sensor_list, data, sensor_count + 1);
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
    g_robot.receive(&raw_bumps_and_wheel_drops, 1);
    //robot.receive(raw_distance_data, 2);
    //robot.receive(raw_angle_data, 2);
    g_robot.receive(raw_proxim_l, 2);
    g_robot.receive(raw_proxim_lf, 2);
    g_robot.receive(raw_proxim_lc, 2);
    g_robot.receive(raw_proxim_rc, 2);
    g_robot.receive(raw_proxim_rf, 2);
    g_robot.receive(raw_proxim_r, 2);

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
void navigate()
{
    int16_t travelled_m = 0;
    int16_t travelled_mm = 0;
    int16_t turned_deg = 0;

    navigation_state state = nav_go;

        // Update travelled distance
        {
            int16_t distance_delta_mm = bytes_to_int16(distance_data[0], distance_data[1]);
            travelled_mm += distance_delta_mm;

            g_distance_bytes[0] = distance_data[0];
            g_distance_bytes[1] = distance_data[1];
            g_distance_delta = distance_delta_mm;
            g_distance = travelled_mm;

            if (travelled_mm >= 1000)
            {
                travelled_m += travelled_mm / 1000;
                travelled_mm = travelled_mm % 1000;
            }

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
#endif

void control()
{
    int back_up_ticks = 0;

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

        acquire_sensors(g_sensors);

        memory_barrier();

        Service_Publish(g_report_service, 0);

        if (g_sensors.wheel_drop)
        {
            g_robot.stop();
            OS_Abort();
        }

        bool back_up = sum(g_sensors.proximity, 6) > 200;

        back_up_ticks = (back_up ? 100 : max(back_up_ticks - 1, 0));

        if (back_up_ticks)
        {
            drive_straight(g_robot, -100);
        }
        else
        {
            //rotate
            //drive(g_robot, 20, 1);

            drive_straight(g_robot, 100);
        }

        Task_Next();
    }
}

void report()
{
    ServiceSubscription *report = Service_Subscribe(g_report_service);

    for(;;)
    {
        Service_Receive(report);

        memory_barrier();

        radio_packet_t tx_packet;
        tx_packet.type = debug_packet_type;
        for (int i = 0; i < 6; ++i)
            tx_packet.debug.proximity[i] = g_sensors.proximity[i];

        Radio_Transmit(&tx_packet, RADIO_WAIT_FOR_TX);

        // Clear receive buffer
        radio_packet_t rx_packet;
        while(Radio_Receive(&rx_packet) == RADIO_RX_MORE_PACKETS) {}
    }
}

void init_radio()
{
    // Start up radio
    pinMode(arduino::pin_radio_vcc, OUTPUT);

    digitalWrite(arduino::pin_radio_vcc, LOW);
    delay(100);
    digitalWrite(arduino::pin_radio_vcc, HIGH);
    delay(100);

    Radio_Init(RADIO_CHANNEL);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, radio_robot0_address, ENABLE);
    // configure radio transceiver settings
    Radio_Configure(RADIO_RATE, RADIO_HIGHEST_POWER);

    // set destination address
    Radio_Set_Tx_Addr(radio_base_address);
}

int r_main()
{
    pinMode(13, OUTPUT);

    //Serial.begin(9600);

    g_report_service = Service_Init();


    // Init radio

    init_radio();

    // Init robot

    g_robot.begin();

    g_robot.send(irobot::op_full_mode );

    {
        // Confirm mode
        uint8_t mode;
        g_robot.send(irobot::op_sensor, irobot::sense_oi_mode);
        g_robot.receive(&mode, 1);
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

    // Create tasks

    Task_Create_RR(report, 0);

    Task_Create_Periodic(control, 0, 5, 5, 0);

    Task_Periodic_Start();

    return 0;
}
