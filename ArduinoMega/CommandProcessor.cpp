#include "CommandProcessor.h"

void processCommand(const String& commandStr, FingerGroupController* fingerGroup) {
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

    if (cmd == "S" && tokenCount == 6) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int speed = atoi(tokens[3]);
        int direction = atoi(tokens[4]);
        unsigned long duration = atol(tokens[5]);

        if (motorID == 0) {
            fingerGroup->controlAll(target, direction, speed, duration);
        } else if (motorID <= 4) {
            fingerGroup->controlSingle(motorID - 1, target, direction, speed, duration);
        }

    } else if (cmd == "C" && tokenCount == 3) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);

        if (motorID == 0) {
            fingerGroup->calibrateAll(target);
        } else if (motorID <= 4) {
            fingerGroup->calibrateSingle(motorID - 1, target);
        }

    } else if (cmd == "M" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int distance = atoi(tokens[3]);

        if (motorID == 0) {
            fingerGroup->moveAll(target, distance);
        } else if (motorID <= 4) {
            fingerGroup->moveSingle(motorID - 1, target, distance);
        }
    } else if (cmd == "MF" && tokenCount == 5) {
        int distances[4];
        for (int i = 0; i < 4; ++i) {
            distances[i] = atoi(tokens[i + 1]);
        }
        fingerGroup->moveFinger(distances);
    }
    else if (cmd == "D" && tokenCount == 4) {
        int motorID = atoi(tokens[1]);
        int target = atoi(tokens[2]);
        int position = atoi(tokens[3]);

        if (motorID == 0) {
            fingerGroup->moveByAll(target, position);
        } else if (motorID <= 4) {
            fingerGroup->moveBySingle(motorID - 1, target, position);
        }

    } else {
        Serial.println("Unknown or invalid command.");
    }
}
