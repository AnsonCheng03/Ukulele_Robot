#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "UpperMotor.h"
#include "JobQueue.h"
#include "CalibrationPhase.h"

class RackMotor : public UpperMotor {
public:
    RackMotor(int startPin, int directionPin, int speedPin, int sensorPin, int motorID);
    int getSpeedPin();

    void setup();
    void calibrate() override;
    void update() override;
    void move(int positionMm) override;
    void up();
    void down();

private:
    int sensorPin;
    CalibrationPhase calibrationPhase;
    unsigned long calibrationPhaseStart;
    int getSensorValue();
};

#endif // RACKMOTOR_H



