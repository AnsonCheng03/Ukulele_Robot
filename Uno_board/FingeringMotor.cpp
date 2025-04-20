#include "FingeringMotor.h"

FingeringMotor::FingeringMotor(int startPin, int motorID, const FingeringMotorConfig& config)
    : startPin(startPin), motorID(motorID), isRunning(false), currentState(IDLE),
      movementStartMicros(0), movementDuration(0),
    //   , reverseDirection(config.reverseDirection) 
      defaultDurationMs(config.defaultDurationMs) {}

void FingeringMotor::setup() {
    pinMode(startPin, OUTPUT);
    stop();
    Serial.println("Fingering motor setup for pin: " + String(startPin));
}

void FingeringMotor::control(int direction, int speedHz, int durationTenths) {
    Serial.println("Control fingering motor - Direction: " + String(direction) + ", Speed: " + String(speedHz) +
                   ", Duration: " + String(durationTenths * 0.1) + "s");
    moveFor(durationTenths * 100);
}

void FingeringMotor::calibrate() {
    Serial.println("Calibrating fingering motor...");
    moveFor(100); // Move for 1 second for calibration
}

void FingeringMotor::move() {
    Serial.println("Fingering motor move called (1s)");
    moveFor(defaultDurationMs);
}

void FingeringMotor::moveBy(int durationMs, bool reverse) {
    Serial.println("Fingering motor moveBy: " + String(durationMs) + "ms" + (reverse ? " (reverse)" : ""));
    moveFor(durationMs);
}

void FingeringMotor::moveForever() {
    Serial.println("Fingering motor repeating start");
    repeating = true;
    isOnCycle = false;         // start with OFF
    inPause = false;           // start without pause
    lastToggleMicros = micros();
    currentState = MOVING;
}

void FingeringMotor::stopRepeat() {
    Serial.println("Fingering motor repeating stopped");
    repeating = false;
    inPause = false;
    stop();
    currentState = IDLE;
}

void FingeringMotor::moveFor(unsigned long durationMs) {
    start();
    movementStartMicros = micros();
    movementDuration = durationMs * 1000UL;  // convert ms to µs
    currentState = MOVING;
}

void FingeringMotor::update() {
    unsigned long now = micros();

    if (repeating) {
        if (!inPause && now - lastToggleMicros >= defaultDurationMs * 1000UL) {
            if (isOnCycle) stop(); else start();
            isOnCycle = !isOnCycle;
            inPause = true;
            lastToggleMicros = now;
        } else if (inPause && now - lastToggleMicros >= pulsePauseMicros) {
            inPause = false;
            lastToggleMicros = now;
        }
    } else if (currentState == MOVING && micros() - movementStartMicros >= movementDuration) {
        stop();
        currentState = IDLE;
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
