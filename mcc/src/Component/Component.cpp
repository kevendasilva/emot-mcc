#include <Arduino.h>
#include "Component.h"

Component::Component(int componentId, String name, int pin) {
  this->componentId = componentId;
  this->name = name;
  this->pin = pin;
}

int Component::getComponentId() {
  return componentId;
}

String Component::getName() {
  return name;
}

int Component::getPin() {
  return pin;
}
