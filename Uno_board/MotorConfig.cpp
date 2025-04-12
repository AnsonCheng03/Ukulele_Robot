#include "MotorConfig.h"

// Customize these values per motor
UpperMotorConfig sliderConfigs[] = {
    {100, 1000, 0.01, true},  // LEFT_UP
    {100, 1000, 0.01, false},  // LEFT_DOWN
    {100, 1000, 0.01, true},   // RIGHT_UP
    {100, 1000, 0.01, true}    // RIGHT_DOWN
};

UpperMotorConfig rackConfigs[] = {
    {100, 900, 0.012, false, 2, 3, 30},   // LEFT_UP
    {100, 1100, 0.008, false, 2, 3, 30},  // LEFT_DOWN
    {100, 950, 0.01, false, 2, 3, 30},     // RIGHT_UP
    {100, 1050, 0.009, false, 2, 3, 30}    // RIGHT_DOWN
};

FingeringMotorConfig fingeringConfigs[] = {
    {1, false},  // LEFT_UP
    {1, false},  // LEFT_DOWN
    {1, true},   // RIGHT_UP
    {1, true}    // RIGHT_DOWN
};
