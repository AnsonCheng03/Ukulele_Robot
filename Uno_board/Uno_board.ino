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

void receiveEvent(int bytes)
{
    // Read the incoming command into a buffer
    char command[32]; // Adjust the size as needed
    int index = 0;

    while (Wire.available() && index < sizeof(command) - 1)
    {
        command[index++] = Wire.read();
    }
    command[index] = '\0'; // Null-terminate the string

    // Print the received command for debugging
    Serial.print("Received command: ");
    Serial.println(command);

    if (strncmp(command, "CONFIG", 6) == 0)
    {
        // Send the configuration for all ports
        sendConfig();
    }
    else if (strncmp(command, "CONTROL,", 8) == 0)
    {
        // Parse the control command to select either slider or motor
        if (bytes >= 8)
        {                                           // CONTROL + 1 type + 1 speed + 1 direction + 4 duration
            char *token = strtok(command + 8, ","); // Start reading after "CONTROL,"
            if (token)
            {
                // Parse control target (slider or motor)
                char *target = token;
                token = strtok(NULL, ","); // speed
                if (token)
                    speed_hz = atoi(token);

                token = strtok(NULL, ","); // duration
                if (token)
                    duration = atof(token);

                token = strtok(NULL, ","); // direction
                if (token)
                    direction = atoi(token);

                // Print control details
                Serial.print("Controlling ");
                Serial.print(target);
                Serial.print(" - ");
                Serial.print("Direction: ");
                Serial.print(direction);
                Serial.print(", Speed: ");
                Serial.print(speed_hz);
                Serial.print(", Duration: ");
                Serial.println(duration);

                // Control either slider or motor based on the target
                if (strcmp(target, "slider") == 0)
                {
                    controlSlider();
                }
                else if (strcmp(target, "motor") == 0)
                {
                    controlMotor();
                }
                else
                {
                    Serial.println("Unknown target specified.");
                }
            }
            else
            {
                Serial.println("Not enough parameters for CONTROL.");
            }
        }
        else
        {
            Serial.println("Not enough bytes for CONTROL.");
        }
    }
    else if (strncmp(command, "CALIBRATE", 9) == 0)
    {
        calibrateSensor();
    }
    else
    {
        Serial.println("Sensor not activated for calibration.");
    }
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

// Function to send configuration of ports
void sendConfig()
{
    Serial.println("Sending configuration...");

    // Send slider configuration
    Serial.print("Slider Config - ");
    Serial.print("Start Pin: ");
    Serial.print(slider_start_pin);
    Serial.print(", Speed Pin: ");
    Serial.print(slider_speed_pin);
    Serial.print(", Sensor Pin: ");
    Serial.println(slider_sensor_pin);

    // Send motor configuration
    Serial.print("Motor Config - ");
    Serial.print("Start Pin: ");
    Serial.print(motor_start_pin);
    Serial.print(", Speed Pin: ");
    Serial.print(motor_speed_pin);
    Serial.print(", Direction Pin: ");
    Serial.println(motor_direction_pin);
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
