#ifndef ROBOT_TAG_GUN_INCLUDED
#define ROBOT_TAG_GUN_INCLUDED

namespace robot_tag_game {

// Sending a byte takes 36 ms
// (4 ms each bit + a leading 0 bit = 9 * 4 ms)

class gun
{
public:
    gun();
    void init();
    void run();
    bool send(char byte);

    // Internal!
    void next_tick();
private:
    void send_lo();
    void send_hi();
    int m_current_bit_index;
    int m_current_bit_phase;
    char m_byte_to_send;
};

}

#endif // ROBOT_TAG_GUN_INCLUDED
