#include "RackMotor.h"

RackMotor::RackMotor(int startPin, int directionPin, int speedPin, int sensorPin, int motorID, const UpperMotorConfig& config)
    : UpperMotor(startPin, directionPin, speedPin, motorID, config), sensorPin(sensorPin), slider(nullptr), config(config) {}

void RackMotor::setup() {
    UpperMotor::setup();
    pinMode(sensorPin, INPUT);
    Serial.println("Rack motor setup for sensor pin: " + String(sensorPin));
}

void RackMotor::moveUntilTouchSensor(bool towardSensor) {
    UpperMotor::moveUntilTouchSensor(!towardSensor);
}

void RackMotor::update() {
    int sensorValue = getSensorValue();
    if (currentState == MOVING && sensorValue < 1000 && trueState != CALIBRATING) {
        bool movingTowardSensor = currentDirectionSignal == (reverseDirection ? HIGH : LOW);

        if (movingTowardSensor) {
            Serial.println("Rack sensor triggered while moving toward it. Stopping. ID: " + String(motorID));
            stopMovement();
            currentPosition = 0;
            isCalibrated = true;
        }
    }


    UpperMotor::update(); // Handles MOVING → IDLE transitions

    if (trueState == CALIBRATING) {
        switch (calibrationPhase) {
            case CALIBRATION_INIT:
                if (sensorValue < 1000) 
                    calibrationPhase = CALIBRATION_BACK_OFF;
                else 
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                break;

            case CALIBRATION_BACK_OFF:
                if(currentState != MOVING) {
                    moveUntilTouchSensor(false);
                }
                calibrationPhaseStart = millis();
                calibrationPhase = CALIBRATION_WAIT_1;
                break;

            case CALIBRATION_WAIT_1:
                if (millis() - calibrationPhaseStart >= 100) {
                    calibrationPhase = CALIBRATION_WAIT_RELEASE;
                }
                break;

            case CALIBRATION_WAIT_RELEASE:
                if (sensorValue > 1000) {  // sensor no longer triggered
                    stopMovement();
                    calibrationPhase = CALIBRATION_SEEK_SENSOR;
                }
                break;

            case CALIBRATION_SEEK_SENSOR:
                if(currentState != MOVING) {
                    moveUntilTouchSensor();
                }
                if (sensorValue <= 1000) {
                    Serial.println("Sensor triggered. Calibration done for motor ID: " + String(motorID));
                    calibrationPhase = CALIBRATION_DONE;
                }
                break;

            case CALIBRATION_DONE:
                trueState = IDLE;
                stopMovement();
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
        up();
    } else {
        down();
    }
}

// need integrate with calibration later
void RackMotor::up() {
    Serial.println("Rack motor moving up...");
    moveBy(reverseDirection ? -2 : 2);
}

void RackMotor::down() {
    // Check if slider is set
    if (slider == nullptr) {
        Serial.println("Error: No slider assigned to RackMotor.");
        return;  // Exit the method if slider is not assigned
    }

    Serial.println("Rack motor moving down...");

    // Get the slider's current position using the slider reference
    int sliderPosition = slider->getCurrentPosition();

    // Calculate the down distance using linear interpolation (slope)
    int downDistance = config.normalDownDistance + 
                       (sliderPosition - config.measurementThreshold) * 
                       (config.extendedDownDistance - config.normalDownDistance) / 
                       (config.maxDistance - config.measurementThreshold);

    // Log the calculated down distance for debugging
    Serial.println("Slider position: " + String(sliderPosition) + "mm, Calculated down distance: " + String(downDistance) + "mm");

    // Move by the calculated down distance
    moveBy(downDistance, !reverseDirection);
}


void RackMotor::setSlider(Slider* slider) {
    this->slider = slider;
}

int RackMotor::getSensorValue() {
    return analogRead(sensorPin);
}
