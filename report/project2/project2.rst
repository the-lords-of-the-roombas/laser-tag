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
takes to create a task. The average of 6 measurements is 48.3 us.

System tasks yielding to each other
-----------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_system_yield.cpp>`__
- `Trace <traces/trace-system-yield.png>`__


The main task creates 3 system tasks. Each one indicates its operation by
switching a trace channel high, working for some time, and switching it back
low; each one operates on a different trace channel (4, 5, or 6) and
does a different amount of work (1, 2, or 3 ms), which allows identification
of the tasks. After that, a task switches the trace channel 7 high.
