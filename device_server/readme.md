# Raspberry Pi Device Server with Bluetooth and I2C

This project runs a device server on a Raspberry Pi, enabling I2C device configuration and command reception over Bluetooth. It scans for I2C devices and sends motor control commands to an Arduino Uno or ESP32.

## Requirements

- Raspberry Pi with Bluetooth and I2C
- Connected Arduino Uno or ESP32 via I2C
- Bluetooth-enabled device to connect and send commands

# Setup

## 1. Enable I2C and Bluetooth on Raspberry Pi

```bash
sudo raspi-config
```

### Enable I2C under Interfacing Options

Install necessary libraries:

```bash
sudo apt update
sudo apt install -y i2c-tools python3-smbus bluetooth bluez blueman python3-serial libbluetooth-dev
pip3 install pybluez
```

## 2. Save and Run Script

### Create Script Directory

```bash
mkdir -p /home/pi/pi_device_server
```

### Save Script

Save the script to `/home/pi/pi_device_server/device_server.py`
Make Executable:

```bash
chmod +x /home/pi/pi_device_server/device_server.py
```

### Run Script

```bash
python3 /home/pi/pi_device_server/device_server.py
```

## 3. Bluetooth Pairing and Command Format

Use any Bluetooth-enabled device to pair and send commands to the Raspberry Pi.

Command Format
Use the format: (address, Speed-Hz, 1/0, second)
Examples:

8, 100, 1, 5 - Send a command to device at address 8 to run at 100 Hz speed for 5 seconds in forward direction.
Troubleshooting
Device Detection: Ensure all I2C devices are properly connected.
Bluetooth: Use bluetoothctl to manage devices if needed.

## Auto update script

curl https://raw.githubusercontent.com/AnsonCheng03/CSCI_FYP/refs/heads/read_me/device_server/device_server.py -o /home/pi/pi_device_server/device_server.py && chmod +x /home/pi/pi_device_server/device_server.py
