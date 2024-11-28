#include "RackMotor.h"

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
    currentPosition = 0;
    Serial.println("Rack motor calibration complete.");
}
