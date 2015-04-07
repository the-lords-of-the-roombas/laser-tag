#include "../world.hpp"
#include "../radio/radio.h"
#include "../radio_packets/radio_packets.hpp"
#include <Arduino.h>

static uint8_t bot_ids[4] = { 'A', 'B', 'C', 'D' };

static int radio_power_pin = 10;

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
}
}


void init_radio()
{
    // Start up radio
    pinMode(radio_power_pin, OUTPUT);

    digitalWrite(radio_power_pin, LOW);
    delay(100);
    digitalWrite(radio_power_pin, HIGH);
    delay(100);

    uint8_t radio_base_address[5] = BASE_RADIO_ADDRESS;

    Radio_Init(RADIO_CHANNEL);
    // configure the receive settings for radio pipe 0
    Radio_Configure_Rx(RADIO_PIPE_0, radio_base_address, ENABLE);
    // configure radio transceiver settings
    Radio_Configure(RADIO_RATE, RADIO_HIGHEST_POWER);
}

void handle_packet( const radio_packet_t & rx_pkt )
{
    switch(rx_pkt.type)
    {
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
    default:
        Serial.println("Received unexpected packet type!");
        break;
    }
}

void transmit_sonar_trigger_packet(uint8_t bot_id)
{
    radio_packet_t tx_pkt;
    tx_pkt.type = sonar_trigger_packet_type;
    tx_pkt.sonar_trigger.id = bot_id;

    uint8_t bot_address[5] = { BOT_RADIO_ADDRESS_PREFIX, bot_id };

    Radio_Set_Tx_Addr(bot_address);

    Radio_Transmit(&tx_pkt, RADIO_RETURN_ON_TX);
}

int main()
{
    // Init Arduino library
    init();

    // Init user communication
    pinMode(13, OUTPUT);
    Serial.begin(9600);

    // Init radio
    init_radio();

    unsigned int bot_id_index = 0;

    unsigned long then = millis();

    for(;;)
    {
        radio_packet_t rx_pkt;
        RADIO_RX_STATUS rx_status = Radio_Receive(&rx_pkt);
        if (rx_status == RADIO_RX_SUCCESS || rx_status == RADIO_RX_MORE_PACKETS)
        {
            digitalWrite(13, HIGH);
            handle_packet(rx_pkt);
        }
        else
        {
            digitalWrite(13, LOW);
        }

        unsigned int now = millis();

        if (now - then >= 50)
        {
            then = now;

            transmit_sonar_trigger_packet(bot_ids[bot_id_index]);

            ++bot_id_index;
            if(bot_id_index >= 4)
                bot_id_index = 0;
        }
    }
}
