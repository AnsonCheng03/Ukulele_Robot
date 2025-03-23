#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include <Servo.h>

class Device {
public:
    Device(int startPin, int directionPin, int speedPin, int motorID);
    void setup();
    void control(int direction, int speedHz, int durationTenths);
    void moveBy(int positionMm, bool reverse = false);
    void update();
    bool isMovementComplete();
    int getmotorID();
    
protected:
    int startPin, directionPin, speedPin, motorID;
    bool isMoving;
    bool isCalibrated;
    int currentPosition;
    int max_distance;
    int fixedMoveSpeed;
    unsigned long moveStartMillis;
    unsigned long moveDuration;
    double distanceToDurationRatio;

    void startMovement(unsigned long durationTenths);
    void stopMovement();
    void start();
    void stop();
    void setDirection(int direction);
};

#endif // DEVICE_H
