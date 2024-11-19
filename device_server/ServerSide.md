1.

from bluedot.btcomm import BluetoothServer
from signal import pause

def data_received(data):
print(data)
s.send(data)

s = BluetoothServer(data_received)
pause()

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

```
curl https://raw.githubusercontent.com/AnsonCheng03/CSCI_FYP/refs/heads/read_me/device_server/device_server.py -o /home/pi/pi_device_server/device_server.py && chmod +x /home/pi/pi_device_server/device_server.py
```
