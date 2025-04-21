#ifndef FINGER_GROUP_CONTROLLER_H
#define FINGER_GROUP_CONTROLLER_H

#include "FingerUnit.h"

class FingerGroupController {
public:
    FingerGroupController(FingerUnit* units[4]);

    void setup();
    void calibrate();
    void update();
    void moveFinger(const int distances[4]); 
    bool areAllFingersReady();
    void resetPluckState();

private:
    FingerUnit* fingers[4];
    bool selectedToPluck[4] = {false, false, false, false};
    bool hasPlucked = false;
    bool debugMoveQueued = false;

    void triggerPluckSelected(const bool selected[4], unsigned int pulseMicros = 10000);
};

#endif
