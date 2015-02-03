#ifndef UVIC_RTSYS_STATE_INCLUDED
#define UVIC_RTSYS_STATE_INCLUDED

#include "packet.h"

struct state
{
  struct
  {
    int x; // [-100, 100]
    int y; // [-100, 100]
    bool pressed;
  } joystick;

  struct
  {
    int16_t speed;
    int16_t radius;
  } drive;

  struct
  {
    bool joystick_was_pressed;
    bool transmit;
  } shoot;

  radiopacket_t tx_packet;
  radiopacket_t rx_packet;
};

#endif // UVIC_RTSYS_STATE_INCLUDED
