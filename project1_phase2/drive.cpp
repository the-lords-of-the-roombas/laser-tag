#include "drive.h"
#include "radio.h"
#include "utilities.h"
#include "Arduino.h"

void drive_init(state* s)
{
  s->drive.speed = 0;
  s->drive.radius = 0;
}

void drive(void * data)
{
  state * s = (state*) data;

  int x = s->joystick.x;
  int y = s->joystick.y;

#if 0
  Serial.print("x: ");
  Serial.println(x);
  Serial.print("y: ");
  Serial.println(y);
#endif

  // radius

  int16_t radius;

  if (x > 20)
    radius = 1;
  else if (x < -20)
    radius = -1;
  else
    radius = 0x8000;

  s->drive.radius = radius;

#if 0
  if (x > -10 && x < 10)
    radius = 0x8000;
  else if (x >= 10)
    radius = map(x, 10,100,-2000,-1);
  else
    radius = map(x, -10,-100,2000,1);
#endif

  // speed

  int16_t speed;

  // If turning, use a small speed
  if (x < -20 || x > 20)
    speed = 100;
  else if (y > 20)
    speed = 200;
  else if (y < -20)
    speed = -200;
  else
    speed = 0;

  s->drive.speed = speed;

#if 0
  int16_t speed = s->speed;


  if (y > 10)
  {
    // assuming 50ms task period
    if (speed < 0)
      speed = 0;
    else
      speed += 2;

    if (speed > 500)
      speed = 500;
  }
  else if (y < 10)
  {
    if (speed > 0)
      speed = 0;
    else
      speed -= 2;

    if (speed < -500)
      speed = -500;
  }

  s->speed = speed;
#endif

#if 1
  //Serial.println("drive");
  //Serial.println(radius);
  //Serial.print("speed: ");
  //Serial.println(speed);
#endif

  // send to radio
#if 1
  s->tx_packet.type = COMMAND;
  s->tx_packet.payload.command.command = 137;
  s->tx_packet.payload.command.num_arg_bytes = 4;
  s->tx_packet.payload.command.arguments[0] = HIGH_BYTE(speed);
  s->tx_packet.payload.command.arguments[1] = LOW_BYTE(speed);
  s->tx_packet.payload.command.arguments[2] = HIGH_BYTE(radius);
  s->tx_packet.payload.command.arguments[3] = LOW_BYTE(radius);

  int result = Radio_Transmit(&s->tx_packet, RADIO_WAIT_FOR_TX);
#if 0
  if (result == RADIO_TX_SUCCESS)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);
#endif
#endif

#if 1
  static bool b = false;
  if (b)
    digitalWrite(13, HIGH);
  else
    digitalWrite(13, LOW);
  b = !b;
#endif
}
