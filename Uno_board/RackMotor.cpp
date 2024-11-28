#include "RackMotor.h"
#define RACK_CALIBRATION_START_POSITION 10

RackMotor::RackMotor(int startPin, int directionPin, int speedPin)
    : Device(startPin, directionPin, speedPin) {}

void RackMotor::calibrate() {
    Serial.println("Calibrating rack motor...");
    setDirection(HIGH);
    analogWrite(speedPin, 500);
    startMovement(5);
    Serial.println("Calibration started: Moving rack motor");
    // Assume calibration is done successfully for now
    isCalibrated = true;
    currentPosition = RACK_CALIBRATION_START_POSITION;
    Serial.println("Rack motor calibration complete.");
}

// need integrate with calibration later
void RackMotor::up() {
    Serial.println("Rack motor moving up...");
    setDirection(HIGH);
    analogWrite(speedPin, 500);
    startMovement(5);
    Serial.println("Rack motor moving up for 0.5s");
}

void RackMotor::down() {
    Serial.println("Rack motor moving down...");
    setDirection(LOW);
    analogWrite(speedPin, 500);
    startMovement(5);
    Serial.println("Rack motor moving down for 0.5s");
}