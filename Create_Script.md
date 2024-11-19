2. Set Up Device Detection Script

Create and open the script file:

```
nano /home/pi/pi_device_server/device_server.py
```

Follow the instructions in device_server and copy the code `device_server.py` into the file

3. Configure Bluetooth for Auto Pairing
   Edit the Bluetooth configuration to allow automatic pairing and multiple connections:

4. Running the Device Server Script
   Run the script to start the device server:

```
python3 /home/pi/pi_device_server/device_server.py
```

The server will:

Detect I2C devices and print any connected devices matching those in the dictionary.
Enable Bluetooth pairing mode and accept incoming connections automatically.
Forward received commands to the Arduino Uno over USB. 5. Testing and Usage
Bluetooth Pairing: Use a Bluetooth-enabled device (phone, laptop) to pair with the Raspberry Pi. You should see connection messages printed to the console.
Sending Commands: Once connected, send commands from your Bluetooth device, and they will be forwarded to the Arduino Uno.

Troubleshooting
If you don’t see the expected I2C devices, double-check connections and I2C addresses.
To check paired Bluetooth devices and manage connections:

```
bluetoothctl
```
