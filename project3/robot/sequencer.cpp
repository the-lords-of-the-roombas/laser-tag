
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
#include "../util.h"
#include <util/atomic.h>
#include <Arduino.h>

namespace robot_tag_game {

sequencer::sequencer
(controller::input_t *ctl_in, controller::output_t *ctl_out,
 sequencer::output_t *seq_out,
 Service *sonar_request, Service *sonar_reply):
    m_ctl_in(ctl_in),
    m_ctl_out(ctl_out),
    m_seq_out(seq_out)
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

    uint16_t seek_direction_time = 3000;
    behavior_t last_turn_behavior = seek_left;
    //controller::direction_t seek_direction;

    bool blink_led = false;
    bool coin = false;

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

        bool target_seen = sonar_cm < 100;

        ctl_in.sonar_cm = sonar_cm;

        // Select behavior

        behavior_t next_behavior = behavior;

        switch(behavior)
        {
        case seek_straight:
        {
            if (target_seen)
            {
                next_behavior = chase;
            }
            else if (time - behavior_onset > seek_direction_time)
            {
                seek_direction_time = (uint16_t) random_uint8(TCNT1) * 10 + 2000;

                // 1/4 chance true
                coin = (random_uint8(TCNT1) & 0xFF) < 64;

                if (coin)
                    next_behavior = last_turn_behavior;
                else
                {
                    if (last_turn_behavior == seek_left)
                        next_behavior = seek_right;
                    else
                        next_behavior = seek_left;
                }

                last_turn_behavior = next_behavior;
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
#if 0
            if (next_behavior == seek_right)
                last_turn = controller::right;
            else if (next_behavior == seek_left)
                last_turn = controller::left;
#endif
            break;
        }
        case seek_left:
        case seek_right:
        {
            if (target_seen)
            {
                next_behavior = chase;
            }
            else if (time - behavior_onset > 300)
                next_behavior = seek_straight;
            break;
        }
        case chase:
        {
            if (!target_seen)
            {
                next_behavior = seek_right;
            }
            else if (ctl_out.object_centered)
            {
                blink_led = true;
            }
            else
            {
                blink_led = !blink_led;
            }

            if (blink_led)
                digitalWrite(13, HIGH);
            else
                digitalWrite(13, LOW);
            break;
        }
        case critical_turn_right:
        case critical_turn_left:
        {
            if (time - behavior_onset > 300)
                next_behavior = seek_straight;
            break;
        }
        }

        // Select critical behaviors if necessary

        if (ctl_out.bump_left)
            next_behavior = critical_turn_right;
        else if (ctl_out.bump_right)
            next_behavior = critical_turn_left;

        // Switch to selected behavior

        if (next_behavior != behavior)
        {
            behavior = next_behavior;
            behavior_onset = time;
        }

        if (behavior != chase)
        {
            digitalWrite(13, LOW);
        }

        // Execute behavior

        switch(behavior)
        {
        case seek_straight:
        {
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::straight;
            ctl_in.speed = controller::fast;
            break;
        }
        case seek_left:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::left;
            ctl_in.speed = controller::slow;
            break;
        case seek_right:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::right;
            ctl_in.speed = controller::slow;
            break;
        case chase:
            ctl_in.behavior = controller::chase;
            ctl_in.direction = controller::straight;
            ctl_in.speed = controller::super_fast;
            break;
        case critical_turn_left:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::left;
            ctl_in.speed = controller::fast;
            break;
        case critical_turn_right:
            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::right;
            ctl_in.speed = controller::fast;
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
            m_seq_out->behavior = behavior;
            m_seq_out->coin = coin;
            *m_ctl_in = ctl_in;
        }

        while((Now() - time) < 200)
            ;
    }
}


}
