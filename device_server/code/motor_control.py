import smbus

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

def send_motor_command(slave_address, target, speed, direction, duration):
    try:
        control_data = [
            target,
            (speed >> 8) & 0xFF,
            speed & 0xFF,
            (duration >> 24) & 0xFF,
            (duration >> 16) & 0xFF,
            (duration >> 8) & 0xFF,
            duration & 0xFF,
            direction
        ]
        i2c_bus.write_i2c_block_data(slave_address, 0x02, control_data)
        print(f"Command sent to slave {slaves[slave_address]['Name']} (address {hex(slave_address)})")
    except OSError as e:
        print(f"Failed to communicate with slave {slaves[slave_address]['Name']} (address {hex(slave_address)}): {e}")

def calibrate_motor(slave_address, sensor_pin):
    try:
        i2c_bus.write_i2c_block_data(slave_address, 3, [])
        print(f"Calibration command sent to slave at address {hex(slave_address)}")
    except OSError as e:
        print(f"Failed to calibrate slave at address {hex(slave_address)}: {e}")

