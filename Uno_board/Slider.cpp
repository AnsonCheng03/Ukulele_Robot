#include "Slider.h"

Slider::Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID)
    : Device(startPin, directionPin, speedPin, motorID), sensorPin(sensorPin), isCalibrating(false) {}

void Slider::setup() {
    Device::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Slider setup for sensor pin: " + String(sensorPin));
    max_distance = 100;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
}

void Slider::calibrate() {
    Serial.println("Calibrating slider...");
    isCalibrating = true;
    if(sensorPin > 0) {
        Serial.println("slider sensor pin: " + String(sensorPin) + ", sensor value: " + String(digitalRead(sensorPin)));

        unsigned long waitDuration = 100; // Combined duration for movement and waiting (in milliseconds)
        unsigned long movementDuration = waitDuration / 100; // Start movement parameter derived from wait duration

        unsigned long startWaitTime = millis();

        // Step 1: If the sensor is already HIGH, move backward for 5 seconds
        if (digitalRead(sensorPin) == HIGH) {
            setDirection(motorID >= 10 ? LOW : HIGH);
            analogWrite(speedPin, 1000);
            startMovement(movementDuration); // Move backward for 5 seconds

            unsigned long loopWaitTime = millis();
            while (millis() - loopWaitTime < waitDuration) {
                // Waiting for 5 seconds
            }
        }

        // Step 2: Wait for 5 seconds after movement to avoid command override
        startWaitTime = millis();
        while (millis() - startWaitTime < waitDuration) {
            // Waiting for 5 seconds
        }

        // Step 3: Move slowly until it reaches the sensor, 5 seconds per loop
        while (digitalRead(sensorPin) == LOW) {
            setDirection(motorID >= 10 ? HIGH : LOW);
            analogWrite(speedPin, 1000);
            startMovement(movementDuration); // Move slowly for 5 seconds

            // Wait for 5 seconds before starting the next movement
            unsigned long loopWaitTime = millis();
            while (millis() - loopWaitTime < waitDuration) {
                // Waiting for 5 seconds
            }
        }

        isCalibrated = true;
        Serial.println("Slider calibration complete.");
    } else {
        isCalibrated = true;
        currentPosition = 0;
        Serial.println("Slider calibration complete.");
    }
    isCalibrating = false;
}



int Slider::getSensorValue() {
    return digitalRead(sensorPin);
}

void Slider::move(int positionMm)
{
    if (!isCalibrated)
    {
        Serial.println("Device not calibrated. Cannot move.");
        return;
    }
    moveBy(positionMm - currentPosition, motorID >= 10);
}

void Slider::update() {
    Device::update();
    if (isCalibrating && !isMoving) {
        isCalibrating = false;
        isCalibrated = true;
        currentPosition = 0;
        stop();
        Serial.println("Slider calibration complete.");
    }
}