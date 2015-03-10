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
addresses. The task is then enqueued or added to the list according to it's level.::

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

A queue called periodic_task_list to hold all of our periodic tasks.
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
system started.::

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
of the tasks. After that, a task switches the trace channel 7 high.
