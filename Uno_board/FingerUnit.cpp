#include "FingerUnit.h"
#include "JobQueue.h"

FingerUnit::FingerUnit(Slider* slider, RackMotor* rackMotor, FingeringMotor* fingeringMotor)
    : slider(slider), rackMotor(rackMotor), fingeringMotor(fingeringMotor) {}

void FingerUnit::setup() {
    slider->setup();
    rackMotor->setup();
    fingeringMotor->setup();
}

void FingerUnit::calibrate() {
    slider->calibrate();
    rackMotor->calibrate();
    fingeringMotor->calibrate();
}

void FingerUnit::update() {
    slider->update();
    rackMotor->update();
    fingeringMotor->update();
}

bool FingerUnit::isMovementComplete() {
    return slider->isMovementComplete() && 
           rackMotor->isMovementComplete() &&
           fingeringMotor->isMovementComplete();
}

void FingerUnit::moveFinger(int distanceMm) {
    rackMotor->up();
    if (distanceMm >= 0) {
        slider->move(distanceMm);
        enqueueJob([](void* ctx) {
            static_cast<RackMotor*>(ctx)->down();
        }, rackMotor);
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
