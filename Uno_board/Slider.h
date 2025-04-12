#ifndef SLIDER_H
#define SLIDER_H

#include "UpperMotor.h"
#include "FSMState.h"
#include "MotorConfig.h"

class Slider : public UpperMotor {
public:
    Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID, const UpperMotorConfig& config);
    void setup();
    void calibrate();
    void update();
    void move(int positionMm) override;
    int getSensorValue();
    int getCurrentPosition();

private:
    int sensorPin;
    CalibrationPhase calibrationPhase;
    unsigned long calibrationPhaseStart;
};

#endif // SLIDER_H
