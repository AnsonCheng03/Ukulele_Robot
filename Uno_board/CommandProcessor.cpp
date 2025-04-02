// CommandProcessor.cpp
#include "CommandProcessor.h"
#include "Fingering.h"
#include "JobQueue.h"

void processCommand(const String& commandStr,
                    Slider sliders[],
                    RackMotor rackMotors[],
                    FingeringMotor fingeringMotors[]) {

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

        if (motorID == 0) {
            if(target == 0 || target == 1)
                for (int i = 0; i < 4; ++i)
                    sliders[i].control(direction, speed, duration);
            if(target == 0 || target == 2)
                for (int i = 0; i < 4; ++i)
                    rackMotors[i].control(direction, speed, duration);
            if(target == 0 || target == 3)
                for (int i = 0; i < 4; ++i)
                    fingeringMotors[i].control(direction, speed, duration);
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

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i)
                    sliders[i].calibrate();
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i)
                    rackMotors[i].calibrate();
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i)
                    fingeringMotors[i].calibrate();
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

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i)
                    sliders[i].move(distance);
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i)
                    rackMotors[i].move(distance);
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i)
                    fingeringMotors[i].move();
        } else if (motorID <= 4) {
            if (target == 0)
                moveFinger(sliders[motorID - 1], rackMotors[motorID - 1], distance);
            else if (target == 1)
                sliders[motorID - 1].move(distance);
            else if (target == 2)
                rackMotors[motorID - 1].move(distance);
            else if (target == 3)
                fingeringMotors[motorID - 1].move();
        }

    } else if (cmd == "D" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int position = atoi(tokens[3]);

        if (motorID == 0) {
            if (target == 0 || target == 1)
                for (int i = 0; i < 4; ++i)
                    sliders[i].moveBy(position);
            if (target == 0 || target == 2)
                for (int i = 0; i < 4; ++i)
                    rackMotors[i].moveBy(position);
            if (target == 0 || target == 3)
                for (int i = 0; i < 4; ++i)
                    fingeringMotors[i].moveBy(position);
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
