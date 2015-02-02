#ifndef UVIC_RTSYS_STATE_INCLUDED
#define UVIC_RTSYS_STATE_INCLUDED

#include "packet.h"

struct state
{
  int x; // [-100, 100]
  int y; // [-100, 100]
  bool sw;
  int speed;
  radiopacket_t tx_packet;
  radiopacket_t rx_packet;
};

#endif // UVIC_RTSYS_STATE_INCLUDED
