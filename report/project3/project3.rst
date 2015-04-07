CSC 460/560: Design and Analysis of Real-Time Systems
=====================================================

Project 3
=========

Authors: Jakob Leben and Darren Prince

Code: https://github.com/the-lords-of-the-roombas/laser-tag/tree/master/project3

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
distance sensor and na IR LED using an Arduino board with an AVR ATMega 2560
microprocessor. The nature of each of these hardware components and the
required interaction with the microprocessor differ greatly, as far as relation
to real time is concerned. A **real-time operating system (RTOS)**, developed in
project 2 and extended in this project, was of great assistance in implementing
functional, robust, maintainable software.

Our software organization is inspired by the 3-layer architecture as defined by
Firby (Adaptive execution in complex dynamic worlds, 1990), and
further explored by Gat (On Three-Layer Architectures, 1998).
On the lowest level is the controller

