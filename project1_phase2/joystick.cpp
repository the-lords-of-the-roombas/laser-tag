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

void joystick_read(state* s)
{
  int x = analogRead(x_pin);
  int y = analogRead(y_pin);

  // TODO: scale x and y to range 0 - 100;

  bool sw = !digitalRead(sw_pin);

  s->x = x;
  s->y = y;
  s->sw = sw;
}
