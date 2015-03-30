
/*
  seek:
    turn around and look for first target closer than threshold
  approach:
    move forward until close proximity sensors show something
    if target lost, go back to seek
    when in close proximity, switch to "aim"
  aim:
    Face obstacle.
    When facing obstacle, shoot.
    If sonar lost, turn randomly around.
    If sonar lost for enough time, go back to seek.
*/

#include "sequencer.hpp"
#include "sonar.hpp"
#include <util/atomic.h>
#include <Arduino.h>

namespace robot_tag_game {

sequencer::sequencer
(controller::input_t *ctl_in, controller::output_t *ctl_out,
 Service *sonar_request, Service *sonar_reply):
    m_ctl_in(ctl_in),
    m_ctl_out(ctl_out)
{
    m_sonar_request = sonar_request;
    m_sonar_reply = Service_Subscribe(sonar_reply);
}

void sequencer::run()
{
    controller::input_t ctl_in;
    controller::output_t ctl_out;

    behavior_t behavior = seek_straight;
    uint16_t behavior_onset = Now();
    controller::direction_t last_turn = controller::left;

    for(;;)
    {
        Service_Publish(m_sonar_request, 0);
        int16_t sonar_cycles = Service_Receive(m_sonar_reply);
        uint16_t sonar_cm = sonar::cycles_to_cm(sonar_cycles);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ctl_out = *m_ctl_out;
        }

        uint16_t time = Now();

        // Update environment variables

        ctl_in.sonar_cm = sonar_cm;

        // Update behavior

        behavior_t next_behavior = behavior;

        switch(behavior)
        {
        case seek_straight:
        {
            if (time - behavior_onset > 3000)
            {
                if (last_turn == controller::left)
                    next_behavior = seek_right;
                else
                    next_behavior = seek_left;
            }
            else if (ctl_out.object_left || ctl_out.object_right)
            {
                if (!ctl_out.object_right)
                    next_behavior = seek_right;
                else if (!ctl_out.object_left)
                    next_behavior = seek_left;
                else
                    next_behavior = seek_right;
            }

            if (next_behavior == seek_right)
                last_turn = controller::right;
            else if (next_behavior == seek_left)
                last_turn = controller::left;

            break;
        }
        case seek_left:
        case seek_right:
        {
            if (time - behavior_onset > 500)
                next_behavior = seek_straight;
            break;
        }
        case critical_turn:
        {
            if (time - behavior_onset > 1000)
                next_behavior = seek_straight;
            break;
        }
        }

        if (ctl_out.bump)
            next_behavior = critical_turn;

        if (next_behavior != behavior)
        {
            behavior = next_behavior;
            behavior_onset = time;
        }

        // Execute behavior

        switch(behavior)
        {
        case seek_straight:
        {
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::straight;
            break;
        }
        case seek_left:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::left;
            break;
        case seek_right:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::right;
            break;
        case critical_turn:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::left;
            break;
        }

#if 0
        switch(ctl_in.behavior)
        {
        case controller::wait:
        {
            ctl_in.behavior = controller::seek;
            ctl_in.seek_direction = controller::left;
            break;
        }
        case controller::seek:
        {

            if (sonar_cm <= ctl_in.sonar_cm_seek_threshold)
            {
                ctl_in.behavior = controller::approach;
            }
            break;
        }
        case controller::approach:
        {
            if (sonar_cm > ctl_in.sonar_cm_seek_threshold)
            {
                ctl_in.behavior = controller::seek;
            }
            break;
        }
        default:
            break;
        }
#endif

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            *m_ctl_in = ctl_in;
        }

        while((Now() - time) < 200)
            ;
    }
}


}
