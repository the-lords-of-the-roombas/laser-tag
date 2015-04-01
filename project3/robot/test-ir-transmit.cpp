#include "gun.hpp"
#include "../arduino_config.h"
#include "Arduino.h"

using namespace robot_tag_game;

int r_main()
{
    gun g;

    g.init();

    for(;;)
    {
        delay(500);
        g.send('w');
    }

    return 0;
}
