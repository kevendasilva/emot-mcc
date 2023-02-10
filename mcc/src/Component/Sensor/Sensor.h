#ifndef SENSOR_H
#define SENSOR_H

#include "../Component.h"

class Sensor : public Component {
  public:
    int read();
    Sensor(int componentId, String name, int pin);
};

#endif