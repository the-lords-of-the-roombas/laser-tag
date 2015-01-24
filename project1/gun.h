#ifndef UVIC_RTSYS_GUN_INCLUDED
#define UVIC_RTSYS_GUN_INCLUDED

#include <stdint.h>

struct gun_state
{
    bool enabled;
    uint16_t code;
    int current_bit;
};

void gun_init(gun_state*);

void gun_trigger(gun_state*, uint8_t code);

void gun_transmit(void*);

#endif // UVIC_RTSYS_GUN_INCLUDED
