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
    int sensorValue = getSensorValue();
    // Serial.println("Sensor value: " + String(sensorValue) + " ID: " + String(motorID) + " State: " + String(currentState));

    if (currentState == MOVING && sensorValue < 1000 && millis() > moveIgnoreSensorUntil && trueState != CALIBRATING) {
        bool movingTowardSensor = currentDirectionSignal == (reverseDirection ? HIGH : LOW);

        if (movingTowardSensor) {
            Serial.println("Slider sensor triggered while moving toward it. Stopping. ID: " + String(motorID));
            stopMovement();
            currentPosition = 0;
            isCalibrated = true;
        }
    }

    UpperMotor::update(); // Handles MOVING → IDLE transitions

    if (trueState == CALIBRATING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                setDirection(reverseDirection ? LOW : HIGH);
                analogWrite(speedPin, 2000);
                if (sensorValue < 1000) 
                    calibrationPhase = CALIBRATION_BACK_OFF;
                else 
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                break;

            case CALIBRATION_BACK_OFF:
                setDirection(reverseDirection ? LOW : HIGH);
                analogWrite(speedPin, 1000);
                startMovement(10000);  
                calibrationPhase = CALIBRATION_WAIT_RELEASE;
                break;

            case CALIBRATION_WAIT_RELEASE:
                if (sensorValue > 1000) {  // sensor no longer triggered
                    calibrationPhase = CALIBRATION_WAIT_1;
                    calibrationPhaseStart = millis();
                }
                break;

            case CALIBRATION_WAIT_1:
                if (millis() - calibrationPhaseStart >= 1000) {
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                    calibrationPhaseStart = millis();
                }
                break;

            case CALIBRATION_SEEK_SENSOR:
                // Serial.println("Seeking sensor... " + String(motorID) + " " + String(sensorValue));
                setDirection(reverseDirection ? HIGH : LOW);
                analogWrite(speedPin, 1000);
                startMovement(10000);
                if (sensorValue <= 1000) {
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