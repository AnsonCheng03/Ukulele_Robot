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

    void controlSingle(int index, int target, int direction, int speed, unsigned long duration);
    void calibrateSingle(int index, int target);
    void moveSingle(int index, int target, int distance);
    void moveBySingle(int index, int target, int position);
    void controlAll(int target, int direction, int speed, unsigned long duration);
    void calibrateAll(int target);
    void moveAll(int target, int distance);
    void moveByAll(int target, int position);

private:
    FingerUnit* fingers[4];
    bool selectedToPluck[4] = {false, false, false, false};
    bool hasPlucked = false;
    bool debugMoveQueued = false;

    void triggerPluckSelected(const bool selected[4], unsigned int pulseMicros = 10000);
};

#endif
