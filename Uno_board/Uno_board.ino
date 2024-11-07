#include <Wire.h>
#include <Servo.h>

#define boardAddress 8 // Define the I2C address of this slave

#define slider_start_pin 9
#define slider_direction_pin 10
#define slider_speed_pin 11
#define slider_sensor_pin 12

#define motor_start_pin 4
#define motor_direction_pin 5
#define motor_speed_pin 6

#define CMD_CONFIG 1
#define CMD_CONTROL 2
#define CMD_CALIBRATE 3

Servo motor; // Servo motor object
 
float speed_hz, duration;
byte direction;

void setup()
{
    Serial.begin(9600); // Start serial communication for debugging
    Serial.println("Starting motor controller");

    pinMode(slider_start_pin, OUTPUT);
    pinMode(slider_direction_pin, OUTPUT);
    pinMode(slider_speed_pin, OUTPUT);
    pinMode(slider_sensor_pin, INPUT);
    pinMode(motor_start_pin, OUTPUT);
    pinMode(motor_direction_pin, OUTPUT);
    pinMode(motor_speed_pin, OUTPUT);

    preloadMotor();

    // Check sensor condition and calibrate before initializing Wire communication
    calibrateSensor();

    // Initialize communication
    Wire.begin(boardAddress); // Assuming board address 8, change it to your board address
    Wire.onReceive(receiveEvent);

    // Start the motor controller in idle state
    Serial.println("Starting broadcast");
}

void loop()
{
    // Loop is empty as all actions are triggered by I2C commands
}

void preloadMotor()
{
    digitalWrite(slider_start_pin, LOW);
    digitalWrite(motor_start_pin, LOW);
    analogWrite(slider_speed_pin, 1000);
    analogWrite(motor_speed_pin, 1000);
    digitalWrite(slider_direction_pin, LOW);
    digitalWrite(motor_direction_pin, LOW);
    delay(1000);
    digitalWrite(slider_direction_pin, HIGH);
    digitalWrite(motor_direction_pin, HIGH);
    delay(1000);
    digitalWrite(slider_start_pin, HIGH);
    digitalWrite(motor_start_pin, HIGH);
}

// Function to calibrate the motor or slider

void receiveEvent(int bytes) {
    // Buffer to store incoming data
    uint8_t buffer[32]; // Adjust size as needed
    int index = 0;

    // Read incoming data into buffer
    while (Wire.available() && index < sizeof(buffer)) {
        buffer[index++] = Wire.read();
    }

    // Print received data for debugging
    Serial.print("Received data: ");
    for (int i = 0; i < index; i++) {
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }
    Serial.println();

    // Process command
    switch (buffer[0]) {
        case CMD_CONFIG:
            sendConfig();
            break;

        case CMD_CONTROL:
            if (index >= 8) { // Ensure enough bytes for CONTROL command
                uint8_t target = buffer[1]; // 1 for slider, 2 for motor
                uint16_t speed_hz = (buffer[2] << 8) | buffer[3]; // 2 bytes for speed
                uint32_t duration = ((uint32_t)buffer[4] << 24) | ((uint32_t)buffer[5] << 16) |
                                    ((uint32_t)buffer[6] << 8) | buffer[7]; // 4 bytes for duration
                uint8_t direction = buffer[8]; // 1 byte for direction

                // Print control details
                Serial.print("Controlling target: ");
                Serial.print(target == 1 ? "slider" : "motor");
                Serial.print(", Direction: ");
                Serial.print(direction);
                Serial.print(", Speed: ");
                Serial.print(speed_hz);
                Serial.print(", Duration: ");
                Serial.println(duration);

                // Control the target
                if (target == 1) {
                    controlSlider();
                } else if (target == 2) {
                    controlMotor();
                } else {
                    Serial.println("Unknown target specified.");
                }
            } else {
                Serial.println("Not enough data for CONTROL.");
            }
            break;

        case CMD_CALIBRATE:
            calibrateSensor();
            break;

        default:
            Serial.println("Unknown command.");
            break;
    }
}

// Function to send configuration of ports
void sendConfig() {
    uint8_t configData[6]; // Array to hold configuration data for transmission

    // Fill in the configuration data
    configData[0] = slider_start_pin;
    configData[1] = slider_speed_pin;
    configData[2] = slider_sensor_pin;
    configData[3] = motor_start_pin;
    configData[4] = motor_speed_pin;
    configData[5] = motor_direction_pin;

    // Debugging output
    Serial.print("Slider Start Pin: ");
    Serial.println(configData[0]);
    Serial.print("Slider Speed Pin: ");
    Serial.println(configData[1]);
    Serial.print("Slider Sensor Pin: ");
    Serial.println(configData[2]);
    Serial.print("Motor Start Pin: ");
    Serial.println(configData[3]);
    Serial.print("Motor Speed Pin: ");
    Serial.println(configData[4]);
    Serial.print("Motor Direction Pin: ");
    Serial.println(configData[5]);

    // Begin transmission to the master device (assuming the master address is known)
    // Wire.beginTransmission(0x01); // Replace with actual I2C address if needed
    // Wire.write(configData, sizeof(configData)); // Send configuration data as bytes
    // Wire.endTransmission();

    Serial.println("Configuration data sent");

}

void calibrateSensor()
{
    Serial.println("Calibrating...");

    // Check if sensor is set for slider calibration
    if (digitalRead(slider_sensor_pin) == HIGH)
    {
        while (digitalRead(slider_sensor_pin) == HIGH)
        {
            analogWrite(slider_speed_pin, 500); // Move motor backward for calibration
            digitalWrite(slider_direction_pin, LOW);
            delay(100); // Short delay during calibration
        }
        digitalWrite(slider_start_pin, HIGH); // Mark calibration complete
        Serial.println("Slider calibration complete.");
    }
    else
    {
        Serial.println("Sensor not activated for slider calibration.");
    }

    // If you need motor calibration as well, you can add similar logic here
}

// Function to control the slider motor
void controlSlider()
{
    digitalWrite(slider_direction_pin, direction);
    analogWrite(slider_speed_pin, speed_hz); // Control speed using PWM
    digitalWrite(slider_start_pin, HIGH);    // Start motor
    delay(duration * 1000);                  // Wait for the duration
    digitalWrite(slider_start_pin, LOW);     // Stop motor
}

// Function to control the motor
void controlMotor()
{
    digitalWrite(motor_direction_pin, direction);
    analogWrite(motor_speed_pin, speed_hz); // Control speed using PWM
    digitalWrite(motor_start_pin, HIGH);    // Start motor
    delay(duration * 1000);                 // Wait for the duration
    digitalWrite(motor_start_pin, LOW);     // Stop motor
}
