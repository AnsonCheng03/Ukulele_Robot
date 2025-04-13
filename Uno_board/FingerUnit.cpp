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
                moveState = CALIBRATING_SLIDER;
                slider->calibrate();
            }
            break;

        case CALIBRATING_SLIDER:
            slider->update();
            if (slider->isMovementComplete()) {
                moveState = CALIBRATING_FINGERING;
                fingeringMotor->calibrate();
            }
            break;

        case CALIBRATING_FINGERING:
            fingeringMotor->update();
            if (fingeringMotor->isMovementComplete()) {
                moveState = FINGER_IDLE; // Calibration complete
                Serial.println("All motors calibrated.");
            }
            break;

        case FINGER_UP:
            if (rackMotor->isMovementComplete()) {
                slider->move(pendingDistance);
                moveState = FINGER_SLIDE;
            }
            break;

        case FINGER_SLIDE:
            if (slider->isMovementComplete()) {
                rackMotor->down();
                moveState = FINGER_DOWN;
            }
            break;

        case FINGER_DOWN:
            if (rackMotor->isMovementComplete()) {
                fingeringMotor->move();
                moveState = FINGER_PRESS;
            }
            break;

        case FINGER_PRESS:
            if (fingeringMotor->isMovementComplete()) {
                moveState = FINGER_IDLE;
            }
            break;

        default:
            break;
    }
}


bool FingerUnit::isMovementComplete() {
    return slider->isMovementComplete() && 
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

Slider* FingerUnit::getSlider() {
    return slider;
}

RackMotor* FingerUnit::getRackMotor() {
    return rackMotor;
}

FingeringMotor* FingerUnit::getFingeringMotor() {
    return fingeringMotor;
}
