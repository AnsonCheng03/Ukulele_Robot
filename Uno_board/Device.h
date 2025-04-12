#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

class Device {
public:
    virtual void setup() = 0;
    virtual void control(int direction, int speedHz, int durationTenths) = 0;
    virtual void calibrate() = 0;
    virtual void moveBy(int positionMm, bool reverse = false) = 0;
    virtual void update() = 0;
    virtual bool isMovementComplete() = 0;
};

#endif // DEVICE_H