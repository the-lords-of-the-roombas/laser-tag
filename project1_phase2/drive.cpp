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
  Serial.println("drive");
  Serial.println(radius);
  Serial.print("speed: ");
  Serial.println(speed);
#endif

  // send to radio

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
}
