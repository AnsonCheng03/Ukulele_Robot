#include "MotorConfig.h"

// {
//     int maxDistance;
//     int fixedMoveSpeed;
//     double distanceToDurationRatio;
//     bool reverseDirection;
// };

UpperMotorConfig sliderConfigs[] = {
    {10000, 500, 0.1, true},  // LEFT_UP
    {10000, 500, 0.1, false},  // LEFT_DOWN
    {10000, 500, 0.1, true},   // RIGHT_UP
    {10000, 500, 0.1, true}    // RIGHT_DOWN
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
    {1000, 1000, 1, false, 1, 8, 305},   // LEFT_UP
    {1000, 1000, 1, false, 1, 8, 305},  // LEFT_DOWN
    {1000, 1000, 1, false, 1, 8, 305},     // RIGHT_UP
    {1000, 1000, 1, false, 1, 8, 305}    // RIGHT_DOWN
};

// {
//     int defaultDurationMs;
//     bool reverseDirection;
// };

FingeringMotorConfig fingeringConfigs[] = {
    {1000, false},  // LEFT_UP
    {1000, false},  // LEFT_DOWN
    {1000, true},   // RIGHT_UP
    {1000, true}    // RIGHT_DOWN
};
