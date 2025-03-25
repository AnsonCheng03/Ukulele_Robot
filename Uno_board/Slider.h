#ifndef SLIDER_H
#define SLIDER_H

#include "Device.h"

enum CalibrationPhase {
    CALIBRATION_INIT,
    CALIBRATION_BACK_OFF,
    CALIBRATION_WAIT_1,
    CALIBRATION_SEEK_SENSOR,
    CALIBRATION_DONE
};

class Slider : public Device {
public:
    Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID);
    void setup();
    void calibrate();
    void update();
    void move(int positionMm);
    int getSensorValue();

private:
    int sensorPin;
    CalibrationPhase calibrationPhase;
    unsigned long calibrationPhaseStart;
};

#endif // SLIDER_H
