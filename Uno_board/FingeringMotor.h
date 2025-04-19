#ifndef FINGERINGMOTOR_H
#define FINGERINGMOTOR_H

#include "Device.h"
#include "MotorConfig.h"
#include "FSMState.h"

class FingeringMotor : public Device {
public:
    FingeringMotor(int startPin, int motorID, const FingeringMotorConfig& config);

    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    void calibrate() override;
    void move();
    void moveBy(int durationMs, bool reverse = false) override;
    void update() override;
    bool isMovementComplete() override;

    void start();
    void stop();
    void up();
    void down();

    void moveForever();
    void stopRepeat();

private:
    void moveFor(unsigned long durationMs);

    int startPin, motorID;
    bool isRunning;
    DeviceState currentState;
    int defaultDurationMs;
    unsigned long movementStartTime;
    unsigned long movementDuration;

    bool repeating = false;
    bool isOnCycle = false;
    bool inPause = false;
    unsigned long lastToggleTime = 0;
    const unsigned long pulsePauseMs = 1000;  // Adjustable pause time between cycles
};

#endif // FINGERINGMOTOR_H
