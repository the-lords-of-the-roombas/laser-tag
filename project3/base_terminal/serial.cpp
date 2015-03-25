#include "serial.hpp"

#include <stdio.h>    // Standard input/output definitions
#include <unistd.h>   // UNIX standard function definitions
#include <fcntl.h>    // File control definitions
#include <errno.h>    // Error number definitions
#include <termios.h>  // POSIX terminal control definitions
#include <string.h>   // String function definitions
#include <sys/ioctl.h>
#include <sys/select.h>

namespace robot_tag_game {

serial::serial():
    m_fd(-1)
{

}

serial::error_code
serial::open(const char* port, int baud_rate)
{
    if (m_fd != -1)
        return false;

    struct termios toptions;
    int fd;

    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDONLY | O_NONBLOCK);

    if (fd == -1)
    {
        return failed_open;
    }

    //int iflags = TIOCM_DTR;
    //ioctl(fd, TIOCMBIS, &iflags);     // turn on DTR
    //ioctl(fd, TIOCMBIC, &iflags);    // turn off DTR

    if (tcgetattr(fd, &toptions) < 0)
    {
        close(fd);
        return failed_get_attributes;
    }

    speed_t brate = baud_rate; // let you override switch below if needed
    switch(baud_rate) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 1;
    toptions.c_cc[VTIME] = 1;
    //toptions.c_cc[VTIME] = 20;

    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0)
    {
        close(fd);
        return failed_set_attributes;
    }

    m_fd = fd;
    FD_ZERO(&m_fd_set);
    FD_SET(fd, &m_fd_set);

    return success;
}

bool await(unsigned int timeout_ms)
{
    if (!is_open())
        return;

    timeval timeout;
    timeout.tv_sec = timeout_ms / 1e3;
    timeout.tv_usec = (timeout_ms % 1e3) * 1e3;

    int result = select(1, &m_fd_set, 0, 0, &timeout, 0);

    return result != -1;
}

size_t read(const char *dst, size_t count)
{
    if (!is_open())
        return 0;

    return ::read(m_fd, dst, count);
}

}
