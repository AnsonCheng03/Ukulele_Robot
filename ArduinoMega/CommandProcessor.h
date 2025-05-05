#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>
#include "FingerGroupController.h"

void processCommand(const String& commandStr, FingerGroupController* fingerGroup);

#endif
