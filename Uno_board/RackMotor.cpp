#include "RackMotor.h"

RackMotor::RackMotor(int startPin, int directionPin, int speedPin, int sensorPin, int motorID)
    : UpperMotor(startPin, directionPin, speedPin, motorID), sensorPin(sensorPin) {}

void RackMotor::setup() {
    UpperMotor::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Rack motor setup for sensor pin: " + String(sensorPin));
    max_distance = 100;
    fixedMoveSpeed = 1000;
    distanceToDurationRatio = 0.01;
}

void RackMotor::update() {
    if (currentState == MOVING && getSensorValue() < 1000) {
        Serial.println("Rack sensor triggered. Stopping and recalibrating.");
        stopMovement();
        currentPosition = 0;
        isCalibrated = true;
    }


    UpperMotor::update(); // Handles MOVING → IDLE transitions

    if (trueState == CALIBRATING && currentState != MOVING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                setDirection(motorID >= 10 ? LOW : HIGH); // Go upward
                analogWrite(speedPin, 2000);
                if (getSensorValue() < 1000) 
                    calibrationPhase = CALIBRATION_BACK_OFF;
                else 
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                break;

            case CALIBRATION_BACK_OFF:
                setDirection(motorID >= 10 ? HIGH : LOW);
                analogWrite(speedPin, 30);
                startMovement(3); 
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
                Serial.println("Seeking sensor..." + String(motorID) + " " + String(getSensorValue()));
                if (getSensorValue() > 1000) {
                    setDirection(motorID >= 10 ? LOW : HIGH); // Keep pulsing up
                    analogWrite(speedPin, 10);
                    startMovement(10000);
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
                Serial.println("Rack motor calibration complete.");
                break;
        }
    }
}

int RackMotor::getSpeedPin() {
    return speedPin;
}

void RackMotor::calibrate() {
    Serial.println("Starting rack motor calibration...");
    trueState = CALIBRATING;
    currentState = CALIBRATING;
    calibrationPhase = CALIBRATION_INIT;
}

void RackMotor::move(int positionMm)
{
    if(positionMm > 0) {
        if(getSensorValue() < 1000) {
            Serial.println("Sensor hit detected. Stopping.");
            stopMovement();
            return;
        }

        up();
    } else {
        down();
    }
}

// need integrate with calibration later
void RackMotor::up() {
    Serial.println("Rack motor moving up...");
    moveBy(motorID >= 10 ? -2 : 2);
}

void RackMotor::down() {
    Serial.println("Rack motor moving down...");
    moveBy(motorID >= 10 ? 2 : -2);
}

int RackMotor::getSensorValue() {
    return analogRead(sensorPin);
}
