#include "FingerGroupController.h"

FingerGroupController::FingerGroupController(FingerUnit* units[4]) {
    for (int i = 0; i < 4; ++i) {
        fingers[i] = units[i];
    }
}

void FingerGroupController::setup() {
    for (int i = 0; i < 4; ++i) {
        fingers[i]->setup();
    }
}

void FingerGroupController::calibrate() {
    for (int i = 0; i < 4; ++i) {
        fingers[i]->calibrate();
    }
}

void FingerGroupController::update() {
    for (int i = 0; i < 4; ++i) {
        fingers[i]->update();
    }

    if (!hasPlucked && areAllFingersReady()) {
        triggerPluckSelected(selectedToPluck);
    }

    if (!debugMoveQueued && areAllFingersReady()) {
        int debugDistances[4] = {120, 350, 10, 230};
        moveFinger(debugDistances);
        debugMoveQueued = true;
    }
}

bool FingerGroupController::areAllFingersReady() {
    for (int i = 0; i < 4; ++i) {
        bool fingerReady = fingers[i]->isMovementComplete();
        if (!fingerReady) {
            return false;
        }
    }
    return true;
}

void FingerGroupController::moveFinger(const int distances[4]) {
    for (int i = 0; i < 4; ++i) {
        selectedToPluck[i] = false;  // reset
        if (distances[i] >= 0) {
            fingers[i]->moveFingerWithoutPluck(distances[i]);
            selectedToPluck[i] = true;
        }
    }

    resetPluckState();
}


void FingerGroupController::triggerPluckSelected(const bool selected[4], unsigned int pulseMicros) {
    if (hasPlucked) return;

    Serial.println("Selected fingers ready. Triggering pluck...");

    for (int i = 0; i < 4; ++i) {
        if (selected[i]) {
            fingers[i]->getFingeringMotor()->start();
        }
    }

    delayMicroseconds(pulseMicros);

    for (int i = 0; i < 4; ++i) {
        if (selected[i]) {
            fingers[i]->getFingeringMotor()->stop();
        }
    }

    hasPlucked = true;
}

void FingerGroupController::resetPluckState() {
    hasPlucked = false;
}

void FingerGroupController::controlSingle(int index, int target, int direction, int speed, unsigned long duration) {
    if (target == 0 || target == 1)
        fingers[index]->getSlider()->control(direction, speed, duration);
    if (target == 0 || target == 2)
        fingers[index]->getRackMotor()->control(direction, speed, duration);
    if (target == 0 || target == 3)
        fingers[index]->getFingeringMotor()->control(direction, speed, duration);
}

void FingerGroupController::calibrateSingle(int index, int target) {
    if (target == 0 || target == 1)
        fingers[index]->getSlider()->calibrate();
    if (target == 0 || target == 2)
        fingers[index]->getRackMotor()->calibrate();
    if (target == 0 || target == 3)
        fingers[index]->getFingeringMotor()->calibrate();
}

void FingerGroupController::moveSingle(int index, int target, int distance) {
    if (target == 0) {
        int moveArray[4] = {-1, -1, -1, -1};
        moveArray[index] = distance;
        moveFinger(moveArray);
    } else if (target == 1) {
        fingers[index]->getSlider()->move(distance);
    } else if (target == 2) {
        fingers[index]->getRackMotor()->move(distance);
    } else if (target == 3) {
        fingers[index]->getFingeringMotor()->move();
    }
}

void FingerGroupController::moveBySingle(int index, int target, int position) {
    if (target == 0 || target == 1)
        fingers[index]->getSlider()->moveBy(position);
    if (target == 0 || target == 2)
        fingers[index]->getRackMotor()->moveBy(position);
    if (target == 0 || target == 3)
        fingers[index]->getFingeringMotor()->moveBy(position);
}

void FingerGroupController::controlAll(int target, int direction, int speed, unsigned long duration) {
    for (int i = 0; i < 4; ++i) {
        controlSingle(i, target, direction, speed, duration);
    }
}

void FingerGroupController::calibrateAll(int target) {
    for (int i = 0; i < 4; ++i) {
        calibrateSingle(i, target);
    }
}

void FingerGroupController::moveAll(int target, int distance) {
    if (target == 0) {
        int moves[4] = {distance, distance, distance, distance};
        moveFinger(moves);
    } else {
        for (int i = 0; i < 4; ++i) {
            moveSingle(i, target, distance);
        }
    }
}

void FingerGroupController::moveByAll(int target, int position) {
    for (int i = 0; i < 4; ++i) {
        moveBySingle(i, target, position);
    }
}
