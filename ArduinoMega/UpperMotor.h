#ifndef UPPERMOTOR_H
#define UPPERMOTOR_H

#include "Device.h"
#include "MotorConfig.h"
#include "FSMState.h"

class UpperMotor : public Device {
public:
    UpperMotor(int startPin, int directionPin, int speedPin, int motorID, const UpperMotorConfig& config);
    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    virtual void move(int positionMm) = 0;
    void moveBy(int positionMm, bool reverse = false) override;
    void update() override;
    bool isMovementComplete() override;
    void moveUntilTouchSensor(bool toward = true);

protected:
    int startPin, directionPin, speedPin, motorID;
    bool isCalibrated;
    int currentPosition;
    int max_distance;
    int fixedMoveSpeed;
    unsigned long moveStartMillis;
    unsigned long moveDuration;
    double distanceToDurationRatio;
    DeviceState currentState;
    DeviceState trueState;
    bool reverseDirection;
    int currentDirectionSignal; 
    bool movingIndefinitely = false; 

    void startMovement(unsigned long durationTenths);
    void stopMovement();
    void start();
    void stop();
    void setDirection(int direction);
};

#endif // UPPERMOTOR_H