# Bluetooth Auto-Start Service for Linux

This tutorial explains how to set up a systemd service to automatically enable Bluetooth, make it discoverable, and pairable on startup.

## Prerequisites

A Linux system with Bluetooth support.
Sudo access to create system files.
Bluetooth service running on your system.

## Steps

1. Create the systemd service file
   Open a terminal and create a systemd service file:

`sudo nano /etc/systemd/system/bluetooth-autostart.service` 2. Add the following configuration
Copy and paste the following configuration into the file:

```
[Unit]
Description=Bluetooth Auto Start Service
After=bluetooth.target

[Service]
Type=oneshot
ExecStart=/usr/bin/bluetoothctl power on
ExecStartPost=/usr/bin/bluetoothctl discoverable on
ExecStartPost=/usr/bin/bluetoothctl pairable on
RemainAfterExit=true

[Install]
WantedBy=multi-user.target
```

3. Reload systemd and enable the service
   After saving the file, reload systemd to apply the changes:

`sudo systemctl daemon-reload`

- Enable the service to start on boot:

`sudo systemctl enable bluetooth-autostart.service` 4. Start the service

- Start the service immediately to apply the changes:

`sudo systemctl start bluetooth-autostart.service` 5. Verify the service

- To check the status of the service, use the following command:

`sudo systemctl status bluetooth-autostart.service`

It should show that the service is active and that the Bluetooth settings have been applied.

## Conclusion

With this setup, your Bluetooth will automatically power on, become discoverable, and pairable on startup. The systemd service ensures that these settings persist even after a reboot.
