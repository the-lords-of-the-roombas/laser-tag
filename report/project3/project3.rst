CSC 460/560: Design and Analysis of Real-Time Systems
=====================================================

Project 3
=========

Team: **Jakob Leben and Darren Prince**

In a group with 3 other teams:

  - Saleh Almuqbil and Jorge Conde
  - Jeff ten Have and Colin Knowles
  - Nathan Vandenberg

Code: https://github.com/the-lords-of-the-roombas/laser-tag/tree/master/project3

`Demo video - Robot <https://vimeo.com/124444439>`__:

.. raw:: html

  <iframe src="https://player.vimeo.com/video/124444439" width="600" height="337" frameborder="0" webkitallowfullscreen mozallowfullscreen allowfullscreen></iframe>

`Demo video - Gameplay <https://www.youtube.com/watch?v=Xwt1jkmxIGU>`__:

.. raw:: html

  <iframe width="600" height="337" src="https://www.youtube.com/embed/Xwt1jkmxIGU" frameborder="0" allowfullscreen></iframe>

Overview
********

Game Rules
----------

Some teams decided to implement completely autonomous robots (no human control).
Since this is quite a big challenge in itself, we decided to keep the rules
of the game rather simple: The goal of each robot is to shoot any other robot as
many times as possible and to get shot as few times as possible within the time
limit of the game. A shot by a robot is only valid when it occurs at least 5
seconds after its previous shot of the same target robot.

Robot Components
----------------

Each robot extends the iRobot Create 2 platform (at least)
with the following components:

- Wireless radio transciever.
- Infra-red (IR) LED emitter, serving as a gun.
- Ultra-sound distance sensor, used to detect other robots (sonar)
- An object installed on top of the robot to facilitate detection by sonar,
  made of cardboard or similar material, of dimensions 40 x 40 x 40 cm.

Playground
----------

The playground is delimited by walls about 5 cm high - just enough to be
detected by the iRobot's built-in close-proximity IR sensors. The robots
can use these sensors to avoid bumping into walls, and to navigate within
the bounds of the playground.

Detection of Other Robots
-------------------------

A robot detects another robot using the sonar. The angle of the sonar
with respect to the floor is large enough for the sonar to not detect
playground walls. However, it detects the cardboard objects placed on top
of other robots exactly for this purpose. The playground walls must be
a couple of meters away from any objects surrounding the playground
(room walls, etc.) A sonar reading of a distance less than the closest object
surrounding the playground thus unambiguously indicates the distance to the
closest competing robot.

Shooting
--------

Each robot is assigned a unique byte of data that identifies it - its ID. A shot
occurs when one robot transmits its ID to another robot using IR communication.
The shooter emits the data using its attached IR emitter, and the target
receives the data using the iRobot's built-in omni-directional IR receiver.

The communication protocol is dictated by the iRobot's built-in receiver.
The data is encoded using a sequence of the following two states: 38 kHz
modulation of IR emission (ON), or no IR emission (OFF).
Transmission of each bit of a byte takes 4 ms. A 1 bit is transmitted as
3 ms ON, followed by 1 ms OFF. A 0 bit is transmitted as 1 ms ON, followed by
3 ms OFF. The end of a bytes is marked by 4 ms OFF.

Coordination by Base Station
----------------------------

A base station is used to coordinate the behavior of the robots and the
gameplay in the following ways:

- Prevents interference of sonars.
- Prevents interference of wireless communication.
- Keeps track of scores.
- Enforces the rule of 5 seconds minimum time between successive shots
  (see game rules above).

Sonars of different robots would interfere if used simultaneously.
The technical specification of ultra-sound sensors suggests at least 50 ms
between successive triggers. The base station sends a wireless packet every
50 ms, each time to a different robot, indicating that the robot is allowed
to trigger the sonar. The robot must trigger the sonar as soon as possible
after the reception of this packet.

Robots must report when they are shot to the base station using a wireless
packet containing the shooter's and target's ID. Simultaneous communication
of multiple robots with the base station would also interfere. The robots
must remember their last shooter, and report it to the base station exclusively
and immediately after receiving a sonar-trigger packet.

The base station counts the number of times each robot has successfully shot
another one, and the number of times it has been shot, based on the wireless
packets received from the robots. The base station will disregard shots
between the same shooter and target with less than 5 seconds intermediate time.

Robot System Design
*******************

In order to implement the required behavior for the game, the iRobot Create 2
platform had to be integrated with a wireless transciever, an ultra-sound
distance sensor and na IR LED using an **Arduino** board with an **AVR ATMega 2560**
microprocessor. The nature of each of these hardware components and the
required interaction with the microprocessor differ greatly, as far as relation
to real time is concerned. A **real-time operating system (RTOS)**, developed in
project 2 and extended in this project, was of great assistance in implementing
functional, robust, maintainable software.

Our software organization is inspired by the **3-layer architecture** as defined
by Firby (Adaptive execution in complex dynamic worlds, 1990), and further
explored by Gat (On Three-Layer Architectures, 1998). The 3 layers are:
controller, sequencer, and planner. They perform increasingly more complex and
time-consuming, but also less time-critical computation. This paradigm is
adapted to our application; it requires little complex planning, so the most
prominent layers are the controller and the sequencer. We call the highest layer
"coordinator".

The coordinator, sequencer and controller are the core subsystems which
in combination implement the behavior of the system in response to the world
and communication with the base station and other robots. In addition,
the sonar and the gun are implemented as fairly self-contained subsystems
with a minimal interface, which allows to easily test them individually,
and which was useful during development while experimenting with their
placement within the entire system.

.. image:: architecture-out.svg

Gun
---

The gun subsystem uses two microcontroller's hardware timers: one is used to generate the
38 kHz PWM signal which is output to the IR LED;
another is used to generate interrupts every 1 ms, and
enable and disable the PWM output according to the IR communication protocol.

The byte transmission is triggered by writing to a shared memory using
the thread-safe ``gun::send`` function from any task.

Sonar
-----

The ultra-sound distance sensor emits ultra-sound pulses when a 10 microseconds
HIGH pulse is input on its signal pin. It responds by outputting a pulse on the
same pin with the width equal to the duration it takes for the ultra-sound echo
to return.

The sonar uses the **input-capture** capability of a microcontroller's hardware timer
to precisely measure the width of the sensor's output pulse. After outputting
a trigger pulse on the sensor signal pin, the timer is configured to
generate an interrupt on a raising edge. On this interrupt, the timer is
reconfigured to interrupt on a falling edge. Finally, the difference between
the times at the two interrupts is measured and published over a service internal
to the sonar subsystem.

The sonar is triggered by publishing on its public **sonar-request service**,
and it publishes the measured echo time over its public **sonar-response service**.

Its timer and sensor input/output coordination code runs as a **system task** in
order to be able to progress from triggering the ultra-sound sensor to listening
for its respons as quickly as possible. However, the work done between waiting
on services is minimal, and so is its interference with other time-critical
tasks.

The sonar subsystem also provides a function to convert the measurement from
time units to distance units.


Controller
----------

The controller handles the most time-critical
tasks: acquisition of sensor data from the robot and control of the robot's
motion.

The operation of the controller consists of a set of **primitive behaviors**,
each being a purely functional mapping between inputs (sensors and behavior
parameters) and outputs (robot motion control). These behaviors are mostly
memory-less, except for the usage of simple time-domain filters; it is important
to respond immediately to critical situations such as proximity of obstacles,
and prevent historical sensory data to affect critical reactions.

At any moment, the controller may be executing one of the behaviors, selected
by the sequencer which also provides parameters:

Wait
  This is the default behavior: the robot stands still.

Go
  The robot keeps moving indefinitely in the specified direction and with
  the specified speed. The "forward" direction allows setting desired radius
  of motion. The "leftward" and "rightward" directions cause the robot to
  turn in place regardless of the radius. The "backward" direction is
  not allowed, and will cause the robot to stand still.

Move
  The robot moves straight forward or turns in place, while decreasing the
  initially specified distance to goal by the distance travelled
  until it reaches 0. The remaining distance is provided as output,
  which allows the sequencer to change behavior upon completion.

Chase
  The robot moves straight forward with desired speed and assumes that
  any encountered obstacle is another robot. When being in very
  close proximity to an obstacle, it turns so as to face the obstacle directly,
  in preparation for a shot.

The controller runs as a periodic task with a period of 25 ms. At the beginning
of each period it acquires the current **sensor data** from the iRobot over a
**serial interface**, which was measured to take approximately 6 ms,
with insignificant deviation. This is combined
with the input data provided by the **sequencer** via **shared memory**.
Shared memory approach was chosen because waiting on a service is inappropriate
for a time-critical periodic task (and is disallowed by the RTOS).
These source of input together form the **input variables**.

The **output variables** are computed from the input variables according to the
currently active behavior (provided itself as one of the input variables).
Some outputs
(velocity and radius) are sent to the iRobot via the serial interface to
**control movement**, which takes a fraction of a millisecond on the part of the
periodic task. Some output variables are **fed back** into input variables: for
example, the remaining distance towards goal used in the *Move* behavior
overwrites the initial distance specified by the sequencer at the onset
of the behavior. Other output variables are provided to the sequencer via
**shared memory**. In addition, the controller publishes the last received
IR byte (as reported by iRobot's built-in IR receiver) over a **shot service**.

There is an additional **critical behavior** which **overrides** any behavior
selected by the sequencer: the response to iRobot **bump** sensors. When a bump
is detected, the robot will move backward a couple centimeters, and then
suppress any forward motion for 1.5 seconds. The sequencer is notified of the
bump as part of the output variables in shared memory.



Sequencer
---------

The sequencer has a set of own higher-level behaviors.
These are **goal-oriented** behaviors - they are switched either when
the goal of the current behavior is achieved, or it is currently deemed
unachievable. Each sequencer behavior dictates a sequence of controller
behaviors and associated parameters. The progression through the sequence,
as well as the decision to switch the sequencer behavior is determined by
the output of the controller, as well as the input to the sequencer from
other subsystems and the base station.
The sequencer also triggers the gun. The completion time of this action is fairly
deterministic, so it can be included as a step in a behavioral sequence.

At any moment, the sequencer executes one of these behaviors:

Seek Straight
  Randomly alternating left and right curves are performed.
  The goal is to scan a large portion of the playground using the
  sonar. When the sonar detects another robot, the sequencer switches to
  the Chase behavior. Alternatively, when an obstacle other than a robot
  is detected, the Seek Left or Seek Right behavior is selected so as to avoid
  the obstacle.
  If a bump is detected, the Critical Turn is performed.

Seek Left/Right
  The robot is turned by 90 degrees to
  the left or right so as to avoid an obstacle. When the turn is complete,
  the Seek Straight behavior is selected.

Chase
  The controller's Chase behavior is used with maximum speed to approach
  the robot detected by the sonar. If the target robot disappears from the
  sonar's sight, its relative direction of movement is guessed: a turn is
  performed in the same direction as the last turn made by the Seek Left or Right
  behavior. If the target is still not seen after the turn, a larger turn in the
  opposite direction is performed. If the target is still invisible, the Seek
  Straight behavior is selected. However, if the target appears close according to
  the sonar, or when the robot is facing it directly as reported by the
  controller, the Shoot behavior is selected.

Shoot
  Three shots are performed at three slightly different angles. Finally,
  the Critical Turn behavior is selected, to turn away from the (hopefully)
  shot target.

Critical Turn
  The robot is turned by 180 degrees and then the Seek Straight behavior is
  selected.


The Seek Straight/Left/Right and Chase behaviors also monitor the controller
output for **bumps**, and will unconditionally switch to the Critical Turn
behavior when a bump is detected.

The sequencer runs as a **round-robin task**. This allows the more time-critical
controller to access shared memory without disabling interrupts, due to
higher priority. It also allows the sequencer to simply busy-wait until
behavior-switching conditions occur.

The sequencer receives the **sonar** distance measurements from the coordinator
via shared memory. The **gun** is triggered by calling the gun's thread-safe
"send" function, and waiting for a pre-determined amount of time for the shot
to complete.

Coordinator
-----------

The coordinator handles tasks of which completion time is longer and less
predictable. This includes triggering and waiting for response from the sonar,
sending wireless data and processing incoming wireless data.

The coordinator runs as a **round-robin task**.
Its entire operation of the coordinator consists of **reponses** to events on
a number of **services**:

Radio service response
  The radio interrupt handler publishes to the service when new radio packets are received.

  According to the inter-system protocol, the coordinator triggers the sonar
  immediately in reponse to the **sonar-trigger packet** received from the base
  station. It does so by publishing to the **sonar-request service**.

  In addition, the last received shot is reported by sending a **shot packet**
  to the base station

Shot service reponse
  The controller publishes to this service the byte received from the shooter's
  gun. The coordinator only stores this byte so that it can be used later
  in reponse to the sonar-trigger wireless packet.

Sonar-reply service reponse
  The sonar publishes the measured distance to the service. The coordinator
  communicates the latest value to the sequencer via shared memory.

For the purpose of the coordinator, the RTOS was extended with the ability
to **wait for multiple services simultaneously** (see section Extensions to RTOS
below).

Base Station
************

Our base station implementation was shared by the entire group of 4 teams:

https://github.com/the-lords-of-the-roombas/laser-tag/blob/master/project3/base/base.cpp

It is fairly simple and did not require the RTOS. It repeatedly reads and
handles all radio packets received from any robot. Meanwhile, it sends
a sonar-trigger packet every 50 ms to a different robot. It remembers the
last time a shot-packet was received for each shooter. When a shot-packet
arrives at least 5 seconds after the last time for the same shooter, it
increases the count of shots given by the shooter and shots received by the
target. This information is printed through the Arduino's serial interface
connected to its USB port, in format::

    <robot ID>: <shots given>/<shots received> ...

Here is a sample output, in form of a screenshot from the
Arduino IDE's serial monitor:

.. image:: game-status.png


Extensions to RTOS
******************

We changed the API and semantics of services so as to better fit the
purpose of this project. The ``Service_Subscribe`` only returns a
``ServiceSubscription`` object which is used by the calling task to
wait for and receive values over the service. We introduced the
``Service_Receive`` function which is used with a ``ServiceSubscription`` object
to read the last published and unread value over the service, or wait for
the next unread value if all have been read. This allows reception of
values even after they were published without the subscriber being blocked
and waiting for the publish event.

For the purpose of the coordinator layer, we extended the RTOS with
the ``Service_Receive_Mux`` function which allows a task to wait on
multiple services simultaneously.

Issues
******

The available sensors were a great limitation to implementation of successful
autonomous robots. Most significantly, the sonar which only detects presence
of other robots in a single direction makes tracking their movement very
impractical and, at best, hardly useful. We have alleviated this by
using a small playground, so that it is more probable for robots to see each
other.

The wide angle of sonar required it to be tilted up signficantly in order not
to confuse playground walls for other robots, which on the other hand
reduced visibility of the robots. The cardboard mounted on top of
the robots did not help much.

We did not get very suitable material to construct playground walls just high
enough. The walls use in the demo video are slightly too low to be detected by
the close-proximity IR sensors, so the robots often bump into walls and must
rely on the poorer 2-dimensional bump sensors rather than 6-dimensional IR
sensors for wall avoidance.

We used parts of the Arduino library, most notably the HardwareSerial library
for communication with the iRobot platform. The Arduino library initialization
routine affects hardware timer configuration, so we called it before the
RTOS initialization routine, so that the latter could override the
configuration. However, the Arduino initialization also globally enables interrupts,
but the RTOS initailization - as was initially provided to us - did not
explicitly disable them. Before this was detected, it caused the RTOS to run all
the time with interrupts enabled, causing the most unexpected behaviors and many
terrible hours of debugging.

Moreover, the radio library that was provided to us is completely unsuitable
for multi-tasking - it accesses shared hardware registers without disabling
interrupts. On quick code inspection, it also does not seem that
the radio interrupt handling code is quite safe for concurrency with the radio
library routines. We solved these issues by explicitly disabling interrupts
whenever calling a radio library routine. However, this is rather suboptimal,
because the routines internally waste some time waiting for hardware, during
which time no interrupt can be handled. The greatest potentail timing issue is
side-stepped by not waiting for the ACK response when transmitting packets.

Conclusion
**********

Despite great limitations imposed by the available hardware, we have managed to
create a robot that can play our designed game autonomously. Translating the
theory of the 3-layer architecture into practice was an intriguing and
satisfying challenge. It contributed to a robot which is highly reponsive to
critical situations, but also able of somewhat sophisticated higher-level
behavioral patterns. Inspired by the desire for coherence and well-structured
code, as required by this particular application, we creatively adapted and
extended the RTOS. We successfully coordinated the inter-system aspects of the
project with other teams in the group. Unfortunately however, only one other
team completed their robot, while two other teams needed assistance in order to
be included in the game. Nevertheless, through collaboration, we achieved a
functional game of fully-autonomous robots.
