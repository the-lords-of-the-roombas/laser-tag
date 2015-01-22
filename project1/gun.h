#ifndef UVIC_RTSYS_GUN_INCLUDED
#define UVIC_RTSYS_GUN_INCLUDED

#include <stdint.h>

void gun_init();

void gun_trigger(uint8_t code);

void gun_task();

#endif // UVIC_RTSYS_GUN_INCLUDED
