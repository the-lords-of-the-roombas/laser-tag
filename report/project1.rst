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

Software


Controlling servo
-----------------

Hardware

Software


Emitting code over IR
---------------------

The requirement was to transmit 1 byte of information encoded as modulation
of IR emission:

- A bit of transmition is represented as the binary choice between:
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

In order to generate the bit values according to our IR communcation protocol,
we enabled the Waveform Generator C output for 500 microseconds for the value 1,
and disabled it for the value 0::

    // Enable Waveform Generator C output:
    TCCR1A |= (1<<COM1C1);

    //...

    // Disable Waveform Generator C output:
    TCCR1A &= ~(1<<COM1C1);

Arduino has the output of the Waveform Generator C of Timer 1 connected to
its digital pin 13, which was set into output mode::

    pinMode(13, OUTPUT);

The big picture
---------------

Complete electrical diagram


Task scheduling and communication
*********************************

We have the following periodic tasks:

- <Task 1>:
    - purpose, period
- <Task 2>:
    - purpose, period
- <task 3>
    - purpose, period

Here is a diagram of the time-triggered scheduled:

<diagram>

We have the following state variables:

Here is a diagram of communication:

<diagram>

