#include <Arduino.h>
#include "Actuator.h"

Actuator::Actuator(int componentId, String name, int pin, int minValue, int maxValue) 
: Component(componentId, name, pin) {
  this->minValue = minValue;
  this->maxValue = maxValue;

  pinMode(pin, OUTPUT);
}

int Actuator::output(int value, String kind) {
  if (lastValue == value) return 0;

  lastValue = value;

  if (kind == "digital") {
    value == 255 ? digitalWrite(pin, HIGH) : digitalWrite(pin, LOW);
    return 1;
  } 
    
  if (kind == "analog") {
    analogWrite(pin, value);
    return 2;
  }

  return -1;
}