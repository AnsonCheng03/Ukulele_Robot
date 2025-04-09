#ifndef FINGERINGMOTOR_H
#define FINGERINGMOTOR_H

#include "Device.h"

class FingeringMotor : public Device {
public:
    FingeringMotor(int startPin, int motorID);
    int getMotorID() override;

    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    void calibrate() override;
    void move();
    void moveBy(int durationMs, bool reverse = false) override; // durationMs instead of positionMm
    void update() override;
    bool isMovementComplete() override;

    void start();
    void stop();
    void up();
    void down();

private:
    void moveFor(unsigned long durationMs);

    int startPin, motorID;
    bool isRunning;
    DeviceState currentState;

    unsigned long movementStartTime;
    unsigned long movementDuration;
};

#endif // FINGERINGMOTOR_H
