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
