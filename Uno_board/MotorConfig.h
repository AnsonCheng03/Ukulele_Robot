#ifndef MOTOR_CONFIG_H
#define MOTOR_CONFIG_H

struct UpperMotorConfig {
    int maxDistance;
    int fixedMoveSpeed;
    double distanceToDurationRatio;
    bool reverseDirection;

    int normalDownDistance;       
    int extendedDownDistance;    
    int measurementThreshold;   
};


struct FingeringMotorConfig {
    int defaultDurationMs;
    bool reverseDirection;
};

extern UpperMotorConfig sliderConfigs[];
extern UpperMotorConfig rackConfigs[];
extern FingeringMotorConfig fingeringConfigs[];

#endif // MOTOR_CONFIG_H
