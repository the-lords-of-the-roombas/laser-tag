#ifndef RADIO_PACKETS_INCLUDED
#define RADIO_PACKETS_INCLUDED

#include "global.hpp"
#include "local.hpp"

#include <stdint.h>

typedef struct radio_packet
{
    uint8_t type;
    uint16_t timestamp;

    // Payload
    union
    {
        // "_filler" just makes sure the packet is exactly 32 bytes long.
        uint8_t _filler[29];

        // global packets
        game_status_payload game_status;
        shot_payload shot;

        // your other packets...
        sonar_trigger_payload sonar_trigger;
        debug_payload debug;
    };

} radio_packet_t;

#endif // RADIO_PACKETS_INCLUDED
