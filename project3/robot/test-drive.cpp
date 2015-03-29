#include "controller.hpp"
#include "../world.hpp"
#include "../arduino_config.h"
#include "../util.h"
#include "../rtos/os.h"
#include "../irobot/irobot.hpp"
#include "../radio/radio.h"
#include "../radio_packets/radio_packets.hpp"
#include <util/atomic.h>
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
static control_behavior g_ctl_behavior;
static control_info g_ctl_info;
Service * volatile g_ctl_info_service;


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
    controller ctl(&g_robot, &g_ctl_behavior, &g_ctl_info, g_ctl_info_service);
    ctl.run();
}

void report()
{
    ServiceSubscription *report = Service_Subscribe(g_ctl_info_service);

    for(;;)
    {
        Service_Receive(report);

        control_info info;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            info = g_ctl_info;
        }
#if 0
        radio_packet_t tx_packet;
        tx_packet.type = debug_packet_type;
        for (int i = 0; i < 3; ++i)
            tx_packet.debug.proximities[i] = g_sensors_derived.proximities[i];

        Radio_Transmit(&tx_packet, RADIO_WAIT_FOR_TX);

        // Clear receive buffer
        radio_packet_t rx_packet;
        while(Radio_Receive(&rx_packet) == RADIO_RX_MORE_PACKETS) {}

        delay(200);
#endif
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

    g_ctl_info_service = Service_Init();

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

    // Init controller

    g_ctl_behavior.type = control_behavior::face_obstacle;

    memory_barrier();

    // Create tasks

    Task_Create_RR(report, 0);

    Task_Create_Periodic(control, 0, 5, 5, 0);

    Task_Periodic_Start();

    return 0;
}
