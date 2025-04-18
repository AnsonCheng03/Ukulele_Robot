# Raspberry Pi Device Server with Bluetooth and I2C

This project runs a device server on a Raspberry Pi, enabling I2C device configuration and command reception over Bluetooth. It scans for I2C devices and sends motor control commands to an Arduino Uno or ESP32.

## Requirements

- Raspberry Pi with Bluetooth and I2C
- Connected Arduino Uno or ESP32 via I2C
- Bluetooth-enabled device to connect and send commands

## Setup

### 0. Flash Raspberry Pi Image

Set the user name to Pi

### 1. Get script

```bash
git clone https://github.com/AnsonCheng03/Ukulele_Robot.git
cd Ukulele_Robot/device_server
chmod +x ./script.sh
sudo ./script.sh
```
