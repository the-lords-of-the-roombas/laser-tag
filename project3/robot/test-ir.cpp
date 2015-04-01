#include "gun.hpp"
#include "../irobot/irobot.hpp"
#include "../arduino_config.h"
#include "../rtos/os.h"

#include "Arduino.h"

using namespace robot_tag_game;

int r_main()
{
    Serial.begin(9600);

    irobot robot(&Serial1, arduino::pin_baud_rate_change);
    robot.begin();

    gun g;
    g.init();

    uint16_t time = Now();

    while(true)
    {
        uint8_t responses[20];

        uint8_t *response = responses;

        unsigned long start, stop;

        start = micros();

        for (int i = 0; i < 20; ++i)
        {
            //delay(1000);

            robot.send(irobot::op_sensor, irobot::sense_infrared_omni);

            uint8_t c = 0;
            robot.receive(&c, 1);
            if (c == 0)
                c = '.';

            *response = c;
            ++response;
        }

        stop = micros();

        Serial.write(responses, 20);
        Serial.print(" ");
        Serial.print(stop - start);
        Serial.println();

        if (Now() - time >= 500)
            g.send('z');
    }

    return 0;
}
