#ifndef FINGERINGMOTOR_H
#define FINGERINGMOTOR_H

#include "Device.h"
#include "MotorConfig.h"

class FingeringMotor : public Device {
public:
    FingeringMotor(int startPin, int motorID, const FingeringMotorConfig& config);

    void setup() override;
    void control(int direction, int speedHz, int durationTenths) override;
    void calibrate() override;
    void move();
    void moveBy(int duration, bool reverse = false) override;
    void update() override;
    bool isMovementComplete() override;
    void start(); 
    void stop();  

    void pulse(unsigned int durationMicros);
    void moveForever();
    void stopRepeat();

private:
    int startPin;
    int motorID;
    int defaultDurationMs;

    // Repeat mode toggling
    bool repeating = false;
    bool isOnCycle = false;
    bool inPause = false;
    unsigned long lastToggleMicros = 0;
    const unsigned long pulsePauseMicros = 1000000UL; // 1s pause by default
};

#endif // FINGERINGMOTOR_H
