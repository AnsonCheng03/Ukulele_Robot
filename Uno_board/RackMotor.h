#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "Device.h"

class RackMotor : public Device {
public:
    RackMotor(int startPin, int directionPin, int speedPin);
    void calibrate();
};

#endif // RACKMOTOR_H
