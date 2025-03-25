#include "Slider.h"

Slider::Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID)
    : Device(startPin, directionPin, speedPin, motorID), sensorPin(sensorPin) {}

void Slider::setup() {
    Device::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Slider setup for sensor pin: " + String(sensorPin));
    max_distance = 100;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
}

void Slider::calibrate() {
    Serial.println("Starting FSM-based calibration...");
    trueState = CALIBRATING;
    currentState = CALIBRATING;
    calibrationPhase = CALIBRATION_INIT;
}

int Slider::getSensorValue() {
    return analogRead(sensorPin);
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
    Device::update(); // Handles MOVING → IDLE transition


    if (trueState == CALIBRATING && currentState != MOVING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                if (getSensorValue() < 900) {
                    Serial.println("Sensor already active. Backing off...");
                    setDirection(motorID <= 10 ? LOW : HIGH);
                    analogWrite(speedPin, 1000);
                    startMovement(1); // ~100ms
                }
                calibrationPhase = CALIBRATION_WAIT_1;
                calibrationPhaseStart = millis();
                break;

            case CALIBRATION_WAIT_1:
                if (millis() - calibrationPhaseStart >= 10) {
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                    calibrationPhaseStart = millis();
                }
                break;

            case CALIBRATION_SEEK_SENSOR:
                Serial.println("Seeking sensor..." + String(motorID) + " " + String(getSensorValue()));
                if (getSensorValue() > 900) {
                    setDirection(motorID <= 10 ? HIGH : LOW);
                    analogWrite(speedPin, 1000);
                    startMovement(1); // pulse to keep checking
                } else {
                    calibrationPhase = CALIBRATION_DONE;
                }
                break;

            case CALIBRATION_DONE:
                stop();
                trueState = IDLE;
                currentState = IDLE;
                isCalibrated = true;
                currentPosition = 0;
                Serial.println("Slider calibration complete (FSM).");
                break;
        }
    }
}
