#include "radio/radio.h"
#include "radio_packets/radio_packets.hpp"
#include "irobot/irobot.hpp"
#include "arduino_config.h"
#include "Arduino.h"

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

void init_robot_comm()
{
    pinMode(arduino::pin_baud_rate_change, OUTPUT);

    // Wake up the robot.

    digitalWrite(arduino::pin_baud_rate_change, LOW);
    delay(100);
    digitalWrite(arduino::pin_baud_rate_change, HIGH);
    delay(2000);

    // Blink

    digitalWrite(13, HIGH);
    _delay_ms(100);
    digitalWrite(13, LOW);

    // Change baud rate to 19200.

    //digitalWrite(arduino::pin_baud_rate_change, HIGH);
    for(int i = 0; i < 4; ++i)
    {
        _delay_ms(100);
        digitalWrite(arduino::pin_baud_rate_change, LOW);
        _delay_ms(100);
        digitalWrite(arduino::pin_baud_rate_change, HIGH);
    }

    _delay_ms(100);

    // Initialize Arduino Serial1

    Serial1.begin(19200);

    // Initiate conversation with the robot

    Serial1.write(irobot::op_start);
}

int r_main()
{
#if 1
    pinMode(13, OUTPUT);

    Serial.begin(9600);

    init_robot_comm();

    while(true)
    {
        digitalWrite(13, HIGH);
        _delay_ms(200);
        digitalWrite(13, LOW);
        _delay_ms(200);


        Serial1.write(irobot::op_sensor);
        Serial1.write(irobot::sense_charging);
        //Serial1.write(irobot::sense_oi_mode);

        unsigned long time = micros();

        //_delay_ms(100);

        //char response[2];
        //char response;

        while(!Serial1.available() && (micros() - time) < 1e5)
            ;

        unsigned long time_after = micros();

        Serial.print("Time: ");
        Serial.print(time_after - time);
        Serial.print(" us");
        Serial.println();

        int b;
        while((b = Serial1.read()) >= 0)
        {
            Serial.print("Response: ");
            Serial.println(b);
        }

        Serial.println("Done.");
    }

#endif



    return 0;
}

#if 0
int main()
{
    init();
    return r_main();
}
#endif
