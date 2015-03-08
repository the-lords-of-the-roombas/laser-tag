#include <avr/io.h>
#include <util/delay.h>
#include "../os.h"
#include "test_util.h"

void idle()
{
    for(;;) {
        Task_Next();
    }
}

int r_main()
{
    Task_Create_Periodic(idle, 2, 200, 10, 0);
    Task_Create_Periodic(idle, 3, 200, 10, 100);
    Task_Periodic_Start();

    // The first periodic task has 1sec period, and 50ms WCET;
    // its WCET is being extended during main task.
    // However, the second period task starts at 1/5sec,
    // at which time the OS should abort.
    for(;;) {}

    return 0;
}
