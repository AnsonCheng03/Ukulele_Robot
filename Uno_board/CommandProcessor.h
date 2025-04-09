// CommandProcessor.h
#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <Arduino.h>
#include "Slider.h"
#include "RackMotor.h"
#include "FingeringMotor.h"

void processCommand(const String& commandStr,
                    Slider sliders[],
                    RackMotor rackMotors[],
                    FingeringMotor fingeringMotors[]);

#endif
