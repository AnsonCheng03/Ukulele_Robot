#include "Device.h"

#define FIXED_MOVE_SPEED_HZ 1000
#define DISTANCE_TO_DURATION_RATIO 0.01 // Updated ratio: 0.05 seconds per mm
#define MAX_DISTANCE_MM 1000000 // Maximum allowed distance in mm (example large number)

Device::Device(int startPin, int directionPin, int speedPin)
    : startPin(startPin), directionPin(directionPin), speedPin(speedPin), isMoving(false), moveStartMillis(0), moveDuration(0), isCalibrated(false), currentPosition(0) {}

void Device::setup() {
    pinMode(startPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
    pinMode(speedPin, OUTPUT);
    Serial.println("Device setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
}

void Device::control(int direction, int speedHz, int durationTenths) {
    Serial.println("Control device - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
    isCalibrated = false; // Control operation makes the device uncalibrated
    analogWrite(speedPin, speedHz);
    setDirection(direction);
    startMovement(durationTenths);
}

void Device::move(int distanceMm) {
    if (!isCalibrated) {
        Serial.println("Device not calibrated. Cannot move.");
        return;
    }
    if (abs(distanceMm) > MAX_DISTANCE_MM) {
        Serial.println("Requested distance exceeds maximum allowed distance. Cannot move.");
        return;
    }
    int direction = distanceMm >= 0 ? HIGH : LOW;
    int distanceAbs = abs(distanceMm);
    unsigned long durationTenths = distanceAbs * DISTANCE_TO_DURATION_RATIO * 100;
    Serial.println("Move device - Distance: " + String(distanceMm) + "mm, Direction: " + String(direction) + ", Duration: " + String(durationTenths * 0.1) + "s");
    setDirection(direction);
    analogWrite(speedPin, FIXED_MOVE_SPEED_HZ); // Fixed speed for movement
    startMovement(durationTenths);
    currentPosition += distanceMm;
}

void Device::moveTo(int positionMm) {
    move(positionMm - currentPosition);
}

void Device::update() {
    if (isMoving && millis() - moveStartMillis >= moveDuration) {
        stopMovement();
    }
}

void Device::startMovement(unsigned long durationTenths) {
    start();
    isMoving = true;
    moveStartMillis = millis();
    moveDuration = durationTenths * 100;
    Serial.println("Movement started for duration: " + String(durationTenths * 0.1) + "s");
}

void Device::stopMovement() {
    stop();
    isMoving = false;
    Serial.println("Device stopped at pin: " + String(startPin));
}

void Device::start() {
    digitalWrite(startPin, LOW);
    Serial.println("Start pin " + String(startPin) + " set to LOW");
}

void Device::stop() {
    digitalWrite(startPin, HIGH);
    Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void Device::setDirection(int direction) {
    digitalWrite(directionPin, direction);
    Serial.println("Direction pin " + String(directionPin) + " set to " + String(direction));
}

