#include <Wire.h>
#include <Servo.h>

#define boardAddress 5
#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_FINGERING 3
#define CMD_CHORD 4

#define SLIDER_START_PIN 9
#define SLIDER_DIRECTION_PIN 10
#define SLIDER_SPEED_PIN 11
#define SLIDER_SENSOR_PIN 12

#define RACK_START_PIN 4
#define RACK_DIRECTION_PIN 5
#define RACK_SPEED_PIN 6

#define FIXED_MOVE_SPEED_HZ 1000
#define DISTANCE_TO_DURATION_RATIO 0.01 // Updated ratio: 0.05 seconds per mm
#define MAX_DISTANCE_MM 1000000 // Maximum allowed distance in mm (example large number)

Servo servoMotor;

class Device {
public:
    Device(int startPin, int directionPin, int speedPin)
        : startPin(startPin), directionPin(directionPin), speedPin(speedPin), isMoving(false), moveStartMillis(0), moveDuration(0), isCalibrated(false), currentPosition(0) {}

    void setup() {
        pinMode(startPin, OUTPUT);
        pinMode(directionPin, OUTPUT);
        pinMode(speedPin, OUTPUT);
        Serial.println("Device setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
    }

    void control(int direction, int speedHz, int durationTenths) {
        Serial.println("Control device - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
        isCalibrated = false; // Control operation makes the device uncalibrated
        analogWrite(speedPin, speedHz);
        setDirection(direction);
        startMovement(durationTenths);
    }

    void move(int distanceMm) {
        if (!isCalibrated) {
            Serial.println("Device not calibrated. Cannot move.");
            return;
        }
        if (abs(distanceMm) > MAX_DISTANCE_MM) {
            Serial.println("Requested distance exceeds maximum allowed distance. Cannot move.");
            return;
        }
        int direction = distanceMm >= 0 ? HIGH : LOW;
        int distanceAbs = abs(distanceMm);
        unsigned long durationTenths = distanceAbs * DISTANCE_TO_DURATION_RATIO * 100;
        Serial.println("Move device - Distance: " + String(distanceMm) + "mm, Direction: " + String(direction) + ", Duration: " + String(durationTenths * 0.1) + "s");
        setDirection(direction);
        analogWrite(speedPin, FIXED_MOVE_SPEED_HZ); // Fixed speed for movement
        startMovement(durationTenths);
        currentPosition += distanceMm;
    }

    void update() {
        if (isMoving && millis() - moveStartMillis >= moveDuration) {
            stopMovement();
        }
    }

protected:
    int startPin, directionPin, speedPin;
    bool isMoving;
    bool isCalibrated;
    int currentPosition;
    unsigned long moveStartMillis;
    unsigned long moveDuration;

    void startMovement(unsigned long durationTenths) {
        start();
        isMoving = true;
        moveStartMillis = millis();
        moveDuration = durationTenths * 100;
        Serial.println("Movement started for duration: " + String(durationTenths * 0.1) + "s");
    }

    void stopMovement() {
        stop();
        isMoving = false;
        Serial.println("Device stopped at pin: " + String(startPin));
    }

    void start() {
        digitalWrite(startPin, LOW);
        Serial.println("Start pin " + String(startPin) + " set to LOW");
    }

    void stop() {
        digitalWrite(startPin, HIGH);
        Serial.println("Start pin " + String(startPin) + " set to HIGH");
    }

    void setDirection(int direction) {
        digitalWrite(directionPin, direction);
        Serial.println("Direction pin " + String(directionPin) + " set to " + String(direction));
    }
};

class Slider : public Device {
public:
    Slider(int startPin, int directionPin, int speedPin, int sensorPin)
        : Device(startPin, directionPin, speedPin), sensorPin(sensorPin), isCalibrating(false) {}

    void setup() {
        Device::setup();
        pinMode(sensorPin, INPUT);
        Serial.println("Slider setup for sensor pin: " + String(sensorPin));
    }

    void calibrate() {
        Serial.println("Calibrating slider...");
        isCalibrating = true;
        // analogWrite(speedPin, 500);
        // setDirection(LOW);
        // startMovement(100);
        // Serial.println("Calibration started: Moving slider backward");
        // // Assume calibration is done successfully for now
        isCalibrating = false;
        isCalibrated = true;
        currentPosition = 0;
        Serial.println("Slider calibration complete.");
    }

    void update() {
        Device::update();
        if (isCalibrating && !isMoving) {
            isCalibrating = false;
            isCalibrated = true;
            currentPosition = 0;
            stop();
            Serial.println("Slider calibration complete.");
        }
    }

private:
    int sensorPin;
    bool isCalibrating;
};

class RackMotor : public Device {
public:
    RackMotor(int startPin, int directionPin, int speedPin)
        : Device(startPin, directionPin, speedPin) {}

    void calibrate() {
        Serial.println("Calibrating rack motor...");
        setDirection(HIGH);
        analogWrite(speedPin, 500);
        startMovement(5);
        Serial.println("Calibration started: Moving rack motor");
        // Assume calibration is done successfully for now
        isCalibrated = true;
        currentPosition = 0;
        Serial.println("Rack motor calibration complete.");
    }
};

Slider slider(SLIDER_START_PIN, SLIDER_DIRECTION_PIN, SLIDER_SPEED_PIN, SLIDER_SENSOR_PIN);
RackMotor rackMotor(RACK_START_PIN, RACK_DIRECTION_PIN, RACK_SPEED_PIN);

void setup() {
    Serial.begin(9600);
    Serial.println("Board address: " + String(boardAddress));
    slider.setup();
    rackMotor.setup();
    slider.calibrate();
    rackMotor.calibrate();
    Wire.begin(boardAddress);
    Wire.onReceive(receiveEvent);
    Serial.println("Broadcast started");
}

void loop() {
    slider.update();
    rackMotor.update();
    delay(50);
}

void receiveEvent(int bytes) {
    uint8_t buffer[32];
    int index = 0;

    while (Wire.available() && index < sizeof(buffer)) {
        buffer[index++] = Wire.read();
    }

    Serial.print("Received data: ");
    for (int i = 0; i < index; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    switch (buffer[0]) {
        case CMD_CONTROL:
            if (index >= 9) {
                uint8_t target = buffer[1];
                uint16_t speedHz = (buffer[2] << 8) | buffer[3];
                uint32_t durationTenths = ((uint32_t)buffer[4] << 24) | ((uint32_t)buffer[5] << 16) | ((uint32_t)buffer[6] << 8) | buffer[7];
                uint8_t direction = buffer[8];

                Serial.println("CMD_CONTROL: Target = " + String(target) + ", Speed = " + String(speedHz) + ", Duration = " + String(durationTenths * 0.1) + "s, Direction = " + String(direction));

                if (target == 0 || target == 1) {
                    slider.control(direction, speedHz, durationTenths);
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.control(direction, speedHz, durationTenths);
                } 
            } else {
                Serial.println("Not enough data for CONTROL.");
            }
            break;

        case CMD_CALIBRATE:
            if (index >= 2) {
                uint8_t target = buffer[1];
                Serial.println("CMD_CALIBRATE: Target = " + String(target));
                if (target == 0 || target == 1) {
                    slider.calibrate();
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.calibrate();
                } 
            } else {
                Serial.println("Not enough data for CALIBRATE.");
            }
            break;

        case CMD_MOVE:
            if (index >= 6) {
                uint8_t target = buffer[1];
                int32_t distanceMm = ((int32_t)buffer[2] << 24) | ((int32_t)buffer[3] << 16) | ((int32_t)buffer[4] << 8) | buffer[5];
                Serial.println("CMD_MOVE: Target = " + String(target) + ", Distance = " + String(distanceMm) + "mm");
                if (target == 0 || target == 1) {
                    slider.move(distanceMm);
                } 
                
                if (target == 0 || target == 2) {
                    rackMotor.move(distanceMm);
                } 
            } else {
                Serial.println("Not enough data for MOVE.");
            }
            break;

        case CMD_FINGERING:
            Serial.println("CMD_FINGERING received");
            // Implementation for CMD_FINGERING will be added later
            break;

        case CMD_CHORD:
            Serial.println("CMD_CHORD received");
            // Implementation for CMD_CHORD will be added later
            break;

        default:
            Serial.println("Unknown command.");
            break;
    }
}
