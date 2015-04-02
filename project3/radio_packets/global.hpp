#ifndef GLOBAL_RADIO_PACKETS_INCLUDED
#define GLOBAL_RADIO_PACKETS_INCLUDED

enum global_packet_types
{
    game_status_packet_type = 0,
    shot_packet_type,
    sonar_trigger_packet_type,

    // ... there may be more in future ...

    global_packet_type_count = 20
};

struct game_status_payload
{
    uint8_t it_id;
};

struct shot_payload
{
    uint8_t target_id;
    uint8_t shooter_id;
};

struct sonar_trigger_payload
{
    uint8_t id;
};

#endif // GLOBAL_RADIO_PACKETS_INCLUDED
