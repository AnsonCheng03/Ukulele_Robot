#include "Fingering.h"

void rackMotorDown(void* context) {
    RackMotor* rackMotor = static_cast<RackMotor*>(context);
    rackMotor->down();
}

void moveFinger(Slider &slider, RackMotor &rackMotor, int distanceMm) {
    rackMotor.up();
    slider.move(distanceMm);
    enqueueJob(rackMotorDown, &rackMotor); 
}
