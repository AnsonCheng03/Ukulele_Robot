#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "Device.h"
#include "JobQueue.h"

class RackMotor : public Device {
public:
    RackMotor(int startPin, int directionPin, int speedPin, int boardAddress);
    int getSpeedPin();

    void setup();
    void calibrate();
    void move(int positionMm);
    void up();
    void down();
};

#endif // RACKMOTOR_H
