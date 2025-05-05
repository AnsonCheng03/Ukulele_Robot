#include "FingeringMotor.h"

FingeringMotor::FingeringMotor(int startPin, int motorID, const FingeringMotorConfig& config)
    : startPin(startPin), motorID(motorID), defaultDurationMs(config.defaultDurationMs),
      repeating(false), isOnCycle(false), inPause(false), lastToggleMicros(0), pulsePauseMicros(200000) {}

void FingeringMotor::setup() {
    pinMode(startPin, OUTPUT);
    digitalWrite(startPin, LOW);
    Serial.println("Fingering motor setup for pin: " + String(startPin));
}

void FingeringMotor::control(int direction, int speedHz, int durationTenths) {
    Serial.println("Control fingering motor - Direction: " + String(direction) +
                   ", Speed: " + String(speedHz) +
                   ", Duration: " + String(durationTenths * 0.1) + "s");
    pulse(durationTenths * 100000); // tenths of seconds → microseconds
}

void FingeringMotor::calibrate() {
    Serial.println("Calibrating fingering motor...");
    pulse(1000000);  // 1 second
}

void FingeringMotor::pulse(unsigned int durationMicros) {
    digitalWrite(startPin, HIGH);
    delayMicroseconds(durationMicros);
    digitalWrite(startPin, LOW);
    Serial.println("Pulse triggered on pin " + String(startPin) + " (" + String(durationMicros) + "us)");
}

void FingeringMotor::move() {
    Serial.println("Fingering motor move (default)");
    pulse(defaultDurationMs * 1000); // ms → µs
}

void FingeringMotor::moveBy(int duration, bool reverse) {
    Serial.println("Fingering motor moveBy: " + String(duration) + "ms" + (reverse ? " (reverse)" : ""));
    pulse(duration * 1000); // ms → µs
}

void FingeringMotor::moveForever() {
    Serial.println("Fingering motor repeating start");
    repeating = true;
    isOnCycle = false;
    inPause = false;
    lastToggleMicros = micros();
}

void FingeringMotor::stopRepeat() {
    Serial.println("Fingering motor repeating stopped");
    repeating = false;
    inPause = false;
}

void FingeringMotor::update() {
    if (!repeating) return;

    unsigned long now = micros();

    if (!inPause && now - lastToggleMicros >= defaultDurationMs * 1000UL) {
        pulse(10000);  // 10 ms pulse
        isOnCycle = !isOnCycle;
        inPause = true;
        lastToggleMicros = now;
    } else if (inPause && now - lastToggleMicros >= pulsePauseMicros) {
        inPause = false;
        lastToggleMicros = now;
    }
}


bool FingeringMotor::isMovementComplete() {
    return true;
}

void FingeringMotor::start() {
    digitalWrite(startPin, HIGH);
}

void FingeringMotor::stop() {
    digitalWrite(startPin, LOW);
}
