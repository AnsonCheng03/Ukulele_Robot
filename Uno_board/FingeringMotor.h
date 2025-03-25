#ifndef FINGERINGMOTOR_H
#define FINGERINGMOTOR_H

#include "Device.h"
#include "JobQueue.h"

class FingeringMotor : public Device {
public:
    FingeringMotor(int startPin, int directionPin, int speedPin, int motorID);
    int getSpeedPin();

    void setup();
    void calibrate();
    void move(int positionMm);
    void start();
    void stop();
    void up();
    void down();
};

#endif // FINGERINGMOTOR_H
