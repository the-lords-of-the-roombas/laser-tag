
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
    ctl_in.behavior = controller::wait;
    ctl_in.sonar_cm = 400;
    ctl_in.sonar_cm_seek_threshold = 100;

    for(;;)
    {
        uint16_t time = Now();

        Service_Publish(m_sonar_request, 0);
        int16_t sonar_cycles = Service_Receive(m_sonar_reply);
        uint16_t sonar_cm = sonar::cycles_to_cm(sonar_cycles);

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            ctl_out = *m_ctl_out;
        }

        // Update environment variables

        ctl_in.sonar_cm = sonar_cm;

        // Update behavior

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
#if 1
            if (sonar_cm <= ctl_in.sonar_cm_seek_threshold)
            {
                ctl_in.behavior = controller::approach;
            }
#endif
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

        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            *m_ctl_in = ctl_in;
        }

        while((Now() - time) < 200)
            ;
    }
}


}
