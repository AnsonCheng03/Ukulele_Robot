#include "Slider.h"
#include "RackMotor.h"
#include <Wire.h>

#define boardAddress 5
#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_FINGERING 3
#define CMD_CHORD 4

#define SLIDER_START_PIN 9
#define SLIDER_DIRECTION_PIN 10
#define SLIDER_SPEED_PIN 11
#define SLIDER_SENSOR_PIN 12

#define RACK_START_PIN 4
#define RACK_DIRECTION_PIN 5
#define RACK_SPEED_PIN 6

Slider slider(SLIDER_START_PIN, SLIDER_DIRECTION_PIN, SLIDER_SPEED_PIN, SLIDER_SENSOR_PIN);
RackMotor rackMotor(RACK_START_PIN, RACK_DIRECTION_PIN, RACK_SPEED_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Board address: " + String(boardAddress));
    slider.setup();
    rackMotor.setup();
    slider.calibrate();
    rackMotor.calibrate();
    Wire.begin(boardAddress);
    Wire.onReceive(receiveEvent);
    Serial.println("Broadcast started");
}

void loop() {
    slider.update();
    rackMotor.update();
    delay(50);
}

void receiveEvent(int bytes) {
    uint8_t buffer[32];
    int index = 0;

    while (Wire.available() && index < sizeof(buffer)) {
        buffer[index++] = Wire.read();
    }

    Serial.print("Received data: ");
    for (int i = 0; i < index; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    switch (buffer[0]) {
        case CMD_CONTROL:
            if (index >= 9) {
                uint8_t target = buffer[1];
                uint16_t speedHz = (buffer[2] << 8) | buffer[3];
                uint32_t durationTenths = ((uint32_t)buffer[4] << 24) | ((uint32_t)buffer[5] << 16) | ((uint32_t)buffer[6] << 8) | buffer[7];
                uint8_t direction = buffer[8];

                Serial.println("CMD_CONTROL: Target = " + String(target) + ", Speed = " + String(speedHz) + ", Duration = " + String(durationTenths * 0.1) + "s, Direction = " + String(direction));

                if (target == 0 || target == 1) {
                    slider.control(direction, speedHz, durationTenths);
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.control(direction, speedHz, durationTenths);
                } 
            } else {
                Serial.println("Not enough data for CONTROL.");
            }
            break;

        case CMD_CALIBRATE:
            if (index >= 2) {
                uint8_t target = buffer[1];
                Serial.println("CMD_CALIBRATE: Target = " + String(target));
                if (target == 0 || target == 1) {
                    slider.calibrate();
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.calibrate();
                } 
            } else {
                Serial.println("Not enough data for CALIBRATE.");
            }
            break;

        case CMD_MOVE:
            if (index >= 6) {
                uint8_t target = buffer[1];
                int32_t distanceMm = ((int32_t)buffer[2] << 24) | ((int32_t)buffer[3] << 16) | ((int32_t)buffer[4] << 8) | buffer[5];
                Serial.println("CMD_MOVE: Target = " + String(target) + ", Distance = " + String(distanceMm) + "mm");
                if (target == 0 || target == 1) {
                    slider.move(distanceMm);
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.move(distanceMm);
                } 
            } else {
                Serial.println("Not enough data for MOVE.");
            }
            break;

        case CMD_FINGERING:
            Serial.println("CMD_FINGERING received");
            // Implementation for CMD_FINGERING will be added later
            break;

        case CMD_CHORD:
            Serial.println("CMD_CHORD received");
            // Implementation for CMD_CHORD will be added later
            break;

        default:
            Serial.println("Unknown command.");
            break;
    }
}
