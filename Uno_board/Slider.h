#ifndef SLIDER_H
#define SLIDER_H

#include "Device.h"

class Slider : public Device {
public:
    Slider(int startPin, int directionPin, int speedPin, int sensorPin);
    void setup();
    void calibrate();
    void update();
    void move(int positionMm);

private:
    int sensorPin;
    bool isCalibrating;
};

#endif // SLIDER_H
