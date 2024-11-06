# Bluetooth Auto-Start and Periodic Discoverable Service for Linux

This tutorial explains how to set up a systemd service to automatically enable Bluetooth, make it discoverable, and pairable on startup, and ensure it stays discoverable every 1 minute.

## Prerequisites

A Linux system with Bluetooth support.
Sudo access to create system files.
Bluetooth service running on your system.

## Steps

1. Create the systemd service file
   Create a systemd service file to power on Bluetooth, make it discoverable, and pairable on startup:

`sudo nano /etc/systemd/system/bluetooth-autostart.service `

2. Add the following configuration

```
[Unit]
Description=Bluetooth Auto Start Service
After=bluetooth.target

[Service]
Type=simple
ExecStart=/usr/bin/bluetoothctl power on
ExecStartPost=/usr/bin/bluetoothctl discoverable on
ExecStartPost=/usr/bin/bluetoothctl pairable on
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
```

3. Create a timer to ensure Bluetooth stays discoverable
   Create a timer that triggers every 1 minute to make Bluetooth discoverable:

`sudo nano /etc/systemd/system/bluetooth-discoverable.timer`

- Add this content:

```
[Unit]
Description=Bluetooth Discoverable Timer

[Timer]
OnBootSec=1min
OnUnitActiveSec=1min
Unit=bluetooth-discoverable.service

[Install]
WantedBy=timers.target
```

4. Create the discoverable service

Create the service that will be triggered by the timer to make Bluetooth discoverable every minute:

`sudo nano /etc/systemd/system/bluetooth-discoverable.service`
Add the following content:

```
[Unit]
Description=Keep Bluetooth Discoverable

[Service]
ExecStart=/usr/bin/bluetoothctl discoverable on
```

5. Reload systemd and enable the services

After saving all files, reload systemd:

`sudo systemctl daemon-reload`

- Enable and start the Bluetooth services:

```
sudo systemctl enable bluetooth-autostart.service
sudo systemctl start bluetooth-autostart.service
sudo systemctl enable bluetooth-discoverable.timer
sudo systemctl start bluetooth-discoverable.timer
```

6. Verify the timer
   - To check if the timer is working correctly, run:

`sudo systemctl status bluetooth-discoverable.timer`

- This will confirm that Bluetooth is discoverable every minute.
