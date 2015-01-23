
#include "scheduler.h"
#include "time.h"

//#include <iostream>

using namespace std;

static task_t *tasks;
static uint32_t task_count;

void scheduler_task_init
(task_t *task, uint32_t delay, uint32_t period, task_cb cb, bool enabled)
{
    task->callback = cb;
    task->is_enabled = enabled;
    task->period = period;
    task->delay = delay;
    task->next_time = 0;
}

bool scheduler_task_is_enabled(task_t *task)
{
    return task->is_enabled;
}

void scheduler_task_set_enabled(task_t *task, bool enabled)
{
    task->is_enabled = enabled;
}

void scheduler_init(task_t *t, uint32_t c)
{
    tasks = t;
    task_count = c;

    microseconds_t now = current_time_micros();

    for(uint32_t task_idx = 0; task_idx < task_count; ++task_idx)
    {
        task_t & task = tasks[task_idx];
        task.next_time = now + task.delay;
    }
}

microseconds_t scheduler_run()
{
    microseconds_t now = current_time_micros();

    // FIXME: What to do in case no task?
    // Here, we are requesting a run at most after 1 second.
    microseconds_t one_second = 1e6;
    microseconds_t next_time = now + one_second;

    //std::cout << "now: " << now << endl;

    for(uint32_t task_idx = 0; task_idx < task_count; ++task_idx)
    {
        task_t & task = tasks[task_idx];

        //std::cout << "task " << task_idx << " at: " << task.next_time << endl;

        bool expired = task.next_time <= now;

        if (expired)
        {
            if (task.is_enabled)
                task.callback();

            //uint32_t next_task_time = task.next_time + task.period;
            task.next_time = now + task.period;

#if 0
            uint32_t time_since_task_start =
                    now - start_time - task.delay;
            task.next_time =
                    now - (time_since_task_start % task.period) + task.period;

#endif
#if 0
            //uint32_t occurence_count = (now - start_time - task.delay - 1) / task.period;
            //task.next_time = (occurence_count + 1) * task.period + task.delay + start_time;
#endif
        }

        if (task.next_time < next_time)
            next_time = task.next_time;
    }

    return next_time - now;
}
