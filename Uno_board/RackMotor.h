#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "UpperMotor.h"
#include "JobQueue.h"

class RackMotor : public UpperMotor {
public:
    RackMotor(int startPin, int directionPin, int speedPin, int motorID);
    int getSpeedPin();

    void setup();
    void calibrate();
    void move(int positionMm) override;
    void up();
    void down();
};

#endif // RACKMOTOR_H
