#include "../world.hpp"
#include "../radio/radio.h"
#include "../radio_packets/radio_packets.hpp"
#include <Arduino.h>

static uint8_t radio_base_address[5] = BASE_RADIO_ADDRESS;
static uint8_t radio_robot0_address[5] = ROBOT_0_RADIO_ADDRESS;

static int radio_power_pin = 10;

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
}
}

int main()
{
    // Init Arduino library
    init();

    pinMode(13, OUTPUT);
    Serial.begin(9600);

    // Start up radio
    pinMode(radio_power_pin, OUTPUT);

    digitalWrite(radio_power_pin, LOW);
    delay(100);
    digitalWrite(radio_power_pin, HIGH);
    delay(100);


    Radio_Init(RADIO_CHANNEL);

    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, radio_base_address, ENABLE);
    // configure radio transceiver settings
    Radio_Configure(RADIO_RATE, RADIO_HIGHEST_POWER);

    // set destination address
    Radio_Set_Tx_Addr(radio_robot0_address);

    for(;;)
    {
        radio_packet_t rx_pkt;
        RADIO_RX_STATUS rx_status = Radio_Receive(&rx_pkt);
        if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS)
        {
            digitalWrite(13, HIGH);
            switch(rx_pkt.type)
            {
            case game_status_packet_type:
                break;
            case shot_packet_type:
            {
                Serial.print("SHOT: ");
                Serial.print("Shooter = ");
                Serial.print(rx_pkt.shot.shooter_id);
                Serial.print(" / Target = ");
                Serial.print(rx_pkt.shot.target_id);
                Serial.println();
                break;
            }
            case debug_packet_type:
            {
                Serial.println("---");

                Serial.print("Bumps: ");
                Serial.print(rx_pkt.debug.bump_left);
                Serial.print(" ");
                Serial.print(rx_pkt.debug.bump_right);
                Serial.println();

                Serial.print("Object: ");
                Serial.print(rx_pkt.debug.object_left);
                Serial.print(" ");
                Serial.print(rx_pkt.debug.object_right);
                Serial.println();

                Serial.print("Coin:");
                Serial.println(rx_pkt.debug.coin);

                Serial.print("Bhavior:");
                Serial.println(rx_pkt.debug.seq_behavior);
#if 0
                Serial.print("Behavior: ");
                Serial.print(rx_pkt.debug.ctl_behavior);
                Serial.println();

                Serial.print("Sonar: ");
                Serial.print(rx_pkt.debug.sonar_cm);
                Serial.println();

                Serial.print("Seeking: ");
                Serial.print(rx_pkt.debug.obj_motion);
                Serial.print(" ");
                Serial.println(rx_pkt.debug.obj_seek);

                Serial.print("Last dir: ");
                Serial.println(rx_pkt.debug.last_dir);
                Serial.print("Radius: ");
                Serial.println(rx_pkt.debug.radius);
#endif



#if 0
                Serial.print("Proximity:");
                for (int i = 0; i < 3; ++i)
                {
                    Serial.print(rx_pkt.debug.proximities[i]);
                    Serial.write(' ');
                }
                Serial.println();
#endif
                //Serial.print("P C = ");
                //Serial.println(rx_pkt.debug.proximity_center);

                break;
            }
            default:
                Serial.println("Unknown packet type!");
                break;
            }
        }
        else
        {
            delay(100);
            digitalWrite(13, LOW);
        }
    }
}
