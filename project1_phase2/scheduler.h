
#ifndef UVIC_RTSYS_SCHEDULER_INCLUDED
#define UVIC_RTSYS_SCHEDULER_INCLUDED

#include "time.h"

#include <stdint.h>

typedef void (*task_cb)(void *object);

typedef struct
{
    void *object;
    task_cb callback;
    uint32_t is_enabled;
    uint32_t period;
    uint32_t delay;
    microseconds_t next_time;
} task_t;

void scheduler_task_init
(task_t *task, uint32_t delay, uint32_t period,
 task_cb cb, void *object, bool enabled=true);

bool scheduler_task_is_enabled(task_t *task);
void scheduler_task_set_enabled(task_t *task, bool enabled);

void scheduler_init(task_t *tasks, uint32_t task_count);

microseconds_t scheduler_run();

#endif // UVIC_RTSYS_SCHEDULER_INCLUDED
