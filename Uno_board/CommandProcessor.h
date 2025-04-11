#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>
#include "FingerUnit.h"

void processCommand(const String& commandStr, FingerUnit* fingers[]);

#endif
