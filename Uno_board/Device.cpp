#include "Device.h"

Device::Device(int startPin, int directionPin, int speedPin)
    : startPin(startPin), directionPin(directionPin), speedPin(speedPin), isMoving(false), moveStartMillis(0), moveDuration(0), isCalibrated(false), currentPosition(0), max_distance(0), fixedMoveSpeed(1000), distanceToDurationRatio(0.01) {}
    

void Device::setup()
{
    pinMode(startPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
    pinMode(speedPin, OUTPUT);
    Serial.println("Device setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
}

void Device::control(int direction, int speedHz, int durationTenths)
{
    Serial.println("Control device - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
    isCalibrated = false; // Control operation makes the device uncalibrated
    analogWrite(speedPin, speedHz);
    setDirection(direction);
    startMovement(durationTenths);
}

void Device::moveBy(int distanceMm)
{
    if (distanceMm == 0)
    {
        Serial.println("Requested distance is zero. No movement needed.");
        return;
    }
    if (max_distance != 0 &&
        (currentPosition + distanceMm < 0 || currentPosition + distanceMm > max_distance))
    {
        Serial.println("Requested distance exceeds minimum allowed distance. Cannot move.");
        return;
    }
    int direction = distanceMm >= 0 ? HIGH : LOW;
    int distanceAbs = abs(distanceMm);
    unsigned long durationTenths = distanceAbs * distanceToDurationRatio * 100;
    if (durationTenths <= 0)
    {
        Serial.println("Requested distance is too small. Cannot move.");
        return;
    }
    Serial.println("Move device - Distance: " + String(distanceMm) + "mm, Direction: " + String(direction) + ", Duration: " + String(durationTenths * 0.1) + "s");
    setDirection(direction);
    analogWrite(speedPin, fixedMoveSpeed); // Fixed speed for movement
    startMovement(durationTenths);
    currentPosition += distanceMm;
}

void Device::update()
{
    if (isMoving && millis() - moveStartMillis >= moveDuration)
    {
        stopMovement();
    }
}

void Device::startMovement(unsigned long durationTenths)
{
    start();
    isMoving = true;
    moveStartMillis = millis();
    moveDuration = durationTenths * 100;
    Serial.println("Movement started for duration: " + String(durationTenths * 0.1) + "s");
}

void Device::stopMovement()
{
    stop();
    isMoving = false;
    Serial.println("Device stopped at pin: " + String(startPin));
}

void Device::start()
{
    digitalWrite(startPin, LOW);
    Serial.println("Start pin " + String(startPin) + " set to LOW");
}

void Device::stop()
{
    digitalWrite(startPin, HIGH);
    Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void Device::setDirection(int direction)
{
    digitalWrite(directionPin, direction);
    Serial.println("Direction pin " + String(directionPin) + " set to " + String(direction));
}

bool Device::isMovementComplete()
{
    return !isMoving;
}