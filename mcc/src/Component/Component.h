#ifndef COMPONENT_H
#define COMPONENT_H

class Component {
  protected:
    int componentId;
    String name;
    int pin;

  public:
    Component(int componentId, String name, int pin);
    virtual int getComponentId();
    virtual String getName();
    virtual int getPin();
  };

#endif