#ifndef ROBOT_TAG_GAME_BASE_TERMINAL_SERIAL_INCLUDED
#define ROBOT_TAG_GAME_BASE_TERMINAL_SERIAL_INCLUDED

namespace robot_tag_game {

class serial
{
public:
    enum error_code
    {
        success = 0,
        already_open,
        failed_open,
        failed_get_attributes,
        failed_set_attributes
    };

    class error
    {
    public:
        error(error_code c): code(c) {}
        error_code code;
    };

    serial();
    error_code open(const char* port, int baud_rate);
    bool is_open() { return m_fd != -1; }
    bool await(unsigned int timeout_ms);
    size_t read(const char *dst, size_t count);

private:
    int m_fd;
    int m_fd_set;
};

}

#endif //ROBOT_TAG_GAME_BASE_TERMINAL_SERIAL_INCLUDED
