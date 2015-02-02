#include "drive.h"
#include "radio.h"
#include "Arduino.h"

void drive_init(state* s)
{
  s->speed = 0;
}

void drive(void * data)
{
  state * s = (state*) data;

  int x = s->x;
  int y = s->y;

  // radius

  int16_t radius;

  if (x < 10 || x > -10)
    radius = 0x8000;
  else if (x >= 10)
    radius = map(x, 10,100,-2000,-1);
  else
    radius = map(x, -10,-100,2000,1);

  // speed

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

  // send to radio

  s->tx_packet.type = COMMAND;
  s->tx_packet.payload.command.command = 137;
  s->tx_packet.payload.command.num_arg_bytes = 4;
  s->tx_packet.payload.command.arguments[0] = (radius >> 8) & (0xFF);
  s->tx_packet.payload.command.arguments[1] = radius & (0xFF);
  s->tx_packet.payload.command.arguments[2] = (speed >> 8) & (0xFF);
  s->tx_packet.payload.command.arguments[3] = speed & (0xFF);

  Radio_Transmit(&s->tx_packet, RADIO_RETURN_ON_TX);
}
