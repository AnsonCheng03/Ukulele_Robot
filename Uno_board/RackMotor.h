#ifndef RACKMOTOR_H
#define RACKMOTOR_H

#include "UpperMotor.h"
#include "FSMState.h"
#include "MotorConfig.h"
#include "Slider.h"

class RackMotor : public UpperMotor {
public:
    RackMotor(int startPin, int directionPin, int speedPin, int sensorPin, int motorID, const UpperMotorConfig& config);
    int getSpeedPin();

    void setup();
    void calibrate() override;
    void update() override;
    void setSlider(Slider* slider);
    void move(int positionMm) override;
    void up();
    void down();

private:
    int sensorPin;
    CalibrationPhase calibrationPhase;
    unsigned long calibrationPhaseStart;
    int getSensorValue();
    Slider* slider;
    UpperMotorConfig config;
    void moveUntilTouchSensor(bool toward = true);
};

#endif // RACKMOTOR_H



