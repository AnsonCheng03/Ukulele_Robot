#include "CommandProcessor.h"

void processCommand(const String& commandStr, FingerUnit* fingers[]) {
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
            for (int i = 0; i < 4; ++i) {
                if (target == 0 || target == 1)
                    fingers[i]->getSlider()->control(direction, speed, duration);
                if (target == 0 || target == 2)
                    fingers[i]->getRackMotor()->control(direction, speed, duration);
                if (target == 0 || target == 3)
                    fingers[i]->getFingeringMotor()->control(direction, speed, duration);
            }
        } else if (motorID <= 4) {
            int index = motorID - 1;
            if (target == 0 || target == 1)
                fingers[index]->getSlider()->control(direction, speed, duration);
            if (target == 0 || target == 2)
                fingers[index]->getRackMotor()->control(direction, speed, duration);
            if (target == 0 || target == 3)
                fingers[index]->getFingeringMotor()->control(direction, speed, duration);
        }

    } else if (cmd == "C" && tokenCount == 3) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);

        if (motorID == 0) {
            for (int i = 0; i < 4; ++i) {
                if (target == 0 || target == 1)
                    fingers[i]->getSlider()->calibrate();
                if (target == 0 || target == 2)
                    fingers[i]->getRackMotor()->calibrate();
                if (target == 0 || target == 3)
                    fingers[i]->getFingeringMotor()->calibrate();
            }
        } else if (motorID <= 4) {
            int index = motorID - 1;
            if (target == 0 || target == 1)
                fingers[index]->getSlider()->calibrate();
            if (target == 0 || target == 2)
                fingers[index]->getRackMotor()->calibrate();
            if (target == 0 || target == 3)
                fingers[index]->getFingeringMotor()->calibrate();
        }

    } else if (cmd == "M" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int distance = atoi(tokens[3]);

        if (motorID == 0) {
            for (int i = 0; i < 4; ++i) {
                if (target == 0)
                    fingers[i]->moveFinger(distance);
                else if (target == 1)
                    fingers[i]->getSlider()->move(distance);
                else if (target == 2)
                    fingers[i]->getRackMotor()->move(distance);
                else if (target == 3)
                    fingers[i]->getFingeringMotor()->move();
            }
        } else if (motorID <= 4) {
            int index = motorID - 1;
            if (target == 0)
                fingers[index]->moveFinger(distance);
            else if (target == 1)
                fingers[index]->getSlider()->move(distance);
            else if (target == 2)
                fingers[index]->getRackMotor()->move(distance);
            else if (target == 3)
                fingers[index]->getFingeringMotor()->move();
        }

    } else if (cmd == "D" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int position = atoi(tokens[3]);

        if (motorID == 0) {
            for (int i = 0; i < 4; ++i) {
                if (target == 0 || target == 1)
                    fingers[i]->getSlider()->moveBy(position);
                if (target == 0 || target == 2)
                    fingers[i]->getRackMotor()->moveBy(position);
                if (target == 0 || target == 3)
                    fingers[i]->getFingeringMotor()->moveBy(position);
            }
        } else if (motorID <= 4) {
            int index = motorID - 1;
            if (target == 0 || target == 1)
                fingers[index]->getSlider()->moveBy(position);
            if (target == 0 || target == 2)
                fingers[index]->getRackMotor()->moveBy(position);
            if (target == 0 || target == 3)
                fingers[index]->getFingeringMotor()->moveBy(position);
        }

    } else {
        Serial.println("Unknown or invalid command.");
    }
}
