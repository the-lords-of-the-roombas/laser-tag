
#ifndef UVIC_RTSYS_SCHEDULER_INCLUDED
#define UVIC_RTSYS_SCHEDULER_INCLUDED

#include "time.h"

#include <stdint.h>

typedef void (*task_cb)();

typedef struct
{
    task_cb callback;
    uint8_t is_enabled;
    uint16_t period;
    uint16_t delay;
    milliseconds_t next_time;
} task_t;

void scheduler_task_init
(task_t *task, uint16_t delay, uint16_t period, task_cb cb, bool enabled=true);

bool scheduler_task_is_enabled(task_t *task);
void scheduler_task_set_enabled(task_t *task, bool enabled);

void scheduler_init(task_t *tasks, uint32_t task_count);

milliseconds_t scheduler_run();

#endif // UVIC_RTSYS_SCHEDULER_INCLUDED
