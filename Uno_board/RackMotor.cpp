#include "RackMotor.h"

RackMotor::RackMotor(int startPin, int directionPin, int speedPin, int boardAddress)
    : Device(startPin, directionPin, speedPin, boardAddress) {}

void RackMotor::setup() {
    Device::setup();
    Serial.println("Rack motor setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
    max_distance = 0;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
    setDirection(boardAddress >= 10 ? LOW : HIGH);
    analogWrite(speedPin, 100);
    startMovement(5);
}

int RackMotor::getSpeedPin() {
    return speedPin;
}

void calibrateMotorDown(void* context) {
    RackMotor* rackMotor = static_cast<RackMotor*>(context);
    rackMotor->moveBy(rackMotor->getBoardAddress() >= 10 ? 5 : -5);
}

void RackMotor::calibrate() {
    Serial.println("Calibrating rack motor...");
    setDirection(boardAddress >= 10 ? LOW : HIGH);
    analogWrite(speedPin, 100);
    startMovement(5);
    Serial.println("Calibration started: Moving rack motor");
    isCalibrated = true;
    enqueueJob(calibrateMotorDown, this);
    Serial.println("Rack motor calibration complete.");
}

void RackMotor::move(int positionMm)
{
    if(positionMm > 0) {
        up();
    } else {
        down();
    }
}

// need integrate with calibration later
void RackMotor::up() {
    Serial.println("Rack motor moving up...");
    moveBy(boardAddress >= 10 ? -2 : 2);
}

void RackMotor::down() {
    Serial.println("Rack motor moving down...");
    moveBy(boardAddress >= 10 ? 2 : -2);
}