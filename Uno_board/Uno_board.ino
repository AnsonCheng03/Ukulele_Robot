#include <Wire.h>
#include "Slider.h"
#include "RackMotor.h"
#include "JobQueue.h"
#include "Fingering.h"

#define boardAddress 10
#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_DEBUG 5

#define SLIDER_START_PIN 9
#define SLIDER_DIRECTION_PIN 10
#define SLIDER_SPEED_PIN 11
#define SLIDER_SENSOR_PIN A0
// #define SLIDER_SENSOR_PIN 0

#define RACK_START_PIN 4
#define RACK_DIRECTION_PIN 5
#define RACK_SPEED_PIN 6

Slider slider(SLIDER_START_PIN, SLIDER_DIRECTION_PIN, SLIDER_SPEED_PIN, SLIDER_SENSOR_PIN, boardAddress);
RackMotor rackMotor(RACK_START_PIN, RACK_DIRECTION_PIN, RACK_SPEED_PIN, boardAddress);

unsigned long previousMillis = 0;
const long interval = 50;

void setup()
{
    Serial.begin(9600);
    Serial.println("Board address: " + String(boardAddress));
    slider.setup();
    rackMotor.setup();
    rackMotor.calibrate();
    slider.calibrate();
    Wire.begin(boardAddress);
    Wire.onReceive(receiveEvent);
    Serial.println("Broadcast started");
}

void loop() {
    slider.update();
    rackMotor.update();

    // If both slider and rack motor are not moving, execute the next pending job
    if (slider.isMovementComplete() && rackMotor.isMovementComplete() && !isJobQueueEmpty()) {
        // Get the next job from the queue
        Job nextJob = dequeueJob();
        if (nextJob.function != nullptr) {
            nextJob.function(nextJob.context); // Execute the job with context
        }
    }

    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        // Perform time-sensitive tasks without blocking
    }

    // Handle Serial input if available
    // if (Serial.available() > 0) {
    //     uint8_t buffer[32];
    //     int length = readAndProcessInput(buffer, sizeof(buffer), "Serial");
    //     if (length > 0) {
    //         processCommand(buffer, length);
    //     }
    // }

    // print sensor value
    // Serial.println("Sensor value: " + String(slider.getSensorValue()));
}

void receiveEvent(int bytes)
{
    uint8_t buffer[10];
    int length = readAndProcessInput(buffer, sizeof(buffer), "I2C");
    if (length > 0) {
        processCommand(buffer, length);
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

    switch (buffer[0])
    {
    case CMD_CONTROL:
        if (length >= 9)
        {
            uint8_t target = buffer[1];
            uint16_t speedHz = (buffer[2] << 8) | buffer[3];
            uint32_t durationTenths = ((uint32_t)buffer[4] << 24) | ((uint32_t)buffer[5] << 16) | ((uint32_t)buffer[6] << 8) | buffer[7];
            uint8_t direction = buffer[8];

            Serial.println("CMD_CONTROL: Target = " + String(target) + ", Speed = " + String(speedHz) + ", Duration = " + String(durationTenths * 0.1) + "s, Direction = " + String(direction));

            if (target == 0 || target == 1)
            {
                slider.control(direction, speedHz, durationTenths);
            }

            if (target == 0 || target == 2)
            {
                rackMotor.control(direction, speedHz, durationTenths);
            }
        }
        else
        {
            Serial.println("Not enough data for CONTROL.");
        }
        break;

    case CMD_CALIBRATE:
        if (length >= 2)
        {
            uint8_t target = buffer[1];
            Serial.println("CMD_CALIBRATE: Target = " + String(target));
            if (target == 0 || target == 1)
            {
                slider.calibrate();
            }

            if (target == 0 || target == 2)
            {
                rackMotor.calibrate();
            }
        }
        else
        {
            Serial.println("Not enough data for CALIBRATE.");
        }
        break;

    case CMD_MOVE:
        if (length >= 6)
        {
            uint8_t target = buffer[1];
            int32_t distanceMm = ((int32_t)buffer[2] << 24) | ((int32_t)buffer[3] << 16) | ((int32_t)buffer[4] << 8) | buffer[5];
            Serial.println("CMD_MOVE: Target = " + String(target) + ", Distance = " + String(distanceMm) + "mm");

            if (target == 0)
            {
                moveFinger(slider, rackMotor, distanceMm);
            }
            else if (target == 1)
            {
                slider.move(distanceMm);
            }
            else if (target == 2)
            {
                rackMotor.move(distanceMm);
            }
        }
        else
        {
            Serial.println("Not enough data for MOVE.");
        }
        break;

    case CMD_DEBUG:
        Serial.println("CMD_DEBUG received");
        if (length >= 7) {
            uint8_t action = buffer[1];
            switch (action)
            {
            case 0: // moveBy
                uint8_t target = buffer[2];
                int32_t positionMm = ((int32_t)buffer[3] << 24) | ((int32_t)buffer[4] << 16) | ((int32_t)buffer[5] << 8) | buffer[6];
                if (target == 0 || target == 1)
                {
                    slider.moveBy(positionMm);
                }
                if (target == 0 || target == 2)
                {
                    rackMotor.moveBy(positionMm);
                }
                break;

            default:
                Serial.println("Unknown debug action.");
                break;
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
