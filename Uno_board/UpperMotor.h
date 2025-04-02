#ifndef UPPERMOTOR_H
#define UPPERMOTOR_H

#include "Device.h"

class UpperMotor : public Device {
public:
    UpperMotor(int startPin, int directionPin, int speedPin, int motorID);
    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    virtual void move(int positionMm) = 0;
    void moveBy(int positionMm, bool reverse = false) override;
    void update() override;
    bool isMovementComplete() override;
    int getMotorID() override;

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

    void startMovement(unsigned long durationTenths);
    void stopMovement();
    void start();
    void stop();
    void setDirection(int direction);
};

#endif // UPPERMOTOR_H