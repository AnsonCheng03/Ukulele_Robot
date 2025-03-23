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

void handleSerialInput() {
    while (Serial1.available()) {
        char incomingByte = Serial1.read();

        // Echo to USB serial
        Serial.print("Received: ");
        Serial.println(incomingByte);

        if (incomingByte == '\n') {
            // Process command when newline received
            if (serialBufferIndex > 0) {
                processCommand(serialBuffer, serialBufferIndex);
                serialBufferIndex = 0;
            }
        } else {
            if (serialBufferIndex < sizeof(serialBuffer)) {
                serialBuffer[serialBufferIndex++] = incomingByte;
            }
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

void processCommand(uint8_t* buffer, int length)
{
    if (length < 1) {
        Serial.println("Invalid command length.");
        return;
    }

    uint8_t command = buffer[0];

    switch (command)
    {
    case CMD_CONTROL:
        if (length >= 10)
        {
            uint8_t motorIndex = buffer[1];
            uint8_t target = buffer[2];
            uint16_t speedHz = (buffer[3] << 8) | buffer[4];
            uint32_t durationTenths = ((uint32_t)buffer[5] << 24) | ((uint32_t)buffer[6] << 16) | ((uint32_t)buffer[7] << 8) | buffer[8];
            uint8_t direction = buffer[9];

            Serial.println("CMD_CONTROL: MotorIndex = " + String(motorIndex) + ", Target = " + String(target) +
                           ", Speed = " + String(speedHz) + ", Duration = " + String(durationTenths * 0.1) +
                           "s, Direction = " + String(direction));

            if (motorIndex < 4) {
                if (target == 0 || target == 1)
                    sliders[motorIndex].control(direction, speedHz, durationTenths);
                if (target == 0 || target == 2)
                    rackMotors[motorIndex].control(direction, speedHz, durationTenths);
            }
        }
        else {
            Serial.println("Not enough data for CONTROL.");
        }
        break;

    case CMD_CALIBRATE:
        if (length >= 3)
        {
            uint8_t motorIndex = buffer[1];
            uint8_t target = buffer[2];
            Serial.println("CMD_CALIBRATE: MotorIndex = " + String(motorIndex) + ", Target = " + String(target));

            if (motorIndex < 4) {
                if (target == 0 || target == 1)
                    sliders[motorIndex].calibrate();
                if (target == 0 || target == 2)
                    rackMotors[motorIndex].calibrate();
            }
        }
        else {
            Serial.println("Not enough data for CALIBRATE.");
        }
        break;

    case CMD_MOVE:
        if (length >= 7)
        {
            uint8_t motorIndex = buffer[1];
            uint8_t target = buffer[2];
            int32_t distanceMm = ((int32_t)buffer[3] << 24) | ((int32_t)buffer[4] << 16) |
                                 ((int32_t)buffer[5] << 8) | buffer[6];
            Serial.println("CMD_MOVE: MotorIndex = " + String(motorIndex) + ", Target = " + String(target) +
                           ", Distance = " + String(distanceMm) + "mm");

            if (motorIndex < 4) {
                if (target == 0)
                    moveFinger(sliders[motorIndex], rackMotors[motorIndex], distanceMm);
                else if (target == 1)
                    sliders[motorIndex].move(distanceMm);
                else if (target == 2)
                    rackMotors[motorIndex].move(distanceMm);
            }
        }
        else {
            Serial.println("Not enough data for MOVE.");
        }
        break;

    case CMD_DEBUG:
        Serial.println("CMD_DEBUG received");
        if (length >= 8) {
            uint8_t action = buffer[1];
            uint8_t motorIndex = buffer[2];
            uint8_t target = buffer[3];
            int32_t positionMm = ((int32_t)buffer[4] << 24) | ((int32_t)buffer[5] << 16) |
                                 ((int32_t)buffer[6] << 8) | buffer[7];

            if (motorIndex < 4) {
                switch (action)
                {
                case 0: // moveBy
                    if (target == 0 || target == 1)
                        sliders[motorIndex].moveBy(positionMm);
                    if (target == 0 || target == 2)
                        rackMotors[motorIndex].moveBy(positionMm);
                    break;
                default:
                    Serial.println("Unknown debug action.");
                    break;
                }
            }
        } else {
            Serial.println("Not enough data for DEBUG.");
        }
        break;

    default:
        Serial.println("Unknown command.");
        break;
    }
}
