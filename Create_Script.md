## Device Server on Raspberry Pi
This project sets up a Raspberry Pi as a networked device server. It will:

Detect I2C devices connected to the Raspberry Pi.
Allow Bluetooth pairing mode to automatically accept incoming connections.
Enable command forwarding from connected Bluetooth devices to an Arduino Uno.
Requirements
Raspberry Pi with Bluetooth and I2C capabilities.
Arduino Uno connected via USB.
Bluetooth-enabled device (phone, laptop) to connect and send commands.
Setup Instructions
1. Enable I2C and Bluetooth on Raspberry Pi
Enable I2C:

Open the Raspberry Pi configuration tool:
```
sudo raspi-config
```
Navigate to Interfacing Options > I2C and enable it.
Install I2C and Bluetooth Tools:

Update packages and install necessary libraries:
```
sudo apt update
sudo apt install -y i2c-tools python3-smbus bluetooth bluez blueman python3-serial
```
2. Set Up Device Detection Script
Create Script Directory:

Navigate to the /home/pi/ directory and create the folder structure for the project:
```
mkdir -p /home/pi/pi_device_server
```
Save the Script:

Create and open the script file:

```
nano /home/pi/pi_device_server/device_server.py
```
Copy and paste the code ```device_server.py``` into the file:


Make the Script Executable:

```
chmod +x /home/pi/pi_device_server/device_server.py
```
3. Configure Bluetooth for Auto Pairing
Edit the Bluetooth configuration to allow automatic pairing and multiple connections:

Open the Bluetooth configuration file:

```
sudo nano /etc/bluetooth/main.conf
```
Add or edit the following settings:

```
[General]
AutoEnable=true

[Policy]
AutoPair=true
```
4. Running the Device Server Script
Run the script to start the device server:

```
python3 /home/pi/pi_device_server/device_server.py
```
The server will:

Detect I2C devices and print any connected devices matching those in the dictionary.
Enable Bluetooth pairing mode and accept incoming connections automatically.
Forward received commands to the Arduino Uno over USB.
5. Testing and Usage
Bluetooth Pairing: Use a Bluetooth-enabled device (phone, laptop) to pair with the Raspberry Pi. You should see connection messages printed to the console.
Sending Commands: Once connected, send commands from your Bluetooth device, and they will be forwarded to the Arduino Uno.


Troubleshooting
If you don’t see the expected I2C devices, double-check connections and I2C addresses.
To check paired Bluetooth devices and manage connections:
```
bluetoothctl
```
