#include "radio/radio.h"
#include "radio_packets/radio_packets.hpp"
#include "irobot/irobot.hpp"
#include "arduino_config.h"
#include "Arduino.h"

using namespace robot_tag_game;

extern "C" {

void radio_rxhandler(uint8_t pipenumber)
{

}

}

void radio_test()
{
    radio_packet packet;

    packet.type = game_status_packet_type;
    packet.game_status.it_id = 1;

    Radio_Transmit(&packet, RADIO_WAIT_FOR_TX);
}

int r_main()
{

    pinMode(13, OUTPUT);

    Serial.begin(9600);

    irobot robot(&Serial1, arduino::pin_baud_rate_change);

    robot.begin();

    while(true)
    {
        digitalWrite(13, HIGH);
        _delay_ms(100);
        digitalWrite(13, LOW);
        _delay_ms(100);

        robot.flush_received();

        robot.send(irobot::op_sensor, irobot::sense_infrared_omni);

        char response;
        if(robot.receive(&response, 1))
        {
            if (response == 0)
                Serial.print("No character.");
            else
            {
                Serial.print("Character: ");
                Serial.print(response);
            }
            Serial.println();
        }
        Serial.print("Done.");
    }

    return 0;
}

#if 0
int main()
{
    init();
    return r_main();
}
#endif
