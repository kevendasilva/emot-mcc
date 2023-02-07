#ifndef ACTUATOR_H
#define ACTUATOR_H

#include "../Component.h"

class Actuator : public Component {
  private:
    int minValue;
    int maxValue;
    int lastValue;

  public:
    Actuator(int componentId, String name, int pin, int minValue, int maxValue);
    int output(int value, String kind);
};

#endif