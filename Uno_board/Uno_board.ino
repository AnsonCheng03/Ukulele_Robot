#include <Wire.h>
#include "FingerUnit.h"
#include "Slider.h"
#include "RackMotor.h"
#include "FingeringMotor.h"
#include "CommandProcessor.h"

#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_DEBUG 5

// slider     left_up   left_down right_up right_down
// Start      49        53        23        27
// Direction  47        51        25        29
// Speed      5         3         7         9
// Sensor     A0        A1        A2        A3

// rack       left_up   left_down right_up right_down
// Start      48        52        22        26
// Direction  46        50        24        28
// Speed      4         2         6         8
// Sensor     A5        A6        A7        A8

// fingering  left_up   left_down right_up right_down
// Start      33        34        35        36

const uint8_t SLIDER_START_PINS[] = {49, 53, 23, 27};
const uint8_t SLIDER_DIR_PINS[] = {47, 51, 25, 29};
const uint8_t SLIDER_SPEED_PINS[] = {5, 3, 7, 9};
const uint8_t SLIDER_SENSOR_PIN[] = {A0, A2, A4, A6};

const uint8_t RACK_START_PINS[] = {48, 52, 22, 26};
const uint8_t RACK_DIR_PINS[] = {46, 50, 24, 28};
const uint8_t RACK_SPEED_PINS[] = {4, 2, 6, 8};
const uint8_t RACK_SENSOR_PIN[] = {A1, A3, A5, A7};

const uint8_t FINGER_START_PINS[] = {36, 37, 38, 39};


Slider sliders[4] = {
    Slider(SLIDER_START_PINS[0], SLIDER_DIR_PINS[0], SLIDER_SPEED_PINS[0], SLIDER_SENSOR_PIN[0], 8),
    Slider(SLIDER_START_PINS[1], SLIDER_DIR_PINS[1], SLIDER_SPEED_PINS[1], SLIDER_SENSOR_PIN[1], 11),
    Slider(SLIDER_START_PINS[2], SLIDER_DIR_PINS[2], SLIDER_SPEED_PINS[2], SLIDER_SENSOR_PIN[2], 9),
    Slider(SLIDER_START_PINS[3], SLIDER_DIR_PINS[3], SLIDER_SPEED_PINS[3], SLIDER_SENSOR_PIN[3], 7)
};

RackMotor rackMotors[4] = {
    RackMotor(RACK_START_PINS[0], RACK_DIR_PINS[0], RACK_SPEED_PINS[0], RACK_SENSOR_PIN[0], 8),
    RackMotor(RACK_START_PINS[1], RACK_DIR_PINS[1], RACK_SPEED_PINS[1], RACK_SENSOR_PIN[1], 11),
    RackMotor(RACK_START_PINS[2], RACK_DIR_PINS[2], RACK_SPEED_PINS[2], RACK_SENSOR_PIN[2], 9),
    RackMotor(RACK_START_PINS[3], RACK_DIR_PINS[3], RACK_SPEED_PINS[3], RACK_SENSOR_PIN[3], 12)
};

FingeringMotor fingeringMotors[4] = {
    FingeringMotor(FINGER_START_PINS[0], 8),
    FingeringMotor(FINGER_START_PINS[1], 9),
    FingeringMotor(FINGER_START_PINS[2], 11),
    FingeringMotor(FINGER_START_PINS[3], 12)
};

FingerUnit* fingers[4];

unsigned long previousMillis = 0;
const long interval = 50;
static uint8_t serialBuffer[64];
static int serialBufferIndex = 0;

void setup()
{
    Serial.begin(9600); // USB communication
    Serial.println("Starting");

    Serial1.begin(115200); // TX/RX communication
    for (int i = 0; i < 4; ++i) {
        fingers[i] = new FingerUnit(&sliders[i], &rackMotors[i], &fingeringMotors[i]);
        fingers[i]->setup();
        fingers[i]->calibrate();
    }

}

void loop() {
    updateMotorsAndSliders();
    handleIntervalTasks(); 
    handleSerialInput(); // New function for Serial1 TX/RX
}

void updateMotorsAndSliders() {
    for (int i = 0; i < 4; ++i) {
        fingers[i]->update();
    }
}

void handleIntervalTasks() {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        // Perform time-sensitive tasks here
    }
}

String inputLine = "";

void handleSerialInput() {
    
    while (Serial1.available() > 0) {
        char incomingByte = Serial1.read();

        if (incomingByte == '\n') {
            inputLine.trim();
            if (inputLine.length() > 0) {
                processCommand(inputLine, fingers);
            }
            inputLine = "";
        } else {
            inputLine += incomingByte;
        }
    }
}
