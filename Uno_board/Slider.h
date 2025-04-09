#ifndef SLIDER_H
#define SLIDER_H

#include "UpperMotor.h"
#include "CalibrationPhase.h"

class Slider : public UpperMotor {
public:
    Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID);
    void setup();
    void calibrate();
    void update();
    void move(int positionMm) override;
    int getSensorValue();

private:
    int sensorPin;
    CalibrationPhase calibrationPhase;
    unsigned long calibrationPhaseStart;
};

#endif // SLIDER_H
