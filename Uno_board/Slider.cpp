#include "Slider.h"

Slider::Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID)
    : UpperMotor(startPin, directionPin, speedPin, motorID), sensorPin(sensorPin) {}

void Slider::setup()
{
    UpperMotor::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Slider setup for sensor pin: " + String(sensorPin));
    max_distance = 100;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
}

void Slider::calibrate()
{
    Serial.println("Starting FSM-based calibration...");
    trueState = CALIBRATING;
    currentState = CALIBRATING;
    calibrationPhase = CALIBRATION_INIT;
}

int Slider::getSensorValue()
{
    return analogRead(sensorPin);
}

void Slider::move(int positionMm)
{
    if (!isCalibrated)
    {
        Serial.println("Device not calibrated. Cannot move.");
        return;
    }

    if (getSensorValue() < 1000 && positionMm < 0)
    {
        Serial.println("Sensor hit detected. Cannot move forward.");
        return;
    }

    moveBy(positionMm - currentPosition, motorID >= 10);
}

void Slider::update() {
    if (currentState == MOVING && getSensorValue() < 1000) {
        Serial.println("Slider sensor triggered. Stopping and recalibrating.");
        stopMovement();
        currentPosition = 0;
        isCalibrated = true;
    }

    UpperMotor::update(); // Handles MOVING → IDLE transitions

    if (trueState == CALIBRATING && currentState != MOVING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                setDirection(motorID <= 10 ? LOW : HIGH);
                analogWrite(speedPin, 2000);
                if (getSensorValue() < 1000) 
                    calibrationPhase = CALIBRATION_BACK_OFF;
                else 
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                break;

            case CALIBRATION_BACK_OFF:
                setDirection(motorID <= 10 ? LOW : HIGH);
                analogWrite(speedPin, 1000);
                startMovement(50);  // Back off for a short distance
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
                if (getSensorValue() > 1000) {
                    Serial.println("Seeking sensor... " + String(motorID) + " " + String(getSensorValue()));
                    setDirection(motorID <= 10 ? HIGH : LOW);
                    analogWrite(speedPin, 10);
                    startMovement(10000);
                } else {
                    Serial.println("Sensor triggered. Calibration done.");
                    calibrationPhase = CALIBRATION_DONE;
                }
                break;

            case CALIBRATION_DONE:
                stop();
                trueState = IDLE;
                currentState = IDLE;
                isCalibrated = true;
                currentPosition = 0;
                Serial.println("Slider calibration complete.");
                break;
        }
    }
}

