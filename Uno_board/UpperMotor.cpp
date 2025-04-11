#include "UpperMotor.h"

UpperMotor::UpperMotor(int startPin, int directionPin, int speedPin, int motorID, const UpperMotorConfig& config)
    : startPin(startPin), directionPin(directionPin), speedPin(speedPin), motorID(motorID),
      max_distance(config.maxDistance), fixedMoveSpeed(config.fixedMoveSpeed),
      distanceToDurationRatio(config.distanceToDurationRatio), reverseDirection(config.reverseDirection),
      moveStartMillis(0), moveDuration(0), isCalibrated(false), currentPosition(0),
      currentState(IDLE), trueState(IDLE), moveIgnoreSensorUntil(0) {}

void UpperMotor::setup()
{
    pinMode(startPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
    pinMode(speedPin, OUTPUT);
    Serial.println("UpperMotor " + String(motorID) + " setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
}

void UpperMotor::control(int direction, int speedHz, int durationTenths)
{
    Serial.println("Control upper motor - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
    isCalibrated = false;
    analogWrite(speedPin, speedHz);
    setDirection(reverseDirection ? !direction : direction);
    startMovement(durationTenths);
}

void UpperMotor::moveBy(int distanceMm, bool reverse)
{
    if (distanceMm == 0)
    {
        Serial.println("Requested distance is zero. No movement needed.");
        return;
    }
    // if (max_distance != 0 &&
    //     (currentPosition + distanceMm < 0 || currentPosition + distanceMm > max_distance))
    // {
    //     Serial.println("Requested distance exceeds allowed range. Cannot move.");
    //     return;
    // }
    int direction = distanceMm >= 0 ? HIGH : LOW;
    int distanceAbs = abs(distanceMm);
    unsigned long durationTenths = distanceAbs * distanceToDurationRatio * 100;
    if (durationTenths <= 0)
    {
        Serial.println("Requested distance is too small. Cannot move.");
        return;
    }
    Serial.println("Move upper motor - Distance: " + String(distanceMm) + "mm, Direction: " + String(direction) + ", Duration: " + String(durationTenths * 0.1) + "s");
    setDirection(reverse ? !direction : direction);
    analogWrite(speedPin, fixedMoveSpeed);
    startMovement(durationTenths);
    moveIgnoreSensorUntil = millis() + 300;
    currentPosition += distanceMm;
}

void UpperMotor::update()
{
    if (currentState == MOVING && millis() - moveStartMillis >= moveDuration)
    {
        stopMovement();
    }
}

void UpperMotor::startMovement(unsigned long durationTenths)
{
    start();
    moveStartMillis = millis();
    moveDuration = durationTenths * 100;
    currentState = MOVING;
}

void UpperMotor::stopMovement()
{
    stop();
    if (trueState != CALIBRATING) {
        currentState = IDLE;
    } else {
        currentState = CALIBRATING;
    }
}

void UpperMotor::start()
{
    digitalWrite(startPin, LOW);
}

void UpperMotor::stop()
{
    digitalWrite(startPin, HIGH);
}

void UpperMotor::setDirection(int direction)
{
    digitalWrite(directionPin, direction);
}

bool UpperMotor::isMovementComplete()
{
    return currentState == IDLE;
}

