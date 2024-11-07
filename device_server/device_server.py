import smbus
import serial
import bluetooth
import threading
import time

# Dictionary of I2C slaves with motor configurations
slaves = {
    8: {"Name": "Left First Motor"},
    9: {"Name": "Right First Motor"},
    10: {"Name": "Left Second Motor"},
    11: {"Name": "Right Second Motor"},
    14: {"Name": "Control Motor"},
}

# Initialize I2C bus (1 for Raspberry Pi)
i2c_bus = smbus.SMBus(1)

# Scan for I2C devices and configure found devices
def scan_i2c():
    print("Scanning for I2C devices...")
    detected_devices = []
    
    for address in range(3, 128):  # Typical I2C address range
        try:
            # Use a dummy read to check if a device responds
            i2c_bus.read_byte(address)
            detected_devices.append(address)
            name = slaves.get(address, {}).get("Name", "Unknown")
            print(f"Device {name} found at address: {address}")
        except OSError:
            # No device at this address
            pass
    
    if not detected_devices:
        print("No I2C devices found.")
    else:
        print(f"Found {len(detected_devices)} I2C devices.")

    return detected_devices

def receive_motor_config(slave_address, motor):
    try:
        # Send CMD_CONFIG to request configuration
        i2c_bus.write_i2c_block_data(slave_address, 1, [])  # 1 corresponds to CMD_CONFIG

        # Read the 6-byte configuration response from the slave
        response = i2c_bus.read_i2c_block_data(slave_address, 0, 6)
        
        # Assign the configuration data to the motor dictionary with descriptive keys
        motor["Config"] = {
            "slider_start_pin": response[0],
            "slider_speed_pin": response[1],
            "slider_sensor_pin": response[2],
            "motor_start_pin": response[3],
            "motor_speed_pin": response[4],
            "motor_direction_pin": response[5]
        }

        # Print received configuration for debugging
        print(f"Received configuration from slave {motor['Name']} (address {slave_address}):")
        print(f"  Slider Start Pin: {response[0]}")
        print(f"  Slider Speed Pin: {response[1]}")
        print(f"  Slider Sensor Pin: {response[2]}")
        print(f"  Motor Start Pin: {response[3]}")
        print(f"  Motor Speed Pin: {response[4]}")
        print(f"  Motor Direction Pin: {response[5]}")

    except OSError as e:
        print(f"Failed to communicate with slave {motor['Name']} (address {slave_address}): {e}")

# Send motor control commands
def send_motor_command(slave_address, speed, direction, duration):
    try:
        # Create a control string that includes speed, duration, and direction separated by a comma
        control_data = f"{speed},{duration},{direction}"
        
        # Send motor control command to the slave
        # We use write_i2c_block_data to send the control data as a byte array
        i2c_bus.write_i2c_block_data(slave_address, 0x01, [ord(c) for c in control_data])  # Using 0x01 as the register address
        print(f"Command sent to slave {slaves[slave_address]['Name']} (address {slave_address})")
    except OSError as e:
        print(f"Failed to communicate with slave {slaves[slave_address]['Name']} (address {slave_address}): {e}")


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

def manual_input_handler():
    while True:
        command = input("Enter command (e.g., 'config [slave_address]'):\n").strip()
        if command.startswith("config"):
            _, slave_address = command.split()
            slave_address = int(slave_address, 16)  # Convert to integer if needed
            if slave_address in slaves:
                receive_motor_config(slave_address, slaves[slave_address])
            else:
                print(f"Unknown slave address: {slave_address}")

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
        receive_motor_config(address, motor)

# Start Bluetooth server for command input
bluetooth_thread = threading.Thread(target=bluetooth_server, daemon=True)
bluetooth_thread.start()

# Handle manual input in the main thread
manual_input_handler()
