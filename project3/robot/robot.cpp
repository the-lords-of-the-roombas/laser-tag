#include "controller.hpp"
#include "sequencer.hpp"
#include "sonar.hpp"
#include "gun.hpp"
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

static uint16_t volatile control_period_ticks = 5;

static Service * volatile g_radio_service = 0;

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
    Service_Publish(g_radio_service, pipenumber);
}
}

// Sonar

Service * volatile g_sonar_request_service = 0;
Service * volatile g_sonar_reply_service = 0;

// Gun

gun g_gun;
Service * volatile g_shot_service;

// Control

irobot g_robot(&Serial1, arduino::pin_baud_rate_change);
static controller::input_t g_ctl_in;
static controller::output_t g_ctl_out;
static sequencer::input_t g_seq_in;
static sequencer::output_t g_seq_out;
Service * volatile g_ctl_out_service;


void sequence()
{
    sequencer seq(&g_ctl_in, &g_ctl_out, control_period_ticks * TICK,
                  &g_seq_in, &g_seq_out,
                  g_sonar_request_service, g_sonar_reply_service,
                  &g_gun);
    seq.run();
}

void control()
{
    controller ctl(&g_robot, &g_ctl_in, &g_ctl_out, g_shot_service,
                   control_period_ticks * TICK);
    ctl.run();
}

void report()
{
    ServiceSubscription *sonar_sub = Service_Subscribe(g_sonar_reply_service);
    ServiceSubscription *shot_sub = Service_Subscribe(g_shot_service);
    ServiceSubscription *radio_sub = Service_Subscribe(g_radio_service);

    ServiceSubscription *subs[] = { sonar_sub, shot_sub, radio_sub };

    for(;;)
    {
#if 0
        controller::output_t ctl_out;
        sequencer::output_t seq_out;

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ctl_out = g_ctl_out;
            seq_out = g_seq_out;
        }
#endif
        unsigned int srv_idx;
        int16_t srv_value = Service_Receive_Mux(subs, 3, &srv_idx);

        switch(srv_idx)
        {
        case 0: // Sonar reply
        {
            sequencer::input_t seq_in;
            int16_t sonar_cycles  = srv_value;
            seq_in.sonar_cm = sonar::cycles_to_cm(sonar_cycles);

            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                g_seq_in = seq_in;
            }

            break;
        }
        case 1: // Shot
        {
            int16_t shooter_id = srv_value;

            radio_packet_t tx_packet;
            tx_packet.type = shot_packet_type;
            tx_packet.shot.shooter_id = shooter_id;
            tx_packet.shot.target_id = MY_ID;

            Radio_Transmit(&tx_packet, RADIO_RETURN_ON_TX);

            break;
        }
        case 2: // Radio
        {
            radio_packet_t rx_packet;
            RADIO_RX_STATUS rx_status = RADIO_RX_MORE_PACKETS;
            while(rx_status == RADIO_RX_MORE_PACKETS)
            {
                rx_status = Radio_Receive(&rx_packet);
                if ( rx_status == RADIO_RX_MORE_PACKETS ||
                     rx_status == RADIO_RX_SUCCESS )
                {
                    // Packet received
                    switch(rx_packet.type)
                    {
                    case sonar_trigger_packet_type:
                    {
                        if (rx_packet.sonar_trigger.id == MY_ID)
                            Service_Publish(g_sonar_request_service, 0);
                        break;
                    }
                    default:
                        break;
                    }
                }
            }

            break;
        }
        default:
            break;
        }
    }
}

void sonar_task()
{
    sonar s;
    s.init(g_sonar_request_service, g_sonar_reply_service);
    s.work();
}

void init_radio()
{
    static uint8_t radio_base_address[5] = BASE_RADIO_ADDRESS;
    static uint8_t radio_robot_address[5] = { BOT_RADIO_ADDRESS_PREFIX, MY_ID };

    // Start up radio
    pinMode(arduino::pin_radio_vcc, OUTPUT);

    digitalWrite(arduino::pin_radio_vcc, LOW);
    delay(100);
    digitalWrite(arduino::pin_radio_vcc, HIGH);
    delay(100);

    Radio_Init(RADIO_CHANNEL);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, radio_robot_address, ENABLE);
    // configure radio transceiver settings
    Radio_Configure(RADIO_RATE, RADIO_HIGHEST_POWER);

    // set destination address
    Radio_Set_Tx_Addr(radio_base_address);
}

int r_main()
{
    pinMode(13, OUTPUT);

    // Init radio

    g_radio_service = Service_Init();

    init_radio();

    // Init sonar

    g_sonar_request_service = Service_Init();
    g_sonar_reply_service = Service_Init();

    // Init gun

    g_gun.init();
    g_shot_service = Service_Init();

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

#if 0
    for(int i = 0; i < 3; ++i)
    {
        digitalWrite(13, HIGH);
        delay(300);
        digitalWrite(13, LOW);
        delay(300);
    }
#endif

    // Make sure all shared memory initialization is finished.

    memory_barrier();

    // Create tasks

    Task_Create_RR(report, 0);

    Task_Create_System(sonar_task, 0);

    Task_Create_RR(sequence, 0);

    Task_Create_Periodic(control, 0, control_period_ticks, control_period_ticks, 0);

    Task_Periodic_Start();

    return 0;
}

