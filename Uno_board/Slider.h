#ifndef SLIDER_H
#define SLIDER_H

#include "Device.h"

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
    bool isCalibrating;
};

#endif // SLIDER_H
