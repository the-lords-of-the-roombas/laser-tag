#ifndef ROBOT_TAG_SONAR_INCLUDED
#define ROBOT_TAG_SONAR_INCLUDED

#include "../rtos/os.h"

namespace robot_tag_game {

class sonar
{
public:
    sonar();
    void init(Service *request_service, Service *reply_service);
    int pin();
    void work();
private:
    static void task();
    void speak();
    int listen();
    Service * volatile m_request_service;
    Service * volatile m_reply_service;
    ServiceSubscription * volatile m_request_sub;
    ServiceSubscription * volatile m_echo_sub;
};

}

#endif // ROBOT_TAG_SONAR_INCLUDED
