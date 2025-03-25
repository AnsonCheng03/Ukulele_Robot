#include <Wire.h>
#include "Slider.h"
#include "RackMotor.h"
#include "FingeringMotor.h"
#include "JobQueue.h"
#include "Fingering.h"

#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_DEBUG 5

// slider     left_up   left_down right_up right_down
// Start      49        53        23        27
// Direction  47        51        25        29
// Speed      5         3         7         9

// rack       left_up   left_down right_up right_down
// Start      48        52        22        26
// Direction  46        50        24        28
// Speed      4         2         6         8

// fingering  left_up   left_down right_up right_down
// Start      33        34        35        36
// Direction  32        31        30        29
// Speed      10        11        12        13

const uint8_t SLIDER_START_PINS[] = {49, 53, 23, 27};
const uint8_t SLIDER_DIR_PINS[] = {47, 51, 25, 29};
const uint8_t SLIDER_SPEED_PINS[] = {5, 3, 7, 9};
const uint8_t SLIDER_SENSOR_PIN[] = {A0, A1, A2, A3};

const uint8_t RACK_START_PINS[] = {48, 52, 22, 26};
const uint8_t RACK_DIR_PINS[] = {46, 50, 24, 28};
const uint8_t RACK_SPEED_PINS[] = {4, 2, 6, 8};

const uint8_t FINGER_START_PINS[] = {33, 34, 35, 36};
const uint8_t FINGER_DIR_PINS[] = {32, 31, 30, 29};
const uint8_t FINGER_SPEED_PINS[] = {10, 11, 12, 13};


Slider sliders[4] = {
    Slider(SLIDER_START_PINS[0], SLIDER_DIR_PINS[0], SLIDER_SPEED_PINS[0], SLIDER_SENSOR_PIN[0], 8),
    Slider(SLIDER_START_PINS[1], SLIDER_DIR_PINS[1], SLIDER_SPEED_PINS[1], SLIDER_SENSOR_PIN[1], 8),
    Slider(SLIDER_START_PINS[2], SLIDER_DIR_PINS[2], SLIDER_SPEED_PINS[2], SLIDER_SENSOR_PIN[2], 11),
    Slider(SLIDER_START_PINS[3], SLIDER_DIR_PINS[3], SLIDER_SPEED_PINS[3], SLIDER_SENSOR_PIN[3], 11)
};

RackMotor rackMotors[4] = {
    RackMotor(RACK_START_PINS[0], RACK_DIR_PINS[0], RACK_SPEED_PINS[0], 8),
    RackMotor(RACK_START_PINS[1], RACK_DIR_PINS[1], RACK_SPEED_PINS[1], 8),
    RackMotor(RACK_START_PINS[2], RACK_DIR_PINS[2], RACK_SPEED_PINS[2], 11),
    RackMotor(RACK_START_PINS[3], RACK_DIR_PINS[3], RACK_SPEED_PINS[3], 11)
};

FingeringMotor fingeringMotors[4] = {
    FingeringMotor(FINGER_START_PINS[0], FINGER_DIR_PINS[0], FINGER_SPEED_PINS[0], 8),
    FingeringMotor(FINGER_START_PINS[1], FINGER_DIR_PINS[1], FINGER_SPEED_PINS[1], 9),
    FingeringMotor(FINGER_START_PINS[2], FINGER_DIR_PINS[2], FINGER_SPEED_PINS[2], 10),
    FingeringMotor(FINGER_START_PINS[3], FINGER_DIR_PINS[3], FINGER_SPEED_PINS[3], 11)
};

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
        sliders[i].setup();
        rackMotors[i].setup();
        fingeringMotors[i].setup();
        sliders[i].calibrate();
        rackMotors[i].calibrate();
        fingeringMotors[i].calibrate();
    }

}

void loop() {
    updateMotorsAndSliders();
    executeNextJobIfReady();
    handleIntervalTasks(); 
    handleSerialInput(); // New function for Serial1 TX/RX
}

void updateMotorsAndSliders() {
    for (int i = 0; i < 4; ++i) {
        sliders[i].update();
        rackMotors[i].update();
        fingeringMotors[i].update();
    }
}

void executeNextJobIfReady() {
    bool allComplete = true;
    for (int i = 0; i < 4; ++i) {
        if (!sliders[i].isMovementComplete() || !rackMotors[i].isMovementComplete() || !fingeringMotors[i].isMovementComplete()) {
            allComplete = false;
            break;
        }
    }

    if (allComplete && !isJobQueueEmpty()) {
        Job nextJob = dequeueJob();
        if (nextJob.function != nullptr) {
            nextJob.function(nextJob.context);
        }
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

        // Echo to USB serial
        // Serial.print("Received: ");
        // Serial.println(incomingByte);

        if (incomingByte == '\n') {
            inputLine.trim();
            if (inputLine.length() > 0) {
                processCommand(inputLine);
            }
            inputLine = "";
        } else {
            inputLine += incomingByte;
        }
    }
}

void processCommand(const String& commandStr) {
    Serial.println("Processing: " + commandStr);
    char* tokens[8];
    char buffer[100];
    commandStr.toCharArray(buffer, sizeof(buffer));
    
    int tokenCount = 0;
    char* token = strtok(buffer, " ");
    while (token != nullptr && tokenCount < 8) {
        tokens[tokenCount++] = token;
        token = strtok(nullptr, " ");
    }

    if (tokenCount == 0) return;

    String cmd = tokens[0];
    if (cmd == "control" && tokenCount == 6) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int speed = atoi(tokens[3]);
        int direction = atoi(tokens[4]);
        unsigned long duration = atol(tokens[5]);

        Serial.print("CMD_CONTROL: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);
        Serial.print(", Speed=");
        Serial.print(speed);
        Serial.print(", Direction=");
        Serial.print(direction);
        Serial.print(", Duration=");
        Serial.println(duration);

        if (motorID == 0) {
            if(target == 0 || target == 1)
                for (int i = 0; i < 4; ++i) {
                    sliders[i].control(direction, speed, duration);
                }
            if(target == 0 || target == 2)
                for (int i = 0; i < 4; ++i) {
                    rackMotors[i].control(direction, speed, duration);
                }
            if(target == 0 || target == 3)
                for (int i = 0; i < 4; ++i) {
                    fingeringMotors[i].control(direction, speed, duration);
                }
        } else if (motorID <= 4) {
            if (target == 0 || target == 1)
                sliders[motorID - 1].control(direction, speed, duration);
            if (target == 0 || target == 2)
                rackMotors[motorID - 1].control(direction, speed, duration);
            if (target == 0 || target == 3)
                fingeringMotors[motorID - 1].control(direction, speed, duration);
        }
    } else if (cmd == "C" && tokenCount == 3) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);

        Serial.print("CMD_CALIBRATE: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i) {
                    sliders[i].calibrate();
                }
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i) {
                    rackMotors[i].calibrate();
                }
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i) {
                    fingeringMotors[i].calibrate();
                }
        } else if (motorID <= 4) {
            if (target == 0 || target == 1)
                sliders[motorID - 1].calibrate();
            if (target == 0 || target == 2)
                rackMotors[motorID - 1].calibrate();
            if (target == 0 || target == 3)
                fingeringMotors[motorID - 1].calibrate();
        }
    } else if (cmd == "M" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int distance = atoi(tokens[3]);

        Serial.print("CMD_MOVE: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);
        Serial.print(", Distance=");
        Serial.println(distance);

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i) {
                    sliders[i].move(distance);
                }
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i) {
                    rackMotors[i].move(distance);
                }
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i) {
                    fingeringMotors[i].move(distance);
                }
        } else if (motorID <= 4) {
            // TODO: add fingering motor
            if (target == 0)
                moveFinger(sliders[motorID - 1], rackMotors[motorID - 1], distance);
            else if (target == 1)
                sliders[motorID - 1].move(distance);
            else if (target == 2)
                rackMotors[motorID - 1].move(distance);
            else if (target == 3)
                fingeringMotors[motorID - 1].move(distance);
        }
    } else if (cmd == "D" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int position = atoi(tokens[3]);

        Serial.print("CMD_DEBUG: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);
        Serial.print(", Position=");
        Serial.println(position);

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i) {
                    sliders[i].moveBy(position);
                }
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i) {
                    rackMotors[i].moveBy(position);
                }
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i) {
                    fingeringMotors[i].moveBy(position);
                }
        } else if (motorID <= 4) {
            if (target == 0 || target == 1)
                sliders[motorID - 1].moveBy(position);
            if (target == 0 || target == 2)
                rackMotors[motorID - 1].moveBy(position);
            if (target == 0 || target == 3)
                fingeringMotors[motorID - 1].moveBy(position);
        }
    } else {
        Serial.println("Unknown or invalid command.");
    }
}