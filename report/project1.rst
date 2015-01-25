CSC 460/560: Design and Analysis of Real-Time Systems
=====================================================

Project 1
=========

Authors: Darren Prince and Jakob Leben


Overview
********

Requirements:

- Joystick controls servo to aim attached IR emitter towards target.
- Joystick button triggers emission of a code as IR modulation.
- Software uses time-triggered architecture.

Additional:

- Joystick allows selection of one of a predefined set of codes to
  be emitted.
- A set of LED lights indicates index of selected code in binary form.

Components
**********

(Add details)

- Arduino Mega 2560 (http://www.arduino.cc/)
- Joystick...
- Servo motor...
- IR emitter...
- Power supply
- ...

How each component operates?


Reading joystick data
---------------------

Hardware
........

The joystick was wired to two analog inputs on the Arduino board. The x-axis was wired to analog0 and the y-axis 
was wired to analog1, The joystick switch (registering a press) was wired to digital input 8. The power source 
was wired with the help of a pull up resistor to assist in recording the value of the joystick switch being pressed.

Software
........

The joystick position was polled every 20 ms and the joystick switch was polled every 40ms. Originally we had 
the joystick switch being polled every 20 ms but we found that this resulted in the system recognizing that it 
had been pressed more than once. 

The position of the joystick determined which direction and speed the servo would move in. The value reported by
the joystick was added to the current position of the servo. A larger value reported by the joystick would make the 
servo move faster than a lower value.::

    // Update control variable and keep it in range 0 to 3000
    int servo_ctl = jstick->servo_control + step;
    int range = 3000;
    if(servo_ctl < 0) servo_ctl = 0;
    if(servo_ctl > range) servo_ctl = range;
    jstick->servo_control = servo_ctl;

    // Map control variable to servo degrees and output that to servo
    int servo_degrees = map(servo_ctl, 0, range, 0, 180);
    servo.write(servo_degrees);
	
We configured the software to detect a press of the joystick when the analog signal was below 90 for
downward press and above 270 for a upward press. Originally the joystick was reporting a value of 
between 0 and 1023. However, we placed a resistor in the joystick circuit to be used as a pull up resistor and our 
range of values dropped to a minimum of 0 and a maximum of 360. 

Selecting transmission code
---------------------------

The goal of adding a method for selecting which code was to be transmitted was to
avoid having to hard-code a 'letter' and change this hard coding when we were
instructed to transmit to a different receiver. 

Hardware
........

The hardware used to implement the data transmission rotation was the joystick and two led lights.
The two led lights were setup side by side, to display, in binary, which data the system would emit 
on a joystick press. No leds being lit would refer to a binary code of 0 which we would associate with 
the letter 'A'. A binary code of 1 would refer to 'B', 2 to 'C', and 3 to 'D'.

Software
........


Controlling servo
-----------------

Hardware
........

Software
........


Emitting code over IR
---------------------

The requirement was to transmit 1 byte of information encoded as modulation
of IR emission:

- A bit of transmission is represented as the binary choice between:
    - Oscillation between full and zero emission amplitude at 38kHz (value 1)
    - Constant zero emission amplitude (value 0).
- The length of each bit is 500 microseconds, meaning that emission should
  stay in a state corresponding to the value of any bit for 500 microseconds
  and then change to the state corresponding to the next bit.
- Before a byte is transmitted, a transition from 1 to 0 and a full-length 0
  bit must be observed by the receiver.

Hardware
........

We decided to use the capability of the ATmega2560 chip to generate a PWM
waveform for the 38kHz modulation of IR emission which corresponds to the bit
value of 1. A PWM signal with 50% pulse width was used for the value 1. We
first tried to use the 0% pulse width for the value 0, but this did not
result in constant LOW output, but still produced short HIGH spikes. We
solved this by rather disabling the PWM output altogether to generate the
value 0.

We used the Arduino digital pin 13 for PWM output. The positive
pin of the IR LED was thus connected to this pin, in addition to connecting
the ground pins of the two devices. Moreover, the amount of current drawn
from the Arduino was limited with a resistor.

<diagram>

Software
........

A challenge arises because the Arduino library only allows PWM generation at
a fixed frequency, which is not our desired frequency of 38kHz.
Therefore, we used the AVR C library to precisely configure a chip's
hardware timer and its PWM waveform generation by explicitely setting
the related registers.

Another issue is that the Arduino Servo library uses all the chip's timers
in order to support controlling a large number of Servo motors by the same
Arduino chip. This is heavily redundant as we are only controlling a single
motor. We have solved this by modifying the Servo library and disabling its
use of the timer that we used for our purpose. Luckily, the Servo library
allows a very convenient way to do this by simply removing a
pre-processor definition. In particular, we have commented out the line 62
in the library's header file to disable use of timer number 1 of which
waveform-generation output is connceted to Arduino's digital pin 13.
Disabling the timer 1 in the Servo library is okay,
because the first created instance of the Servo class will use the timer
number 5, and we only need one instance::

    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    #define _useTimer5
    //#define _useTimer1
    #define _useTimer3
    #define _useTimer4

We could then use the timer 1 for our purpose.
In order to generate a 38kHz PWM signal we need to make the timer counter reset
every 1/38000 seconds, that is about 26.316 microseconds.
With our chip's 16MHz clock speed, no clock pre-scaler and a 16 bit counter
register, that corresponds to about 421 clock cycles with no counter register
overflow. This was calculated using the online
`timer calculator`__ by Frank Zhao. In the timer's Fast PWM mode, the
counter will reset when reaching the value in the Output-Compare Register A,
which was set to 412. We generated a 50% PWM signal using the
Waveform Generator C, by setting the  Output-Compare Register C to 210.

.. __: http://eleccelerator.com/avr-timer-calculator/

The timer was configured with the following code::

    #define GUN_TIMER_TOP 421
    #define GUN_TIMER_HALF 210

    void gun_init(gun_state * gun)
    {
        // ...

        // Clear control registers
        TCCR1A = 0;
        TCCR1B = 0;

        // Disable interrupt from Output-Compare C
        TIMSK1 &= ~(1<<OCIE1C);

        // Select Fast PWM mode (15)
        TCCR1A |= (1<<WGM10) | (1<<WGM11);
        TCCR1B |= (1<<WGM12) | (1<<WGM13);

        // Use clock with no prescaler
        TCCR1B |= (1<<CS10);

        // Set counter TOP value by setting Output-Compare Register A
        OCR1A = GUN_TIMER_TOP;

        // Set Waveform Generator C to 50% pulse width
        // by setting Output-Compare Register C
        OCR1C = GUN_TIMER_HALF;

        // ...
    }

Arduino has the output of the Waveform Generator C of Timer 1 connected to
its digital pin 13, which was set to output mode::

    void gun_init(gun_state * gun)
    {
        //  ...
        pinMode(13, OUTPUT);
        //  ...
    }

In order to generate the bit values according to our IR communcation protocol,
we enabled the Waveform Generator C output for 500 microseconds for the value 1,
and disabled it for the value 0::

    static void gun_send_hi()
    {
        // Enable output C:
        TCCR1A |= (1<<COM1C1);
    }

    static void gun_send_lo()
    {
        // Disable output C:
        TCCR1A &= ~(1<<COM1C1);
    }

Transmission of a byte was performed by a periodic task which transmits one
bit every period (500 microseconds). The transmission initialization was
realized by simply transmitting 2 more constant bits (a 1 and a 0) before
transmitting the byte of information. In total, there is thus 10 bits to
transmit. These bits were stored in a 16 bit unsigned integer variable
``gun->code``
In addition, the periodic transmission task required the information about
the last transmitted bit index to persist across periods, which was
stored in the variable ``gun->current_bit``.
The task is activated by other parts of the system by
setting the ``gun->enabled`` variable, and the task disables itself after
transmitting the 10 bits::

  void gun_transmit(void *object)
  {
      gun_state *gun = (gun_state*)object;

      if (gun->enabled)
      {

          if((gun->code >> gun->current_bit) & 0x1)
              gun_send_hi();
          else
              gun_send_lo();

          ++gun->current_bit;

          if (gun->current_bit >= 10)
              gun->enabled = false;
      }
  }


The big picture
---------------

Complete electrical diagram


Task scheduling and communication
*********************************

Schedule
........

We have the following periodic tasks:

#. Servo control
    - Purpose: Reading joystick position and controlling servo position.
    - Period: 20 ms
    - Delay: 0 ms
#. Code selection:
    - Purpose: Reading joystick position and selection of code.
    - Period: 40 ms
    - Delay: 5 ms
#. Gun trigger:
    - Purpose: Reading status of joystick button and triggering
      transsmission of selected code.
    - Period: 40 ms
    - Delay: 10 ms
#. Code transmission:
    - Purpose: Transmission of each individual bit of code.
    - Period: 0.5 ms (500 us)
    - Delay: 0.25 ms (250 us)

This schedules each occurence of each task at a different time.
The most time-critical task is code transmission, because the inter-onset
of each of its executions is quite important. This task also has
the shortest period - 500 us, which means that the onset of the task may
occur at best 250 us away from an onset of a different task. If this
other task was running for more than 250 us, that would delay the onset
of the most time-critical task. However, mind that the correct execution of
this task only matters during the 10-bit transmission. Ideally, this takes
in total exactly 5 ms. Since onsets of any other pair of tasks are at least
5 milliseconds appart, the transmission of a code will always fit into such
a window. Regardless, in our application there is actually no need for
other tasks to run during code transmission, so all issues could be avoided
by temporary disabling the other tasks during transmission. We might explore
this direction in future.

Here is a diagram of one period of the time-triggered scheduled:

<diagram>

Communication
.............

One challenge with the time-triggered design is communication between
tasks. Strong separation of tasks makes scheduling more manageable
and robust, but it opens the question of how communication between tasks
should be implemented.

The ultimate goal of a real-time system is of course
to process real-world inputs and produce real-world outputs.
In the time-triggered design, this processing is broken down into tasks as
atomic units of scheduling.
Some tasks are also atomic units of information flow - they are isolated in that
they only work with system inputs and outputs.
In contrast, some tasks may work with information produced by other tasks.

Let's analyze our information flow and inter-task communication requirements:

.. image:: communication.svg

In the above diagram, tasks are represented as black rectangles.
System inputs and outputs are represented as red ellipses in grey rectangles
which represent other physical systems.
Variables shared between tasks are represented by blue ellipses.

The following state variables are shared between the tasks:

- selected code index
- transmitted code
- transmission status

The following describes the nature of inter-task communication:

- The servo control task does not communicate with other tasks,
  it only works with input from the joystick and output to the servo motor.

- The code selection task reads input from the joystick, updates
  the selected code index and controls the LED's that indicate current
  code selection.

- The gun trigger reads the transmission status to determine whether a
  transmission is currently in progress, and aborts if it is. Otherwise it
  reads the selected code index, uses it to access
  a code from an array of available codes and updates the code to be transmitted.
  It then sets the transmission state to "enabled".

- The code transmission task reads the transmission state variable. If
  it is enabled, it proceeds by reading one bit from the code to be transmitted
  and actually transmitting it. If this was the last bit to be transmitted, it
  updates its status to disabled so that transmission of a new code can be
  triggered.



Since all our code runs sequentially, using shared memory between tasks
is a simple and feasible solution. Nevertheless, the question remains
how to structure the passing of data from the software engineering
point of view. Ideally, the code for a task should be agnostic to
the existence of other tasks and its place in the system as a whole,
and only expose an interface for input and output of data.

Our solution so far is quite simplistic, but feasible for the currently
small amount of tasks and shared variables. We tried to group
state variables into meaningful groups (the ``joystick_state``
and ``gun_state`` structs). Some tasks only access state in one of these
groups, hence the code for these tasks resides in separate files.
However, there are tasks (e.g. the gun trigger task) that facilitate
the carrying of data between other tasks. Hence, we have
*task-specific* "glue code" that resides in the main program file and has
global access. This is less than ideal, and we are contemplating an
organizatinal principle that would avoid mixing low-level task-specific
code with high-level task coordination code.

Scheduler
.........

We implemented our own task scheduler, according to the
Time-Triggered Architecture paradigm. The scheduler represents time
in microseconds using a 32 bit unsigned integer variable which will
overflow in about 70 minutes. It is possible to implement the scheduler
so that it will handle most tasks as expected even in the case of overflow,
as will be explained later.

A task is represented with the following struct::

    typedef uint32_t milliseconds_t;

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

Each task has a callback function executed at times when the task is scheduled,
as well as a pointer to an object representing the task state
passed to the callback. The struct also stores the task period and delay, in
microseconds. The field ``next_time`` is used by the scheduler as an efficient
way of knowing the next time when the task should be executed.

The scheduler is initialized using the ``scheduler_init`` function. At
this moment, it obtains from the chip the current time in
microseconds since the system was started, and sets
 the next execution time of each task to the current time
plus the task's period::

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



The ``scheduler_run`` function again obtains the current time from the system
and compares it to each task's next execution time. If the current time is
larger than the time of any task, the callback function of the task is
executed. This comparison will fail when the task's scheduled time has
overflown but the current time hasn't yet::

    microseconds_t scheduler_run()
    {
        microseconds_t now = current_time_micros();

        // FIXME: What to do in case no task?
        // Here, we are requesting a run at most after 1 second.
        microseconds_t one_second = 1e6;
        microseconds_t next_time = now + one_second;

        for(uint32_t task_idx = 0; task_idx < task_count; ++task_idx)
        {
            task_t & task = tasks[task_idx];

            bool expired = task.next_time <= now;

            if (expired)
            {
                if (task.is_enabled)
                    task.callback(task.object);

                task.next_time = task.next_time + task.period;
            }

            if (task.next_time < next_time)
                next_time = task.next_time;
        }

        return next_time - now;
    }
