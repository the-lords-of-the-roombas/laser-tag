
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
#include "../world.hpp"
#include <util/atomic.h>
#include <Arduino.h>

namespace robot_tag_game {

sequencer::sequencer
(controller::input_t *ctl_in, controller::output_t *ctl_out,
 uint16_t ctl_period_ms,
 sequencer::output_t *seq_out,
 Service *sonar_request, Service *sonar_reply, gun *g):
    m_ctl_in(ctl_in),
    m_ctl_out(ctl_out),
    m_ctl_period_ms(ctl_period_ms),
    m_seq_out(seq_out),
    m_gun(g)
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

    //behavior_t last_turn_behavior = seek_left;
    controller::direction_t last_seek_dir = controller::leftward;


    static const int target_distance_threshold_cm = 100;

    for(;;)
    {
        // Execute behavior

        behavior_t next_behavior = behavior;

        switch(behavior)
        {
        case seek_straight:
        {
            uint16_t duration = (uint16_t) random_uint8(TCNT1) * 14 + 1000;
            uint16_t onset = Now();

            ctl_in.behavior = controller::go;
            ctl_in.direction = controller::forward;
            ctl_in.speed = controller::fast;
            if (last_seek_dir == controller::leftward)
            {
                ctl_in.radius = -600;
                last_seek_dir = controller::rightward;
            }
            else
            {
                ctl_in.radius = 600;
                last_seek_dir = controller::leftward;
            }

            set(ctl_in);

            while(next_behavior == seek_straight)
            {
                wait_ms(100);

                Service_Publish(m_sonar_request, 0);
                int16_t sonar_cycles = Service_Receive(m_sonar_reply);
                uint16_t sonar_cm = sonar::cycles_to_cm(sonar_cycles);

                get(ctl_out);

                bool target_seen = sonar_cm < target_distance_threshold_cm;

                if (target_seen)
                {
                    next_behavior = chase;
                    break;
                }
                else if (Now() - onset > duration)
                {
                    next_behavior = seek_straight;
                    // Force restart of the same behavior:
                    break;
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

                // Critical
                check_bumps(ctl_out, next_behavior);
            }

            break;
        }
        case seek_left:
        case seek_right:
        {
            // FIXME: look for target while turning

            ctl_in.behavior = controller::move;
            ctl_in.direction = behavior == seek_left ?
                        controller::leftward : controller::rightward;
            ctl_in.speed = controller::slow;
            ctl_in.distance = controller::mm_to_distance(150, m_ctl_period_ms);

            set(ctl_in);

            do
            {
                wait_ms(25);

                get(ctl_out);

                check_bumps(ctl_out, next_behavior);
            }
            while (next_behavior == behavior && ctl_out.remaining_distance);

            next_behavior = seek_straight;

            break;
        }
        case chase:
        {
            ctl_in.behavior = controller::chase;
            ctl_in.speed = controller::super_fast;

            set(ctl_in);

            while(next_behavior == chase)
            {
                wait_ms(100);

                Service_Publish(m_sonar_request, 0);
                int16_t sonar_cycles = Service_Receive(m_sonar_reply);
                uint16_t sonar_cm = sonar::cycles_to_cm(sonar_cycles);

                get(ctl_out);

                bool target_seen = sonar_cm < target_distance_threshold_cm;

                if (!target_seen)
                {
                    next_behavior = seek_straight;
                }
                else if (ctl_out.object_centered)
                {
                    next_behavior = shoot;
                }

                // Critical
                check_bumps(ctl_out, next_behavior);
            }

            break;
        }
        case shoot:
        {
            uint16_t hop_distance = controller::mm_to_distance(50, m_ctl_period_ms);

            ctl_in.behavior = controller::move;
            ctl_in.direction = controller::leftward;
            ctl_in.speed = controller::fast;
            ctl_in.distance = hop_distance;

            set(ctl_in);
            do { wait_ms(25); get(ctl_out); }
            while (ctl_out.remaining_distance);

            for (int i = 0; i < 3; ++i)
            {
                if (i > 0)
                {
                    ctl_in.behavior = controller::move;
                    ctl_in.direction = controller::rightward;
                    ctl_in.speed = controller::fast;
                    ctl_in.distance = hop_distance;

                    set(ctl_in);
                    do { wait_ms(25); get(ctl_out); }
                    while (ctl_out.remaining_distance);
                }

                m_gun->send(MY_ID);

                wait_ms(50);
            }

            next_behavior = critical_turn_right;

            break;
        }
        case critical_turn_left:
        case critical_turn_right:
        {
            ctl_in.behavior = controller::move;
            ctl_in.direction = (behavior == critical_turn_left) ?
                        controller::leftward : controller::rightward;
            ctl_in.speed = controller::fast;
            ctl_in.distance = controller::mm_to_distance(300, m_ctl_period_ms);

            set(ctl_in);
            do { wait_ms(25); get(ctl_out); }
            while (ctl_out.remaining_distance);

            next_behavior = seek_straight;

            break;
        }
        default:
            break;
        }

        if (next_behavior != behavior)
        {
            behavior = next_behavior;
            behavior_onset = Now();
        }
    }
}

void sequencer::set(controller::input_t & in)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        *m_ctl_in = in;
    }
}

void sequencer::get(controller::output_t & out)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        out = *m_ctl_out;
    }
}

void sequencer::swap(controller::input_t & in, controller::output_t & out)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        *m_ctl_in = in;
        out = *m_ctl_out;
    }
}

void sequencer::check_bumps(controller::output_t & out, behavior_t & behavior)
{
    if (out.bump_left || out.bump_right)
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            m_ctl_out->bump_left = false;
            m_ctl_out->bump_right = false;
        }

        if (out.bump_left)
            behavior = critical_turn_right;
        else
            behavior = critical_turn_left;
    }
}

void sequencer::wait_ms(uint16_t milliseconds)
{
    uint16_t start = Now();
    while ((Now() - start) < milliseconds)
        ;
}

}
