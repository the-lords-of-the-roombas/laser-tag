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
API and targeting a different Atmel chip, which we partially re-used.


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

Context switching is accomplished by saving the current stack pointer and pointing 
it at the kernel's stack pointer. Once that is complete, the kernel code is run and 
is free to handle all requests. At the bottom of the kernel's stack is the return address, 
this is the spot in the code where it was interrupted and forced into the kernel. 
When this is reached, the context is returned and the application code is resumed. 

In order to accomplish this, r31 and SREG must be saved onto the stack, the interrupts 
must be disabled, and then the rest of the registers must be saved onto the stack. 
To return context, these steps must be done in complete reverse order. 

Reporting errors
----------------
Two methods were implemented to handle errors. First is the kernel_abort_with() method 
which takes an error as a parameter and calls OS_Abort(). OS_Abort() simply displays an 
error code using a LED. The error is displayed using integer values and flashing for that 
many times. We determined there were 5 types of error. These are displayed below:

	// 1: User called OS_Abort().
	ERR_USER_CALLED_OS_ABORT = 0
	
	// 2: Periodic task created after periodic schedule start
	ERR_PERIODIC_SCHEDULE_SETUP
	
	// 3: Periodic schedule invalid (tasks overlap)
	ERR_PERIODIC_SCHEDULE_RUN
	
	// 4: Periodic task not allowed to subscribe to service
	ERR_SERVICE_SUBSCRIBE
	
	// 5: RTOS Internal error.
	ERR_RTOS_INTERNAL_ERROR
	
The second is kernel_assert() which takes a boolean as a parameter. If the parameter is 
set to false, kernel_abort_with will be called with the error ERR_RTOS_INTERNAL_ERROR.

Task creation
-------------

Task creation is done through calling Task_Create(). Creating a system or round robin 
task requires a type, the code the task will run, and an arg as parameters. Creating a 
periodic task requires the same as above as well as three other parameters; A period, 
an offset (start), and a worst case running time. In order to create these tasks, we 
have separated them into three separate calls, one for each type of task. These individual 
calls then call Task_Create() with the appropriate parameters. The parameters are set to 
kernel_request_create_args and enter_kernel() is called with the kernel_request set to 
TASK_CREATE. The kernel then calls kernel_create_task() and this is where the tasks stack 
is created and populated. The stack_bottom and stack_top are saved as integer pointers. 
The code the task needs to run is then pushed onto the stack as well as the address of 
Task_Terminate() to destroy the task if it ever returns. We then make the stack pointer 
point to cell above stack (the top), and make room for 32 registers, SREG and two return 
addresses. The task is then enqueued or added to the list according to it's level.

::

	int8_t Task_Create(uint8_t level, void (*f)(void), int16_t arg, uint16_t period, 
	uint16_t wcet, uint16_t start)
	{
		int retval;
		uint8_t sreg;
		
		sreg = SREG;
		
		Disable_Interrupt();
		
		kernel_request_create_args.f = (voidfuncvoid_ptr)f;
		kernel_request_create_args.arg = arg;
		kernel_request_create_args.level = level;
		kernel_request_create_args.period = period;
		kernel_request_create_args.wcet = wcet;
		kernel_request_create_args.start = start;
		kernel_request = TASK_CREATE;
		
		enter_kernel();
		
		retval = kernel_request_retval;
		SREG = sreg;
		
		return retval;
	}
	
	int8_t Task_Create_System(void (*f)(void), int16_t arg)
	{
		return Task_Create(SYSTEM, f, arg, 0, 0, 0);
	}
	
	int8_t Task_Create_RR( void (*f)(void), int16_t arg)
	{
		return Task_Create(RR, f, arg, 0, 0, 0);
	}
	
	int8_t Task_Create_Periodic(void(*f)(void), int16_t arg, uint16_t period, 
	uint16_t wcet, uint16_t start)
	{
		return Task_Create(PERIODIC, f, arg, period, wcet, start);
	}

2. New functionality
********************

Periodic task scheduling
------------------------

The rtos code that was given to us included two arrays. One called PPP that held a 
char value to identify the task and another called PT that held an int value to 
indicate the remaining ticks on that task. We decided to not use these arrays and 
instead implemented our own way to handle periodic task scheduling.

In general, our periodic_task_list is essentially a linked list. The tasks are never 
dequeued or removed from the list. Upon initializing the periodic_task_list, a 
variable called ticks_at_next_periodic_schedule_check is set equal to the start 
time of the next scheduled periodic task. This value is compared to the current 
tick at every tick of real time in kernel_update_ticker(). If these two values are 
equal, kernel_select_periodic_task() is called and the task is started. The value of 
ticks_at_next_periodic_schedule_check is also set to the next scheduled task time.

We created a few variables to allow us to do this.

A queue called periodic_task_list to hold all of our periodic tasks::

  static queue_t periodic_task_list;

A pointer to the current running periodic task and a boolean value to determine if 
the list of periodic tasks has begun running::

  task_descriptor_t *current_periodic_task = NULL;
  static bool periodic_tasks_running = false;

Variables for timing management::

  static uint16_t volatile ticks_since_system_start = 0;
  static uint16_t ticks_at_next_periodic_schedule_check = 0;
  static uint16_t ticks_since_current_periodic_task = 0;

These are the methods we created to manage periodic task scheduling.

void Task_Periodic_Start()
..........................

This method sets the kernel_request to TASK_PERIODIC_START and enters the kernel. Once 
in the kernel, the context and the current task’s stack pointer are saved. The system 
changes to the kernel’s stack pointer and processes the request. The kernel handles the 
request to TASK_PERIODIC_START by first determining if a periodic task is currently running. 
If so, the kernel will abort with error ERR_PERIODIC_SCHEDULE_SETUP. If periodic_task_running 
is false, it will iterate through the periodic_task_list and set the next_tick (the tick 
at which the task will start) with respect to the ticks_since_system_start and the task's 
offset. Once complete, the context and the stack pointer are restored.	

static void kernel_select_periodic_task()
.........................................

This method scans the periodic_task_list. If there is a current task running, it returns. 
If the ticks_since_system_start equals a tasks start time, it will start that task running. 
It then iterates through all tasks in the periodic list and determines the next task to be 
run and calculates the time until that will occur. The value of 
ticks_at_next_periodic_schedule_check is then set to this amount. Once a task is selected 
to run, the maximum run time of that task is compared to the start time of the next 
scheduled task. If these two tasks overlap, the kernel aborts with error ERR_PERIODIC_SCHEDULE_RUN.


The following methods were modified to handle our periodic task scheduling

static void kernel_update_ticker(void)
......................................

Our system is based on a count up of ticks rather than the "ticks_remaining" approach to 
the original rtos given to us. The first thing this function does is increment 
ticks_since_system_start. It then determines if there is a current_periodic_task and if 
that task is the cur_task. If so, ticks_since_current_periodic_task is incremented and 
the ticks_since_current_periodic_task is compared with the task's wcet. If the 
ticks_since_current_periodic_task is greater or equal to wcet, the kernel is aborted 
with error ERR_PERIODIC_SCHEDULE_RUN. Next, ticks_since_system_start is compared to 
ticks_at_next_periodic_schedule_check. If they are equal, kernel_select_periodic_task() 
is called and the selected task is started.

static void kernel_handle_request(void)
.......................................

This is where the kernel determines what the request is and handles it appropriately.

The request TASK_PERIODIC_START is the where the periodic task scheduler is initialized. 
The kernel first determines if the periodic_task_list has begun being processed. If not, 
periodic_tasks_running is set to true. If it has, there has been an error and the kernel 
is aborted with error ERR_PERIODIC_SCHEDULE_SETUP. Next the periodic_task_list is iterated 
through and all the tasks have their start time (next_tick) set with respect to 
ticks_since_system_start. Each task's start time is then set to ticks_since_system_start + 
1 + task->start. The extra tick added is to allow all tasks to be processed before the 
list is processed and tasks are started. 

static void kernel_dispatch(void)
.................................

This method determines if there is no task running or if the idle task is running. If so, 
a task is set to run. If there is a system task to run, it will set that task to run and 
return. If not it will see if there is a current periodic task running. If so, this task 
is continued running and the method will return. If not, it will determine if there is a 
round robin task to run and if so, it will set that task running. If all three checks fail, 
the current task is set to the idle_task.


Elapsed time
------------

The elapsed time is returned by calling the Now() function.

uint16_t Now()
..............

This method returns the current time in milliseconds. This is calculated by taking the 
ticks since the system started (last_tick) and multiplying that by TICK; 5 milliseconds. 
The time since the last tick is then calculated. First, the number of cycles at the last 
tick is calculated by taking the time from OCR1A and subtracting the number of clock 
cycles in one tick. Then the extra clock cycles is calculated by subtracting that value 
from TCNT1. Finally, the extra clock cycles are converted into milliseconds by dividing 
them by the amount of cycles per millisecond. The extra clock cycles are then added to 
the ticks since the system started to return an accurate value for the time since the 
system started.

::

  /** The RTOS timer's prescaler divisor */
  #define TIMER_PRESCALER 8
  #define CYCLES_PER_MS ((F_CPU / TIMER_PRESCALER) / 1000)

  /** The number of clock cycles in one "tick" or 5 ms */
  #define TICK_CYCLES (CYCLES_PER_MS * TICK)

  uint16_t last_tick = ticks_since_system_start;
  uint16_t cycles_at_last_tick = OCR1A - TICK_CYCLES;
  uint16_t cycles_extra = TCNT1 - cycles_at_last_tick;
  uint16_t ms_extra = cycles_extra / CYCLES_PER_MS;
  ms_now = last_tick * TICK + ms_extra;


Inter-process communication (services)
--------------------------------------

Three methods were created to deal with services. First a service is created and 
initialized by calling the Service_Init() method. Next the service is added to the 
subscribers queue. This is done by calling Service_Subscribe which switches the stack 
pointer with the kernels stack pointer and adds the service to the subscribers list. 
Once in the subscribers list, Service_Publish() is called. This is where the service is 
dequeued from the subscribers queue and added to the appropriate queue in the kernel. 

SERVICE *Service_Init()
.......................

This method returns an empty service that has been initialized in the services list.

void Service_Subscribe( SERVICE *s, int16_t *v )
................................................

Takes a pointer to a service and a value. The method sets the kernel_request to 
SERVICE_SUBSCRIBE and enters the kernel. Once in the kernel, the context and the 
current task’s stack pointer are saved. The system changes to the kernel’s stack 
pointer and processes the request. The kernel handles the request to SERVICE_SUBSCRIBE 
by setting the task’s state to WAITING and enqueueing it to the subscribers queue. If 
the task is periodic, the kernel will abort with ERR_SERVICE_SUBSCRIBE. Once complete, 
the context and the stack pointer are restored.

void Service_Publish( SERVICE *s, int16_t v )
.............................................

Takes a pointer to a service and a value. The method sets the kernel_request to 
SERVICE_PUBLISH and enters the kernel. Once in the kernel, the context and the 
current task’s stack pointer are saved. The system changes to the kernel’s stack 
pointer and processes the request. The kernel handles the request to SERVICE_PUBLISH 
by dequeueing it from the subscribers queue and enqueueing it in the kernel to the 
appropriate queue; either system_queue or rr_queue. Once complete, the context and 
the stack pointer are restored.


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

Services: communication between system tasks
--------------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_system_to_system.cpp>`__
- `Trace <traces/trace-service-system-to-system.png>`__

This test confirms the basic operation of services: one system task
publishes over a service to another system task.

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

Services: periodic task to system task
--------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_periodic_to_system.cpp>`__
- `Trace <traces/trace-service-periodic-to-system.png>`__

This test confirms that communication over a service from a periodic to
a system task works, and that the periodic task is preempted as soon as
it publishes.

The main task creates a service and a periodic task with a 1 tick period.
In each period, the periodic task publishes to the service and then works
for 1 ms. It sets the trace channel 5 high before publishing and low after
the end of work. Moreover, it sets the trace channel 4 high before publishing.
The main task repeatedly subscribes to the service and then sets the trace
channel 4 to low.

The trace confirms the assumption that the system task preempts the
periodic task as soon as the latter publishes while the former is subscribed
(the trace channel 4 becomes low before the channel 5).

Moreover, the duration
that the trace channel stays high is the time it takes to switch between the
tasks. The average of 8 measurements is 45.54 microseconds.

Service: round-robin task to system task
----------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_rr_to_system.cpp>`__
- `Trace <traces/trace-service-rr-to-system.png>`__

This test is very similar to the one above. The trace confirms preemption
of the round-robin task by the system task as soon as the former publishes
when the latter is subscribed. The average task switching time of 6
measurements is 47.23 microseconds, comparable to the other two.

Service: interrupt to system task
---------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_isr_to_system.cpp>`__
- `Trace <traces/trace-service-isr-to-system.png>`__

This test confirm success of communication over a service between an
interrupt service routine (ISR) and a system task.

The main task creates a service and sets up a hardware timer to trigger
an interrupt every 10 ms. The ISR publishes over the service one value
between 1 and 5. The main task repeatedly subscribes to the service and
the works for as many milliseconds as the value received over the service.

The trace channel 4 confirms the desired period between interrupts.
The trace channel 5 confirms that the value is transmitted successfully.

Service: interrupt to round-robin task
--------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_isr_to_rr.cpp>`__
- `Trace <traces/trace-service-isr-to-rr.png>`__

This test is very similar to the one above. Visual inspection of the code
and the trace confirms correct operation. No difference from the above
test is neither expected, nor observed.

Service: invalid subscription from periodic task
------------------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_to_periodic.cpp>`__
- `Trace <traces/trace-service-to-periodic.png>`__

According to the specification for the RTOS, it is not allowed for a
periodic task to subscribe to a service, and the OS should abort in this
case.

In this test, the main function creates a periodic task that starts at
3 ticks plus the time before the start of schedule (1 tick), which is in
total 4 ticks (20 ms). The trace confirms that the OS aborts at that time.

Service: bi-directional communication using two services
--------------------------------------------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_ping_pong.cpp>`__
- `Trace <traces/trace-service-ping-pong.png>`__

This test confirms that two system tasks can communicate back and forth
using two services.

The main task creates two services and two other system tasks. Each
task repeatedly publishes on one service and subscribes to the other.
Publishing makes the publisher yield to the subscriber, which in turn proceeds
to publish over another channel. In order to ensure that one task is indeed
subscribed to the service to which the other one publishes, each task yields
additionally before publishing.

In addition, after subscribing, each task works for as many milliseconds as the
value it receives over a service, increments that value and wraps it to
the range of 1 to 5, and publishes the result over the other service.
The amount of time each task works is indicated on the trace channels 4 and 5,
respectively. The trace confirms correct passing of values over the services.

Service: unicast
----------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_unicast.cpp>`__
- `Trace <traces/trace-service-unicast.png>`__

This test verifies one-to-many communication over multiple services, one
for each pair of communicating tasks.

The main task creates 3 services and three other system tasks. Each
task repeatedly subscribes to one of the services, and then works for
as many milliseconds as the value received over the service. The main task
repeatedly publishes a different value (1, 2, or 3) to each of the services.
To ensure that all subscribers are indeed subscribed at the time of publishing,
it yields before publishing.

Each subscriber keeps one of the trace channels 5, 6, and 7 high as long
as it is running. Moreover, the main task keeps the trace channel 4 high while
publishing to the three services. The trace indicates correct transmission
of values over the services, and expected order of execution of tasks
(the subscribers run in the order of publishing to their respective services).

Service: broadcast
------------------

- `Code <https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project2/rtos/test/test_service_broadcast.cpp>`__
- `Trace <traces/trace-service-broadcast.png>`__

This test verifies one-to-many communication over a single service.

The main task creates 1 service and three other system tasks. Each
task repeatedly subscribes to the single service, and then works for
as many milliseconds as the value received over the service. The main task
repeatedly publishes a different value (1, 2, or 3) to the service.
To ensure that all subscribers are indeed subscribed at the time of publishing,
it yields before publishing.

Each subscriber keeps one of the trace channels 5, 6, and 7 high as long
as it is running. Moreover, the main task keeps the trace channel 4 high while
publishing to the service. The trace indicates correct transmission
of values over the services, and expected order of execution of tasks.
The subscribers run in the order of their subscription, which is the same
as the order of their creation, due to their execution order when the main
task first yields.



