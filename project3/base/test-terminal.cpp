#include "../base_terminal/protocol.h"

#include "Arduino.h"

using namespace robot_tag_game;

int main()
{
    init();

    Serial.begin(9600);

    int score1 = 0;
    int score2 = 0;

    for(;;)
    {
        Serial.print(base::terminal::msg_score);
        Serial.print(" ");
        Serial.print(0);
        Serial.print(" ");
        Serial.print(1);
        Serial.println();

        delay(1000);

        Serial.print(base::terminal::msg_score);
        Serial.print(" ");
        Serial.print(1);
        Serial.print(" ");
        Serial.print(2);
        Serial.println();

        delay(1000);
    }
}
