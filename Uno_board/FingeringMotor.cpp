#include "FingeringMotor.h"

FingeringMotor::FingeringMotor(int startPin, int motorID)
    : startPin(startPin), motorID(motorID), isRunning(false), currentState(IDLE) {}

void FingeringMotor::setup() {
    pinMode(startPin, OUTPUT);
    stop();
    Serial.println("Fingering motor setup for pin: " + String(startPin));
}

int FingeringMotor::getMotorID() {
    return motorID;
}

void FingeringMotor::control(int direction, int speedHz, int durationTenths) {
    // FingeringMotor does not support direction or speed control
    Serial.println("Control fingering motor - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
    start();
    delay(durationTenths * 100);
    stop();
}

void FingeringMotor::calibrate() {
    Serial.println("Calibrating rack motor...");
}

void FingeringMotor::move() {
    Serial.println("Fingering motor move called");
    start();
    delay(200); // Placeholder for actual movement
    stop();
}

void FingeringMotor::moveBy(int positionMm, bool reverse) {
    // Simple placeholder behavior for movement
    Serial.println("Fingering motor moveBy: " + String(positionMm) + "mm" + (reverse ? " (reverse)" : ""));
    start();
    delay(200); // Placeholder
    stop();
}

void FingeringMotor::update() {
    // No update logic needed for simplified motor
}

bool FingeringMotor::isMovementComplete() {
    return true; // Always complete since it's a simple on/off motor
}

void FingeringMotor::start() {
    digitalWrite(startPin, HIGH);
    isRunning = true;
    currentState = MOVING;
    Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void FingeringMotor::stop() {
    digitalWrite(startPin, LOW);
    isRunning = false;
    currentState = IDLE;
    Serial.println("Start pin " + String(startPin) + " set to LOW");
}

void FingeringMotor::up() {
    Serial.println("Fingering motor moving up...");
    moveBy(2);
}

void FingeringMotor::down() {
    Serial.println("Fingering motor moving down...");
    moveBy(-2);
}