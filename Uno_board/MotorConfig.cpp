#include "MotorConfig.h"

// {
//     int maxDistance;
//     int fixedMoveSpeed;
//     double distanceToDurationRatio;
//     bool reverseDirection;
// };

UpperMotorConfig sliderConfigs[] = {
    {10000, 1000, 0.7, true},  // LEFT_UP
    {10000, 1000, 0.7, false},  // LEFT_DOWN
    {10000, 1000, 0.7, true},   // RIGHT_UP
    {10000, 1000, 0.7, true}    // RIGHT_DOWN
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
    {1000, 10, 1, false, 10, 150, 30},   // LEFT_UP
    {1000, 10, 1, false, 10, 85, 30},  // LEFT_DOWN
    {1000, 10, 1, false, 10, 100, 30},     // RIGHT_UP
    {1000, 10, 1, false, 10, 70, 30}    // RIGHT_DOWN
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
