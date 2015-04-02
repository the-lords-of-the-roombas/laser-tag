#ifndef ROBOT_TAG_GAME_SEQUENCER_INCLUDED
#define ROBOT_TAG_GAME_SEQUENCER_INCLUDED

#include "controller.hpp"
#include "gun.hpp"
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
        chase,
        shoot,
        critical_turn_left,
        critical_turn_right
    };

    struct input_t
    {
        input_t(): sonar_cm(1000) {}
        uint16_t sonar_cm;
    };

    struct output_t
    {
        output_t(): coin(false), behavior(seek_straight) {}
        bool coin;
        behavior_t behavior;
    };

    sequencer(controller::input_t *ctl_in,
              controller::output_t *ctl_out,
              uint16_t ctl_period_ms,
              sequencer::input_t *seq_in,
              sequencer::output_t *seq_out,
              Service *sonar_request,
              Service *sonar_reply,
              gun *);

    void run();

private:
    bool target_visible(const sequencer::input_t & in)
    {
        return in.sonar_cm < 250;
    }

    void get(sequencer::input_t & in);

    void set(controller::input_t & in);
    void get(controller::output_t & out);
    void swap(controller::input_t & in, controller::output_t & out);
    void check_bumps(controller::output_t &, behavior_t &);
    uint16_t read_sonar();
    void wait_ms(uint16_t milliseconds);
    controller::direction_t opposite(controller::direction_t dir)
    {
        return dir == controller::leftward
                ? controller::rightward : controller::leftward;
    }

    controller::input_t *m_ctl_in;
    controller::output_t *m_ctl_out;
    uint16_t m_ctl_period_ms;

    sequencer::input_t *m_seq_in;
    sequencer::output_t *m_seq_out;

    Service *m_sonar_request;
    ServiceSubscription *m_sonar_reply;

    gun *m_gun;
};

}

#endif // ROBOT_TAG_GAME_SEQUENCER_INCLUDED
