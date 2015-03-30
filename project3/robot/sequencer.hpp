#ifndef ROBOT_TAG_GAME_SEQUENCER_INCLUDED
#define ROBOT_TAG_GAME_SEQUENCER_INCLUDED

#include "controller.hpp"
#include "../rtos/os.h"
#include <stdint.h>

namespace robot_tag_game {

class sequencer
{
public:
    enum behavior_t
    {
        seek_straight,
        seek_left,
        seek_right,
        critical_turn
    };

    sequencer(controller::input_t *ctl_in,
              controller::output_t *ctl_out,
              Service *sonar_request,
              Service *sonar_reply);

    void run();

private:
    controller::input_t *m_ctl_in;
    controller::output_t *m_ctl_out;
    Service *m_sonar_request;
    ServiceSubscription *m_sonar_reply;
};

}

#endif // ROBOT_TAG_GAME_SEQUENCER_INCLUDED
