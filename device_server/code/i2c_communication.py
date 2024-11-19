import smbus
import time
from motor_control import slaves

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

# Receive motor configuration
def receive_motor_config(slave_address, motor):
    try:
        response = i2c_bus.read_i2c_block_data(slave_address, 1, 6)
        print(response)

        print(f"Received configuration from slave {motor['Name']} (address {slave_address}):")
        print(f"  Slider Start Pin: {response[0]}")
        print(f"  Slider Speed Pin: {response[1]}")
        print(f"  Slider Sensor Pin: {response[2]}")
        print(f"  Motor Start Pin: {response[3]}")
        print(f"  Motor Speed Pin: {response[4]}")
        print(f"  Motor Direction Pin: {response[5]}")
    except OSError as e:
        print(f"Failed to communicate with slave {motor['Name']} (address {slave_address}): {e}")
