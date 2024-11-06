import smbus
import serial
import bluetooth
import time

# Dictionary of I2C slaves with motor configurations
slaves = {
    8: {"Name": "Left First Motor", "Speed_Pin": 9, "Direction_Pin": 10, "Start_Pin": 11, "Sensor_Pin": None},
    9: {"Name": "Right First Motor", "Speed_Pin": 8, "Direction_Pin": 9, "Start_Pin": 10, "Sensor_Pin": 11},
    # Additional devices can be added here
}

# Initialize I2C bus (1 for Raspberry Pi)
i2c_bus = smbus.SMBus(1)

# Scan for I2C devices and configure found devices
def scan_i2c():
    print("Scanning for I2C devices...")
    detected_devices = i2c_bus.scan()
    
    if not detected_devices:
        print("No I2C devices found.")
    else:
        print(f"Found {len(detected_devices)} I2C devices:")
        for device in detected_devices:
            name = slaves.get(device, {}).get("Name", "Unknown")
            print(f"Device {name} found at address: {device}")

    return detected_devices

def send_motor_config(slave_address, motor):
    config_data = f"{motor['Speed_Pin']},{motor['Direction_Pin']},{motor['Start_Pin']},{motor['Sensor_Pin'] if motor['Sensor_Pin'] else 0}"
    try:
        i2c_bus.writeto(slave_address, b'CONFIG,' + config_data.encode())
        print(f"Config sent to {motor['Name']} at address {slave_address}")
    except OSError as e:
        print(f"Failed to communicate with {motor['Name']} (address {slave_address}): {e}")

# Send motor control commands
def send_motor_command(slave_address, speed, direction, duration):
    control_data = f"{speed},{duration},{direction}"
    try:
        i2c_bus.writeto(slave_address, b'CONTROL,' + control_data.encode())
        print(f"Command sent to {slaves[slave_address]['Name']} at address {slave_address}")
    except OSError as e:
        print(f"Failed to communicate with {slaves[slave_address]['Name']} (address {slave_address}): {e}")

# Bluetooth server for pairing and command input
def bluetooth_server():
    server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server_sock.bind(("", bluetooth.PORT_ANY))
    server_sock.listen(1)
    print("Bluetooth pairing mode enabled. Waiting for connections...")

    while True:
        client_sock, client_info = server_sock.accept()
        print(f"Connected to {client_info}")

        try:
            while True:
                data = client_sock.recv(1024).decode("utf-8").strip()
                parsed_input = parse_input(data)
                if parsed_input:
                    slave_address, speed, direction, duration = parsed_input
                    if slave_address in slaves:
                        send_motor_command(slave_address, speed, direction, duration)
                    else:
                        print(f"Unknown slave address: {slave_address}")
        except bluetooth.BluetoothError:
            print("Bluetooth connection closed.")
        finally:
            client_sock.close()

def parse_input(input_str):
    try:
        parts = input_str.replace(' ', '').split(',')
        if len(parts) != 4:
            raise ValueError("Invalid input format")
        
        slave_address = int(parts[0])
        speed = int(parts[1])
        direction = int(parts[2])
        duration = float(parts[3])
        
        return slave_address, speed, direction, duration
    except Exception as e:
        print(f"Error parsing input: {str(e)}")
        print("Please use the format: (address, Speed-Hz, 1/0, second)")
        return None

# Initial scan and configuration
found_devices = scan_i2c()
for address, motor in slaves.items():
    if address in found_devices:
        send_motor_config(address, motor)

# Start Bluetooth server for command input
bluetooth_server()
