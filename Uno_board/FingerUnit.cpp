#include "FingerUnit.h"

FingerUnit::FingerUnit(Slider* slider, RackMotor* rackMotor, FingeringMotor* fingeringMotor)
    : slider(slider), rackMotor(rackMotor), fingeringMotor(fingeringMotor) {}

void FingerUnit::setup() {
    slider->setup();
    rackMotor->setup();
    fingeringMotor->setup();

    rackMotor->setSlider(slider);
}

void FingerUnit::calibrate() {
    moveState = CALIBRATING_RACK;
    rackMotor->calibrate();
}

void FingerUnit::update() {
    slider->update();
    rackMotor->update();
    fingeringMotor->update();

    switch (moveState) {
        case CALIBRATING_RACK:
            rackMotor->update();
            if (rackMotor->isMovementComplete()) {
                waitStartTime = micros();
                moveState = CALIBRATING_WAIT_AFTER_RACK;
            }
            break;

        case CALIBRATING_WAIT_AFTER_RACK:
            if (micros() - waitStartTime >= fingerWaitDelay) {
                moveState = CALIBRATING_SLIDER;
                slider->calibrate();
            }
            break;

        case CALIBRATING_SLIDER:
            slider->update();
            if (slider->isMovementComplete()) {
                waitStartTime = micros();
                moveState = CALIBRATING_WAIT_AFTER_SLIDER;
            }
            break;

        case CALIBRATING_WAIT_AFTER_SLIDER:
            if (micros() - waitStartTime >= fingerWaitDelay) {
                moveState = CALIBRATING_FINGERING;
                fingeringMotor->calibrate();
            }
            break;

        case CALIBRATING_FINGERING:
            fingeringMotor->update();
            if (fingeringMotor->isMovementComplete()) {
                moveState = FINGER_IDLE; // Calibration complete
                Serial.println("All motors calibrated.");

                // moveFinger(310); // Move finger to initial position
                // 5 40 80 120 155 190 220 250 280 305 
            }
            break;

        case FINGER_UP:
            if (rackMotor->isMovementComplete()) {
                waitStartTime = micros();
                moveState = FINGER_WAIT_AFTER_UP;
            }
            break;

        case FINGER_WAIT_AFTER_UP:
            if (micros() - waitStartTime >= fingerWaitDelay) {
                slider->move(pendingDistance == -1 ? 0 : pendingDistance);
                moveState = FINGER_SLIDE;
            }
            break;

        case FINGER_SLIDE:
            if (slider->isMovementComplete()) {
                waitStartTime = micros();
                moveState = FINGER_WAIT_AFTER_SLIDE;
            }
            break;

        case FINGER_WAIT_AFTER_SLIDE:
            if (micros() - waitStartTime >= fingerWaitDelay) {
                if (pendingDistance < 0) {
                    fingeringMotor->move();
                    moveState = FINGER_PRESS;
                } else {
                    rackMotor->down();
                    moveState = FINGER_DOWN;
                }
            }
            break;

        case FINGER_DOWN:
            if (rackMotor->isMovementComplete()) {
                waitStartTime = micros();
                moveState = FINGER_WAIT_AFTER_DOWN;
            }
            break;

        case FINGER_WAIT_AFTER_DOWN:
            if (micros() - waitStartTime >= fingerWaitDelay) {
                if (skipPluck) {
                    skipPluck = false;
                    moveState = FINGER_IDLE;
                } else {
                    fingeringMotor->move();
                    moveState = FINGER_PRESS;
                }
            }
            break;

        case FINGER_PRESS:
            if (fingeringMotor->isMovementComplete()) {
                moveState = FINGER_IDLE;

                // fingeringMotor->moveForever();
            }
            break;

        default:
            break;
    }
}


bool FingerUnit::isMovementComplete() {
    return moveState == FINGER_IDLE && 
           slider->isMovementComplete() && 
           rackMotor->isMovementComplete() &&
           fingeringMotor->isMovementComplete();
}

void FingerUnit::moveFinger(int distanceMm) {
    if (moveState == FINGER_IDLE) {
        pendingDistance = distanceMm;
        rackMotor->up();
        moveState = FINGER_UP;
    }
}

void FingerUnit::moveFingerWithoutPluck(int distanceMm) {
    skipPluck = true;
    moveFinger(distanceMm);  // use original FSM flow
}


Slider* FingerUnit::getSlider() {
    return slider;
}

RackMotor* FingerUnit::getRackMotor() {
    return rackMotor;
}

FingeringMotor* FingerUnit::getFingeringMotor() {
    return fingeringMotor;
}
