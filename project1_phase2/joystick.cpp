#include "joystick.h"
#include "Arduino.h"

static int x_pin = 0; // analog
static int y_pin = 1; // analog
static int sw_pin = 5; // digital

void joystick_init(state* s)
{
  pinMode(sw_pin, INPUT);
  digitalWrite(sw_pin, HIGH);
}

void joystick_read(void* data)
{
  state *s = (state*)data;

  int x = analogRead(x_pin);
  int y = analogRead(y_pin);
  bool sw = digitalRead(sw_pin);


  // FIXME: joystick original range may be smaller:
  x = map(x, 0, 350, -100, 100);
  y = map(y, 0, 350, -100, 100);

  s->x = x;
  s->y = y;
  s->sw = !sw;
}
