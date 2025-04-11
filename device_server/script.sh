#!/bin/bash

# Raspberry Pi Device Server Setup Script

# 1. Enable I2C and Bluetooth on Raspberry Pi
echo "Enabling I2C and Bluetooth..."
sudo raspi-config nonint do_i2c 0

# 2. Install necessary libraries
echo "Updating package lists and installing necessary libraries..."
sudo apt update
sudo apt install -y i2c-tools python3-smbus bluetooth bluez python3-serial libbluetooth-dev git build-essential libglib2.0-dev

# 3. Install Python Bluetooth libraries
echo "Cloning and installing bluepy..."
git clone https://github.com/IanHarvey/bluepy.git
cd bluepy
sudo python3 setup.py build
sudo python3 setup.py install
cd ..

# Install Bluedot and PyBluez
echo "Installing Bluedot and PyBluez..."
sudo pip3 install bluedot git+https://github.com/pybluez/pybluez.git#egg=pybluez --break-system-packages

# 4. Install Raspberry Pi Bluetooth Manager
echo "Cloning and installing Raspberry Pi Bluetooth Manager..."
git clone https://github.com/cbusuioceanu/Raspberry-Pi-Bluetooth-Manager.git rpibtman
cd rpibtman
sudo bash rpibtman.sh
cd ..

# 5. Update Bluetooth configuration
echo "Updating Bluetooth configuration..."
sudo sed -i '/^\[General\]/a AutoEnable=true\nDiscoverableTimeout=0\nAlwaysPairable=true\nJustWorksRepairing=always' /etc/bluetooth/main.conf
sudo sed -i '/^\[Policy\]/a AutoPair=true' /etc/bluetooth/main.conf

# 6. Update machine name
echo "Updating machine name to GuitarRobot..."
echo "PRETTY_HOSTNAME=GuitarRobot" | sudo tee /etc/machine-info > /dev/null

# 7. Add main.py to run on startup
echo "Adding ./code/main.py to run on startup..."
echo "sudo python3 /home/pi/code/main.py" | sudo tee -a /etc/rc.local > /dev/null

# Finish
echo "Setup complete. Please reboot the Raspberry Pi for all changes to take effect."

# Prompt for reboot
read -p "Do you want to reboot now? (y/n): " REBOOT_NOW
if [ "$REBOOT_NOW" = "y" ]; then
    sudo reboot
fi
