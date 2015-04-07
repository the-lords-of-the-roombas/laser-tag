#include "../world.hpp"
#include "../arduino_config.h"
#include "../rtos/os.h"
#include "../radio/radio.h"
#include "../radio_packets/radio_packets.hpp"
#include "Arduino.h"

using namespace robot_tag_game;

static uint8_t radio_base_address[5] = BASE_RADIO_ADDRESS;
static uint8_t radio_robot0_address[5] = { 'B', 'O', 'T', '_', MY_ID };

extern "C" {
void radio_rxhandler(uint8_t pipenumber)
{
}
}

void report()
{
    pinMode(13, OUTPUT);

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

    int i = 0;

    for(;;)
    {
        delay(1000);

        radio_packet_t tx_packet;
        tx_packet.type = debug_packet_type;
        tx_packet.debug.test = i++;

        if (Radio_Transmit(&tx_packet, RADIO_WAIT_FOR_TX) == RADIO_TX_SUCCESS)
        {
            digitalWrite(13, HIGH);
            delay(100);
            digitalWrite(13, LOW);
        }
        else
        {
            digitalWrite(13, HIGH);
            delay(500);
            digitalWrite(13, LOW);
        }

        // Clear receive buffer
        radio_packet_t rx_packet;
        while(Radio_Receive(&rx_packet) == RADIO_RX_MORE_PACKETS) {}
    }
}

int r_main()
{
    Task_Create_RR(report, 0);

    return 0;
}
