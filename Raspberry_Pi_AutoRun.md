# Raspberry Pi Device Server

This project sets up a Raspberry Pi server that detects reachable devices on the I2C network, starts a Bluetooth pairing mode, and facilitates communication with connected Arduino devices.

## Project Overview

- **Functionality**:
  - Detect all reachable devices on the I2C network and verify them against a stored dictionary.
  - Initiate a Bluetooth pairing mode to allow multi-device connections.
  - Enable command transmission from connected Bluetooth devices to Arduino via Raspberry Pi.

- **Service**:
  - A `systemd` service (`pi_device_server.service`) is set up to start this script automatically on boot.

## Directory Structure

The main script and its dependencies are located in `/home/pi/pi_device_server/`.

/home/pi/pi_device_server/ 
└── device_server.py # Main server script


## Installation and Setup

### 1. Create the Project Directory

```bash
mkdir -p /home/pi/pi_device_server
```

2. Place the Python Script in the Directory
Copy device_server.py (your main server script) into /home/pi/pi_device_server/.
3. Make the Script Executable
```
chmod +x /home/pi/pi_device_server/device_server.py
```
4. Create the systemd Service File
Create a new service file with:
```
sudo nano /etc/systemd/system/pi_device_server.service
```
Add the following content to the file:
```
[Unit]
Description=Raspberry Pi Device Server (I2C and Bluetooth)
After=network.target

[Service]
ExecStart=/usr/bin/python3 /home/pi/pi_device_server/device_server.py
WorkingDirectory=/home/pi/pi_device_server
StandardOutput=inherit
StandardError=inherit
Restart=always
User=pi

[Install]
WantedBy=multi-user.target
```
Save and close the file by pressing CTRL+X, then Y, and Enter.

5. Enable and Start the Service
Enable the service to start on boot:
```
sudo systemctl enable pi_device_server.service
```
Start the service immediately:
```
sudo systemctl start pi_device_server.service
```
Check the Status of the service:
```
sudo systemctl status pi_device_server.service
```
6. Reboot to Confirm
Reboot the Raspberry Pi to verify that the service starts automatically:
```
sudo reboot
```
After rebooting, confirm the service is running:
```
sudo systemctl status pi_device_server.service
```
Managing the Service
Start the Service
```
sudo systemctl start pi_device_server.service
```
Stop the Service
```
sudo systemctl stop pi_device_server.service
```
Restart the Service
```
sudo systemctl restart pi_device_server.service
```
Check the Service Status
```
sudo systemctl status pi_device_server.service
```

Notes
Logs: Logs for the service can be viewed with journalctl -u pi_device_server.service.
Modifications: If device_server.py is modified, restart the service with sudo systemctl restart pi_device_server.service.
