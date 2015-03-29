#include "sonar.hpp"
#include "../arduino_config.h"
#include <Arduino.h>
#include <util/delay.h>

using namespace robot_tag_game;

Service * volatile sonar_request_service = 0;
Service * volatile sonar_reply_service = 0;

static void report()
{
    ServiceSubscription *sub = Service_Subscribe(sonar_reply_service);

    for(;;)
    {
        int16_t duration_cycles = Service_Receive(sub);
        /*
        cycles_per_second = 16e6 / SONAR_CLOCK_SCALE;
        cycles_per_us = 16 / SONAR_CLOCK_CYCLE;
        int16_t us = cycles / cycles_per_us;
        cm = us / 58;
        cm = cycles / (cycles_per_us * 58) = cycles / (58 * 16 / SONAR_CLOCK_SCALE)
        */
        int16_t duration_ms =
                ((uint32_t) duration_cycles * SONAR_CLOCK_SCALE) / (58UL * 16UL);
#if 0
        digitalWrite(13, HIGH);
        delay(50);
        digitalWrite(13, LOW);
#endif
        Serial.print(duration_cycles);
        Serial.print(" ");
        Serial.println(duration_ms);
    }
}

static void trigger()
{
    //pinMode(12, OUTPUT);

    for(;;)
    {
        /*for (int i = 0; i < 10; ++i)
            _delay_ms(100);*/
        delay(300);

        //digitalWrite(12, HIGH);
        Service_Publish(sonar_request_service, 0);
        //digitalWrite(12, LOW);
    }
}

static void measure()
{
    sonar s;
    s.init(sonar_request_service, sonar_reply_service);
    s.work();
}

int r_main()
{
    Serial.begin(9600);
    Serial.println("start");

    //pinMode(arduino::pin_sonar_io, OUTPUT);
    pinMode(13, OUTPUT);

    sonar_request_service = Service_Init();
    sonar_reply_service = Service_Init();

    Task_Create_RR(trigger, 2);
    Task_Create_System(measure, 3);
    Task_Create_RR(report, 4);

    return 0;
}
