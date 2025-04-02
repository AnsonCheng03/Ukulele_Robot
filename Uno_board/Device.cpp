#include "Device.h"

Device::Device(int startPin, int directionPin, int speedPin, int motorID)
    : startPin(startPin), directionPin(directionPin), speedPin(speedPin), motorID(motorID), moveStartMillis(0), moveDuration(0), isCalibrated(false), currentPosition(0), max_distance(0), fixedMoveSpeed(1000), distanceToDurationRatio(0.01), currentState(IDLE), trueState(IDLE) {}
    

void Device::setup()
{
    pinMode(startPin, OUTPUT);
    pinMode(directionPin, OUTPUT);
    pinMode(speedPin, OUTPUT);
    Serial.println("Device " + String(motorID) + " setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
}

void Device::control(int direction, int speedHz, int durationTenths)
{
    Serial.println("Control device - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
    isCalibrated = false; // Control operation makes the device uncalibrated
    analogWrite(speedPin, speedHz);
    setDirection(direction);
    startMovement(durationTenths);
}

void Device::moveBy(int distanceMm, bool reverse)
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
    setDirection(reverse ? !direction : direction);
    analogWrite(speedPin, fixedMoveSpeed); // Fixed speed for movement
    startMovement(durationTenths);
    currentPosition += distanceMm;
}

void Device::update()
{
    if (currentState == MOVING && millis() - moveStartMillis >= moveDuration)
    {
        stopMovement();
    }
}


void Device::startMovement(unsigned long durationTenths)
{
    start();
    moveStartMillis = millis();
    moveDuration = durationTenths * 100;
    currentState = MOVING;
    // Serial.println("FSM: Transition to MOVING state.");
}


void Device::stopMovement()
{
    stop();
    // Don't reset to IDLE blindly — let subclass decide if still CALIBRATING
    if (trueState != CALIBRATING) {
        currentState = IDLE;
    } else {
        currentState = CALIBRATING;
    }
    // Serial.println("FSM: Stopping movement, state = " + String(currentState));
}



void Device::start()
{
    digitalWrite(startPin, LOW);
    // Serial.println("Start pin " + String(startPin) + " set to LOW");
}

void Device::stop()
{
    digitalWrite(startPin, HIGH);
    // Serial.println("Start pin " + String(startPin) + " set to HIGH");
}

void Device::setDirection(int direction)
{
    digitalWrite(directionPin, direction);
    // Serial.println("Direction pin " + String(directionPin) + " set to " + String(direction));
}

bool Device::isMovementComplete()
{
    return currentState == IDLE;
}

int Device::getmotorID()
{
    return motorID;
}