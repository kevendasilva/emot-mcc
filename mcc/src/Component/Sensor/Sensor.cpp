#include <Arduino.h>
#include "Sensor.h"

Sensor::Sensor(int componentId, String name, int pin) 
: Component(componentId, name, pin) {
  pinMode(pin, INPUT);
}

int Sensor::read() {
  return analogRead(pin);
}