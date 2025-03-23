#include <Wire.h>
#include "Slider.h"
#include "RackMotor.h"
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

const uint8_t SLIDER_START_PINS[] = {49, 53, 23, 27};
const uint8_t SLIDER_DIR_PINS[] = {47, 51, 25, 29};
const uint8_t SLIDER_SPEED_PINS[] = {5, 3, 7, 9};
#define SLIDER_SENSOR_PIN A0

const uint8_t RACK_START_PINS[] = {48, 52, 22, 26};
const uint8_t RACK_DIR_PINS[] = {46, 50, 24, 28};
const uint8_t RACK_SPEED_PINS[] = {4, 2, 6, 8};


Slider sliders[4] = {
    Slider(SLIDER_START_PINS[0], SLIDER_DIR_PINS[0], SLIDER_SPEED_PINS[0], SLIDER_SENSOR_PIN, 8),
    Slider(SLIDER_START_PINS[1], SLIDER_DIR_PINS[1], SLIDER_SPEED_PINS[1], SLIDER_SENSOR_PIN, 9),
    Slider(SLIDER_START_PINS[2], SLIDER_DIR_PINS[2], SLIDER_SPEED_PINS[2], SLIDER_SENSOR_PIN, 10),
    Slider(SLIDER_START_PINS[3], SLIDER_DIR_PINS[3], SLIDER_SPEED_PINS[3], SLIDER_SENSOR_PIN, 11)
};

RackMotor rackMotors[4] = {
    RackMotor(RACK_START_PINS[0], RACK_DIR_PINS[0], RACK_SPEED_PINS[0], 8),
    RackMotor(RACK_START_PINS[1], RACK_DIR_PINS[1], RACK_SPEED_PINS[1], 9),
    RackMotor(RACK_START_PINS[2], RACK_DIR_PINS[2], RACK_SPEED_PINS[2], 10),
    RackMotor(RACK_START_PINS[3], RACK_DIR_PINS[3], RACK_SPEED_PINS[3], 11)
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
        // sliders[i].calibrate();
        rackMotors[i].calibrate();
    }

}

void receiveEvent(int bytes)
{
    uint8_t buffer[10];
    int length = readAndProcessInput(buffer, sizeof(buffer), "I2C");
    if (length > 0) {
        processCommand(buffer, length);
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
    }
}

void executeNextJobIfReady() {
    bool allComplete = true;
    for (int i = 0; i < 4; ++i) {
        if (!sliders[i].isMovementComplete() || !rackMotors[i].isMovementComplete()) {
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
    while (Serial1.available()) {
        char incomingByte = Serial1.read();

        // Echo to USB serial
        Serial.print("Received: ");
        Serial.println(incomingByte);

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

int readAndProcessInput(uint8_t* buffer, int bufferSize, const char* source)
{
    int index = 0;

    // if (strcmp(source, "Serial") == 0) {
    //     // Read data from Serial
    //     unsigned long startMillis = millis();
    //     while (index < bufferSize && millis() - startMillis < 1000) {
    //         if (!Serial.available()) {
    //             continue;
    //         }
    //         int c = Serial.read();
    //         if (c == '\n') {
    //             break;
    //         }
    //         // make c (string) to number
    //         if(c >= '0' && c <= '9') {
    //             c = c - '0';
    //         } else if(c >= 'A' && c <= 'F') {
    //             c = c - 'A' + 10;
    //         } else if(c >= 'a' && c <= 'f') {
    //             c = c - 'a' + 10;
    //         } else {
    //             continue;
    //         }
    //         buffer[index++] = c;
    //     }
    // } else if (strcmp(source, "I2C") == 0) {
        // Read data from I2C
        while (Wire.available() && index < bufferSize) {
            buffer[index++] = Wire.read();
        }
    // }

    // Print received data
    if (index > 0) {
        Serial.print("Received data from ");
        Serial.print(source);
        Serial.print(": ");
        for (int i = 0; i < index; i++) {
            Serial.print(buffer[i], HEX);
            Serial.print(" ");
        }
        Serial.println();
    }

    return index;
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


        if (motorID < 4) {
            if (target == 0 || target == 1)
                sliders[motorID].control(direction, speed, duration);
            if (target == 0 || target == 2)
                rackMotors[motorID].control(direction, speed, duration);
        }
    } else if (cmd == "calibrate" && tokenCount == 3) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);

        Serial.print("CMD_CALIBRATE: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);

        if (motorID < 4) {
            if (target == 0 || target == 1)
                sliders[motorID].calibrate();
            if (target == 0 || target == 2)
                rackMotors[motorID].calibrate();
        }
    } else if (cmd == "move" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int distance = atoi(tokens[3]);

        Serial.print("CMD_MOVE: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);
        Serial.print(", Distance=");
        Serial.println(distance);


        if (motorID < 4) {
            if (target == 0)
                moveFinger(sliders[motorID], rackMotors[motorID], distance);
            else if (target == 1)
                sliders[motorID].move(distance);
            else if (target == 2)
                rackMotors[motorID].move(distance);
        }
    } else if (cmd == "debug" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int position = atoi(tokens[3]);

        Serial.print("CMD_DEBUG: MotorID=");
        Serial.print(motorID);
        Serial.print(", Target=");
        Serial.print(target);
        Serial.print(", Position=");
        Serial.println(position);

        if (motorID < 4) {
            if (target == 0 || target == 1)
                sliders[motorID].moveBy(position);
            if (target == 0 || target == 2)
                rackMotors[motorID].moveBy(position);
        }
    } else {
        Serial.println("Unknown or invalid command.");
    }
}