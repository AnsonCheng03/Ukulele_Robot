#include "FingeringMotor.h"

FingeringMotor::FingeringMotor(int startPin, int directionPin, int speedPin, int motorID)
    : Device(startPin, directionPin, speedPin, motorID) {}

void FingeringMotor::setup() {
    Device::setup();
    Serial.println("Rack motor setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
    max_distance = 0;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
    setDirection(motorID >= 10 ? LOW : HIGH);
    analogWrite(speedPin, 100);
    startMovement(5);
}

int FingeringMotor::getSpeedPin() {
    return speedPin;
}

void FingeringMotor::calibrate() {
    Serial.println("Calibrating rack motor...");
    setDirection(motorID >= 10 ? LOW : HIGH);
    analogWrite(speedPin, 100);
    startMovement(5);
    Serial.println("Calibration started: Moving rack motor");
    isCalibrated = true;
    Serial.println("Rack motor calibration complete.");
}

void FingeringMotor::move(int positionMm)
{
    if(positionMm > 0) {
        up();
    } else {
        down();
    }
}

void FingeringMotor::start()
{
    digitalWrite(startPin, HIGH);
    Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void FingeringMotor::stop()
{
    digitalWrite(startPin, LOW);
    Serial.println("Start pin " + String(startPin) + " set to LOW");
}

// need integrate with calibration later
void FingeringMotor::up() {
    Serial.println("Rack motor moving up...");
    moveBy(motorID >= 10 ? -2 : 2);
}

void FingeringMotor::down() {
    Serial.println("Rack motor moving down...");
    moveBy(motorID >= 10 ? 2 : -2);
}