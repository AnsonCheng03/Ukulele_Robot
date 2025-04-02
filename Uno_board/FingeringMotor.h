#ifndef FINGERINGMOTOR_H
#define FINGERINGMOTOR_H

#include "Device.h"
#include "JobQueue.h"

class FingeringMotor : public Device {
public:
    FingeringMotor(int startPin, int motorID);
    int getMotorID() override;

    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    void calibrate() override;
    void move();
    void moveBy(int positionMm, bool reverse = false) override;
    void update() override;
    bool isMovementComplete() override;

    void start();
    void stop();
    void up();
    void down();

private:
    int startPin, motorID;
    bool isRunning;
    DeviceState currentState;
};

#endif // FINGERINGMOTOR_H
