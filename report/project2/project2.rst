CSC 460/560: Design and Analysis of Real-Time Systems
=====================================================

Project 2
=========

Authors: Jakob Leben and Darren Prince

Code: https://github.com/the-lords-of-the-roombas/laser-tag

Overview
********

The main topic of this project is the real-time operating system (RTOS).
We were provided a specification of an RTOS application programming interface
in form of a C header file containing function declarations which we had to
implement for the Atmel ATMega2560 chip on an Arduino board.
We were also provided a partial implementation of an older
API and targetting a different Atmel chip, which we partially re-used.


Objectives:

1. Port provided RTOS code to the new API and the Atmel ATMega2560 chip:
    - Modify context switching code for "extended addressing" of program memory
      on Atmel ATMega2560.
    - Adjust how different error types are reported when the OS aborts.
    - Fit the task creation code to the new API: split the single task creation
      function into three: one for each task type (system, periodic, round-robin).

2. Add new functionality to the RTOS:
    - Implement a new periodic task scheduling mechanism.
    - Implement a function to get time in milliseconds since system start.
    - Implement simple inter-process communication ("services")

3. Test and profile the RTOS:
    - Implement a number of test programs that validate the operation of
      the RTOS.
    - Measure durations of important RTOS functions.

1. Porting provided RTOS code
*****************************

Context switching and extended addressing
-----------------------------------------

Reporting errors
----------------

Task creation
-------------

2. New functionality
********************

Periodic task scheduling
------------------------

Elapsed time
------------

Inter-process communcation (services)
-------------------------------------

3. Testing and profiling
************************

Main
----

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_main.cpp>`__
- `Trace <traces/trace-main.png>`__

This is the basic sanity test the confirms that the application's main function
``r_main`` is called at system startup as the main task.

The main function switches the trace channel 4 between high and low every 5 ms.

System task creation
--------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_system_create.cpp>`__
- `Trace <traces/trace-system-create.png>`__


The main task works for 1 ms, creates another system task, works for 1 ms more,
and then terminates. The created task does exactly the same. Thus, an infinite
chain of tasks is created where each one creates the next one.

System tasks should not be pre-empted when they create other system tasks,
so every task should complete its 2 ms work before the next task runs.
This is visible in the task-trace channels. Each next task is
assigned a task-trace channel equal to task number % 4.

Moreover, each task switches the trace channel 4 high just before creating
another task, and low just after that. Thus, we can measure the time it
takes to create a task.
The average of 6 measurements is 48.3 microseconds.

System tasks yielding to each other
-----------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_system_yield.cpp>`__
- `Trace <traces/trace-system-yield.png>`__


The main task creates 3 system tasks.

Each created task indicates its operation by
switching a trace channel high, working for some time, and switching it back
low; each one operates on a different trace channel (4, 5, or 6) and
does a different amount of work (1, 2, or 3 ms), which allows identification
of the tasks.

After doing some work, a task switches the trace channel 7 high, yields,
and then switches the channel back low. Because a different task starts
running as soon as one yields, the trace channel will be switched high by
the yielding task and then low by the task that gets to run next. We can
thus measures the task switching time between consecutive rising and falling
edges of the trace channel 7.
The average of 6 measurements is 38.92 microseconds.

Periodic task creation
----------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_create.cpp>`__
- `Trace <traces/trace-periodic-create.png>`__

This simple test just confirms that a periodic task is created, started
at the specified time and run at a specified interval.

The main task creates one periodic task.
Before starting the periodic schedule, the main task works for 8 ms.
The periodic schedule starts at the next tick, which is at 10 ms.

The periodic task starts at 0 ticks, it has a period of 1 tick and WCET of
1 tick. It keeps the trace channel 5 high while running. It works for
1 ms before yielding, which is within its WCET.

Moreover, the main task swithes the trace channel 4
high just before and low just after the periodic task creation,
allowing to measure the task creation time. One measurement gave 48.584
microseconds, which is not significantly different from the system task
creation. This is expected, as the code path is very similar.

Periodic task scheduling
------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_schedule.cpp>`__
- `Trace <traces/trace-periodic-schedule.png>`__

The main task creates 3 periodic tasks:

  1. Start = 0 ticks, Period = 2 ticks, WCET = 1 tick
  2. Start = 1 ticks, Period = 4 ticks, WCET = 1 tick
  3. Start = 3 ticks, Period = 4 ticks, WCET = 1 tick

It then works for 4 ms before starting the periodic schedule. The schedule
will thus start at the next tick, which is at 5 ms.

Each periodic task keeps a different trace channel high while running (channel
4, 5, or 6), and works for 1 ms before yielding. This verifies that the task
code actually runs. It also allows to measure when a task first runs,
and the time difference between two onsets of a task.
The measured onset times correspond to the requested periodic schedule.

Invalid periodic schedule
-------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_schedule_overlap.cpp>`__
- `Trace <traces/trace-periodic-schedule-overlap.png>`__

This test confirms that the OS aborts when trying to run an invalid periodic
task schedule.

The main function creates three periodic tasks:

  1. Start = 0 ticks, Period = 2 ticks, WCET = 1 tick
  2. Start = 1 ticks, Period = 4 ticks, WCET = 3 tick
  3. Start = 3 ticks, Period = 4 ticks, WCET = 1 tick

The second task has WCET 3 ticks, which makes it overlap with the first task.
For example, first execution of the second task starts at 1 tick and may
run until 1 + 3 = 4th tick. However, the second execution of the first task
starts at 2 ticks.

The OS aborts at the moment the offending task (the second task) is about to
run, which is at 1 tick. Since the periodic schedule starts at 10 ms, the
OS aborts at 15 ms.

Periodic task takes too long
----------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_task_timeout.cpp>`__
- `Trace <traces/trace-periodic-task-timeout.png>`__

This test confirms that the OS aborts when a task does not yield within
its WCET.

The main task creates 2 periodic tasks:

  1. Start = 0 ticks, Period = 5 ticks, WCET = 1 tick
  2. Start = 1 ticks, Period = 5 ticks, WCET = 1 tick

The second task never yields. The OS aborts at the moment when the offending
task first reaches its WCET, which is at 2 ticks. Because the periodic
schedule starts at 10 ms, the OS aborts at 20 ms.

Periodic task preemption
------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_preempt.cpp>`__
- `Trace <traces/trace-periodic-preempt.png>`__

This test confirms:

  - System tasks preempt periodic tasks.
  - Allowed running time of periodic tasks is extended beyond their WCET
    by the duration that they are being preempted.
  - None of this affects inter-onset time of periodic tasks.

The main task creates a periodic task which starts at 1 tick, has a period
of 5 ticks and WCET of 1 tick.

The periodic task repeatedly creates a
system task and then yields. It sets the trace channel 4 high just
before creation of the system task and low just after that.

The system
task sets the trace channel 5 high, works for 10 ms (2 ticks), sets the trace
channel low, and then terminates.

By observing the trace channels 4 and 5, we deduce that the periodic task is
preempted by the system task as soon as the system task is created, and the
system task runs to completion before the periodic task resumes. This means
that it will take at least 10 ms (2 ticks) before the periodic task yields,
which is longer than its WCET (1 tick). However, the OS does not abort, which
means the allowed runnning time of the periodic task is successfully extended
beyond its WCET while it is being preempted.

The trace also confirms that the inter-onset time of the periodic task is
unaffected (5 ticks = 25 ms).

Periodic task preemption too long
---------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_periodic_preempt_timeout.cpp>`__
- `Trace <traces/trace-periodic-preempt-timeout.png>`__

This test confirms that the OS aborts when preemption of a periodic task
extends its running time beyond the next onset of a periodic task.

The main task creates 2 periodic tasks:

  1. Start = 0 ticks, Period = 5 ticks, WCET = 1 tick
  2. Start = 1 ticks, Period = 5 ticks, WCET = 1 tick

The first periodic task creates a system task which preempts it for longer
than its WCET, thus running over the onset of the second task. The OS
aborts when the second task is first about to run - that is at 1 tick
plus the 5 ms offset of the periodic schedule start = 10 ms.

Round-robin task creation
-------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_rr_create.cpp>`__
- `Trace <traces/trace-rr-create.png>`__

This simple test confirms that round-robin tasks are created successfully.

The main task creates a round-robin task which starts running after the
main task completes its 10 ms of work. The round-robin task switches
the trace channel 5 between high and low every 2 ms.

Moreover, the main task switches the trace channel 4 high just before
creation of the round-robin task, and low just after that, which allows
to measure the task creation time. One measurement gave 48.416 microseconds,
comparable to creation of other tasks, as expected.

Round-robin task interleaving
-----------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_rr_interleave.cpp>`__
- `Trace <traces/trace-rr-interleave.png>`__

This test confirms that round-robin tasks are interleaved in the order of
their creation, each one running for 1 tick.

The main task creates 4 round-robin tasks, works for 10 ms and then terminates,
at which point the first round-robin task runs.

Each round-robin task indicates operation by switching a different trace channel
(4, 5, 6, or 7). Repeatedly, the channel is switched between high and low
every 23 ms.

We can observe from the first 4 trace channels that tasks are indeed being
switched every single tick (5ms). Moreover, the last 4 trace channels indicate
that all the tasks progress at the same speed, completing each of their
23 ms work periods at the same time. Because they are interleaved, this
time is extended to about 4 times 23 ms = 92 ms (a bit shorter because
of different starting times).

Round-robin task preemption
---------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_rr_preempt.cpp>`__
- `Trace <traces/trace-rr-preempt.png>`__

This test confirms that round-robin tasks are preempted both by system and
periodic tasks.

The main task creates a round-robin task and a periodic task, and then
terminates.

The round-robin task repeatedly works for 20 ms and then creates a system
task, switching the trace channel 4 high and low just before and after
the system task creation.

The system task switches the trace channel 5 high, works for 1 ms, and then
switches the trace channel back low.

The periodic task runs every 1 tick (5 ms). At each run, it switches the
trace channel 6 high, works for 1 ms, and switches the trace channel back
to low.

By comparing the trace channel 0 (which shows when the periodic task is
being selected as the current kernel task) with other channels, we can
observe that the round-robin task is being preempted by both other types of
tasks.
Moreover, the trace also shows a case where an occurrence of a
system task overlaps with a scheduled occurence of the periodic task, displacing
the execution of the periodic task forwad in time. This results in an
increase of the periodic task's inter-onset time from 5 ms to 5.8 ms, and
preemption of the round-robin task for 2 ms instead of 1 ms.


System clock
------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_time.cpp>`__
- `Trace <traces/trace-time.png>`__

This test confirms that the system clock works correctly - that is, the
function ``Now`` returns the time in milliseconds since start of OS.

The main function repeatedly picks one of the 4 different durations (3, 6, 9,
or 12 milliseconds). Each time, it queries the OS time, works for the
desired duration, and queries the OS time again. Then it computes the
difference between the reported time measurements and works for the
computed amount of time.

The trace channel 4 is switched high just before and low just after the two
time queries, and the channel 5 is switched high just before and low just after
the work period corresponding to the measured time. This way it is possible
to measure and compare the actual measured duration with the duration
reported by the OS. The trace confirms that they match.

Note that the
slight differences are due to the unavoidable imperfection of the duration of
the ``_delay_ms`` function, the overhead of switching pins high and low
and of the called functions, and the hardware's smallest quantum of time - the
duration of a single CPU cycle.

Service: communcation between system tasks
------------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_system_to_system.cpp>`__
- `Trace <traces/trace-service-system-to-system.png>`__

The main task creates a service and a system task that will publish over
the service.
The publisher repeatedly picks a number between 5 and 1 and publishes it
over the service. The main task repeatedly subscribes to the service and then
works for as many milliseconds as the number received over the service.
The main task sets the trace channel 5 high just before the work and low just
after that. Measuring the work times confirms that the correct values are passed
over the service.

Moreover, the publisher sets the trace channel 4 high just before publishing,
and the main task sets it low just after subscribing. This way we can measure
the time it takes to switch from the publisher to the subscriber. Three
measurements gave an average of 47.66 microseconds.


