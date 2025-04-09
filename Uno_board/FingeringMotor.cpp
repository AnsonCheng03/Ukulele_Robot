#include "FingeringMotor.h"

FingeringMotor::FingeringMotor(int startPin, int motorID)
    : startPin(startPin), motorID(motorID), isRunning(false), currentState(IDLE),
      movementStartTime(0), movementDuration(0) {}

void FingeringMotor::setup() {
    pinMode(startPin, OUTPUT);
    stop();
    Serial.println("Fingering motor setup for pin: " + String(startPin));
}

int FingeringMotor::getMotorID() {
    return motorID;
}

void FingeringMotor::control(int direction, int speedHz, int durationTenths) {
    Serial.println("Control fingering motor - Direction: " + String(direction) + ", Speed: " + String(speedHz) +
                   ", Duration: " + String(durationTenths * 0.1) + "s");
    moveFor(durationTenths * 100);
}

void FingeringMotor::calibrate() {
    Serial.println("Calibrating fingering motor...");
}

void FingeringMotor::move() {
    Serial.println("Fingering motor move called (1s)");
    moveFor(1000);  // 1 second
}

void FingeringMotor::moveBy(int durationMs, bool reverse) {
    Serial.println("Fingering motor moveBy: " + String(durationMs) + "ms" + (reverse ? " (reverse)" : ""));
    moveFor(durationMs);
}

void FingeringMotor::moveFor(unsigned long durationMs) {
    start();
    movementStartTime = millis();
    movementDuration = durationMs;
    currentState = MOVING;
}

void FingeringMotor::update() {
    if (currentState == MOVING && millis() - movementStartTime >= movementDuration) {
        stop();
        currentState = IDLE;
        Serial.println("Fingering motor movement complete");
    }
}

bool FingeringMotor::isMovementComplete() {
    return currentState == IDLE;
}

void FingeringMotor::start() {
    digitalWrite(startPin, HIGH);
    isRunning = true;
    Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void FingeringMotor::stop() {
    digitalWrite(startPin, LOW);
    isRunning = false;
    Serial.println("Start pin " + String(startPin) + " set to LOW");
}

void FingeringMotor::up() {
    Serial.println("Fingering motor moving up...");
    move();
}

void FingeringMotor::down() {
    Serial.println("Fingering motor moving down...");
    move();
}
