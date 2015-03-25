#ifndef ROBOT_TAG_GUN_INCLUDED
#define ROBOT_TAG_GUN_INCLUDED

namespace robot_tag_game {

class gun
{
public:
    gun();
    void init();
    void run();
    void next_tick();
private:
    void send_lo();
    void send_hi();
    int current_bit_index;
    int current_bit_phase;
    char byte_to_send;
};

}

#endif // ROBOT_TAG_GUN_INCLUDED
