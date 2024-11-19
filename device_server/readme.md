# Raspberry Pi Device Server with Bluetooth and I2C

This project runs a device server on a Raspberry Pi, enabling I2C device configuration and command reception over Bluetooth. It scans for I2C devices and sends motor control commands to an Arduino Uno or ESP32.

## Requirements

- Raspberry Pi with Bluetooth and I2C
- Connected Arduino Uno or ESP32 via I2C
- Bluetooth-enabled device to connect and send commands

## Setup

### 0. Flash Raspberry Pi Image

Set the user name to Pi

### 1. Enable I2C and Bluetooth on Raspberry Pi

```bash
sudo raspi-config
```

Choose Interface Options
Choose I2C
Then enable.

### 2. Install necessary libraries: (Make sure you are connected to internet)

```bash
sudo apt update
sudo apt install -y i2c-tools python3-smbus bluetooth bluez blueman python3-serial libbluetooth-dev git build-essential libglib2.0-dev
git clone https://github.com/IanHarvey/bluepy.git
cd bluepy
sudo python3 setup.py build
sudo python3 setup.py install
sudo pip3 install bluedot git+https://github.com/pybluez/pybluez.git#egg=pybluez --break-system-packages
cd ../
git clone https://github.com/cbusuioceanu/Raspberry-Pi-Bluetooth-Manager.git rpibtman && cd rpibtman && sudo bash rpibtman.sh
```

### 3. Setup Bluetooth

#### 1. Edit Bluetooth configuration file

```bash
sudo nano /etc/bluetooth/main.conf
```

Add or edit the following settings:

```md
[General]
AutoEnable=true
DiscoverableTimeout = 0
AlwaysPairable = true
JustWorksRepairing = always

[Policy]
AutoPair=true
```

#### 2. Edit the machine name

Goto `sudo nano /etc/machine-info`

```md
PRETTY_HOSTNAME=GuitarRobot
```

### 4. Change Startup Settings

#### 1. Set bluetooth

```bash
sudo /usr/bin/btmgmt io-cap 3
```
