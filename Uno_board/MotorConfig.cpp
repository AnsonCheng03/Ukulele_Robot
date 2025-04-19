#include "MotorConfig.h"

// {
//     int maxDistance;
//     int fixedMoveSpeed;
//     double distanceToDurationRatio;
//     bool reverseDirection;
// };

UpperMotorConfig sliderConfigs[] = {
    {10000, 1000, 0.001, true},  // LEFT_UP
    {10000, 1000, 0.001, false},  // LEFT_DOWN
    {10000, 1000, 0.001, true},   // RIGHT_UP
    {10000, 1000, 0.001, true}    // RIGHT_DOWN
};

// {
//     int maxDistance;
//     int fixedMoveSpeed;
//     double distanceToDurationRatio;
//     bool reverseDirection;

//     int normalDownDistance;       
//     int extendedDownDistance;    
//     int measurementThreshold;   
// };

UpperMotorConfig rackConfigs[] = {
    //
    {100, 9, 0.012, false, 2, 3, 30},   // LEFT_UP
    {100, 10, 0.008, false, 2, 3, 30},  // LEFT_DOWN
    {100, 9.5, 0.01, false, 2, 3, 30},     // RIGHT_UP
    {100, 10.5, 0.009, false, 2, 3, 30}    // RIGHT_DOWN
};

// {
//     int defaultDurationMs;
//     bool reverseDirection;
// };

FingeringMotorConfig fingeringConfigs[] = {
    {1, false},  // LEFT_UP
    {1, false},  // LEFT_DOWN
    {1, true},   // RIGHT_UP
    {1, true}    // RIGHT_DOWN
};
