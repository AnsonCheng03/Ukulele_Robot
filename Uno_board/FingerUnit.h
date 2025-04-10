#ifndef FINGER_UNIT_H
#define FINGER_UNIT_H

#include "Slider.h"
#include "RackMotor.h"
#include "FingeringMotor.h"

enum FingerMoveState {
    FINGER_IDLE,
    FINGER_UP,
    FINGER_SLIDE,
    FINGER_DOWN,
    FINGER_PRESS,
    CALIBRATING_SLIDER,
    CALIBRATING_RACK,
    CALIBRATING_FINGERING
};

class FingerUnit {
public:
    FingerUnit(Slider* slider, RackMotor* rackMotor, FingeringMotor* fingeringMotor);

    void setup();
    void calibrate();
    void update();
    bool isMovementComplete();

    // Combined motion
    void moveFinger(int distanceMm);

    // Direct access if needed
    Slider* getSlider();
    RackMotor* getRackMotor();
    FingeringMotor* getFingeringMotor();

private:
    Slider* slider;
    RackMotor* rackMotor;
    FingeringMotor* fingeringMotor;

    FingerMoveState moveState = FINGER_IDLE;
    int pendingDistance = 0;
};

#endif // FINGER_UNIT_H
