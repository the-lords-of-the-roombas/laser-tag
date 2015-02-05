#include "shoot.h"
#include "packet.h"
#include "cops_and_robbers.h"
#include "radio.h"
#include "Arduino.h"

void shoot_init(state* s)
{
  s->shoot.joystick_was_pressed = false;
}

static void shoot_transmit(state *s)
{
  s->tx_packet.type = IR_COMMAND;
  s->tx_packet.payload.ir_command.ir_command = SEND_BYTE;
  s->tx_packet.payload.ir_command.ir_data = 'A';

  int result = Radio_Transmit(&s->tx_packet, RADIO_WAIT_FOR_TX);
}

void shoot(void* data)
{
  state *s = (state*) data;

  if (s->joystick.pressed && !s->shoot.joystick_was_pressed)
  {
    shoot_transmit(s);
  }

  s->shoot.joystick_was_pressed = s->joystick.pressed;
}
