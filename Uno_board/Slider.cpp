#include "Slider.h"

Slider::Slider(int startPin, int directionPin, int speedPin, int sensorPin, int motorID, const UpperMotorConfig& config)
    : UpperMotor(startPin, directionPin, speedPin, motorID, config), sensorPin(sensorPin) {}

void Slider::setup()
{
    UpperMotor::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Slider setup for sensor pin: " + String(sensorPin));
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

    moveBy(positionMm - currentPosition, reverseDirection);
}

void Slider::update() {
    if (currentState == MOVING && getSensorValue() < 1000 && millis() > moveIgnoreSensorUntil) {
        Serial.println("Slider sensor triggered. Stopping and recalibrating. ID: " + String(motorID));
        stopMovement();
        currentPosition = 0;
        isCalibrated = true;
    }

    UpperMotor::update(); // Handles MOVING → IDLE transitions

    if (trueState == CALIBRATING && currentState != MOVING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                setDirection(reverseDirection ? LOW : HIGH);
                analogWrite(speedPin, 2000);
                if (getSensorValue() < 1000) 
                    calibrationPhase = CALIBRATION_BACK_OFF;
                else 
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                break;

            case CALIBRATION_BACK_OFF:
                setDirection(reverseDirection ? LOW : HIGH);
                analogWrite(speedPin, 1000);
                startMovement(30);  // Back off for a short distance
                calibrationPhase = CALIBRATION_WAIT_1;
                calibrationPhaseStart = millis();
                break;

            case CALIBRATION_WAIT_1:
                if (millis() - calibrationPhaseStart >= 1000) {
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                    calibrationPhaseStart = millis();
                }
                break;

            case CALIBRATION_SEEK_SENSOR:
                Serial.println("Seeking sensor... " + String(motorID) + " " + String(getSensorValue()));
                setDirection(reverseDirection ? HIGH : LOW);
                analogWrite(speedPin, 10);
                startMovement(10000);
                if (getSensorValue() <= 1000) {
                    Serial.println("Sensor triggered. Calibration done for motor ID: " + String(motorID));
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

int Slider::getCurrentPosition() {
    return currentPosition;  // Assuming currentPosition holds the slider's position in mm
}