#ifndef FINGER_UNIT_H
#define FINGER_UNIT_H

#include "Slider.h"
#include "RackMotor.h"
#include "FingeringMotor.h"

class FingerUnit {
public:
    FingerUnit(Slider* slider, RackMotor* rackMotor, FingeringMotor* fingeringMotor);

    void setup();
    void calibrate();
    void update();
    bool isMovementComplete();
    void moveFingerWithoutPluck(int distanceMm);

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
    bool skipPluck = false;
    int pendingDistance = 0;
    unsigned long waitStartTime = 0;
    const unsigned long fingerWaitDelay = 200000UL; // 200ms -> 200,000µs
};

#endif // FINGER_UNIT_H
