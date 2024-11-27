#include <Wire.h>
#include <Servo.h>

#define boardAddress 5
#define CMD_CONTROL 0
#define CMD_CALIBRATE 1
#define CMD_MOVE 2
#define CMD_FINGERING 3
#define CMD_CHORD 4

Servo servoMotor;

class Device {
public:
    Device(int startPin, int directionPin, int speedPin)
        : startPin(startPin), directionPin(directionPin), speedPin(speedPin), isMoving(false), moveStartMillis(0), moveDuration(0) {}

    void setup() {
        pinMode(startPin, OUTPUT);
        pinMode(directionPin, OUTPUT);
        pinMode(speedPin, OUTPUT);
        Serial.println("Device setup for pins: " + String(startPin) + ", " + String(directionPin) + ", " + String(speedPin));
    }

    void control(int direction, int speedHz, int durationTenths) {
        Serial.println("Control device - Direction: " + String(direction) + ", Speed: " + String(speedHz) + ", Duration: " + String(durationTenths * 0.1) + "s");
        start();
        analogWrite(speedPin, speedHz);
        setDirection(direction);
        startMovement(durationTenths * 100);
    }

    void update() {
        if (isMoving && millis() - moveStartMillis >= moveDuration) {
            stopMovement();
        }
    }

protected:
    int startPin, directionPin, speedPin;
    bool isMoving;
    unsigned long moveStartMillis;
    unsigned long moveDuration;

    void startMovement(unsigned long duration) {
        isMoving = true;
        moveStartMillis = millis();
        moveDuration = duration;
        Serial.println("Movement started for duration: " + String(duration * 0.1) + "s");
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
        if (digitalRead(sensorPin) == HIGH) {
            isCalibrating = true;
            analogWrite(speedPin, 500);
            setDirection(LOW);
            startMovement(100);
            Serial.println("Calibration started: Moving slider backward");
        } else {
            Serial.println("Sensor not activated for calibration.");
        }
    }

    void update() {
        Device::update();
        if (isCalibrating && !isMoving && digitalRead(sensorPin) == HIGH) {
            startMovement(100);
            Serial.println("Continuing calibration movement...");
        } else if (isCalibrating && digitalRead(sensorPin) == LOW) {
            isCalibrating = false;
            stop();
            Serial.println("Slider calibration complete.");
        }
    }

private:
    int sensorPin;
    bool isCalibrating;
};

Slider slider(9, 10, 11, 12);
Device motorDevice(4, 5, 6);

void setup() {
    Serial.begin(9600);
    Serial.println("Board address: " + String(boardAddress));
    slider.setup();
    motorDevice.setup();
    Wire.begin(boardAddress);
    Wire.onReceive(receiveEvent);
    Serial.println("Broadcast started");
}

void loop() {
    slider.update();
    motorDevice.update();
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
                uint16_t speedHz = buffer[2] | (buffer[3] << 8);
                uint32_t durationTenths = buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
                uint8_t direction = buffer[8];

                Serial.println("CMD_CONTROL: Target = " + String(target) + ", Speed = " + String(speedHz) + ", Duration = " + String(durationTenths * 0.1) + "s, Direction = " + String(direction));

                if (target == 1) {
                    slider.control(direction, speedHz, durationTenths);
                } else if (target == 2) {
                    motorDevice.control(direction, speedHz, durationTenths);
                } else {
                    Serial.println("Unknown target.");
                }
            } else {
                Serial.println("Not enough data for CONTROL.");
            }
            break;

        case CMD_CALIBRATE:
            if (index >= 2) {
                uint8_t target = buffer[1];
                Serial.println("CMD_CALIBRATE: Target = " + String(target));
                if (target == 1) {
                    slider.calibrate();
                } else if (target == 2) {
                    Serial.println("No calibration implemented for motorDevice.");
                } else {
                    Serial.println("Unknown target for CALIBRATE.");
                }
            } else {
                Serial.println("Not enough data for CALIBRATE.");
            }
            break;

        case CMD_MOVE:
            Serial.println("CMD_MOVE received");
            // Implementation for CMD_MOVE will be added later
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
