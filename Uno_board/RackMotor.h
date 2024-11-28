#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "Device.h"

class RackMotor : public Device {
public:
    RackMotor(int startPin, int directionPin, int speedPin);
    void calibrate();
    void up();
    void down();
};

#endif // RACKMOTOR_H
