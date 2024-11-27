import smbus

# Dictionary of I2C slaves with motor configurations
slaves = {
    5: {"Name": "Right Motor 1"}, # Near Guitar
    6: {"Name": "Right Motor 2"}, 
    # 10: {"Name": "Left Second Motor"},
    # 11: {"Name": "Right Second Motor"},
    # 14: {"Name": "Control Motor"},
}

# Initialize I2C bus (1 for Raspberry Pi)
i2c_bus = smbus.SMBus(1)

def send_motor_command(slave_address, command_type, target, *args):
    try:
        control_data = [target]
        if command_type == 0:  # Control
            speed = args[0]
            duration = args[1]
            direction = args[2]
            control_data.extend([
                (speed >> 8) & 0xFF,
                speed & 0xFF,
                (duration >> 24) & 0xFF,
                (duration >> 16) & 0xFF,
                (duration >> 8) & 0xFF,
                duration & 0xFF,
                direction & 0xFF
            ])
        elif command_type == 1:  # Calibrate
            pass
        elif command_type == 2:  # Move
            distance = args[0]
            control_data.extend([
                (distance >> 24) & 0xFF,
                (distance >> 16) & 0xFF,
                (distance >> 8) & 0xFF,
                distance & 0xFF
            ])
        elif command_type == 3:  # Fingering
            fingering_details = args[0]
            control_data.extend(fingering_details)
        elif command_type == 4:  # Chord
            chord_details = args[0]
            control_data.extend(chord_details)

        i2c_bus.write_i2c_block_data(slave_address, command_type, control_data) # 0x00 is control command
        print(f"Sending command to slave {slaves[slave_address]['Name']} (address {hex(slave_address)}) with type {command_type}, target {target} and args {args}")
    except OSError as e:
        print(f"Failed to communicate with slave {slaves[slave_address]['Name']} (address {hex(slave_address)}): {e}")

def handle_command_input(command):
    command_parts = command.split()
    if len(command_parts) < 2:
        print("Invalid command format")
        return
    try:
        slave_address = int(command_parts[0])

        # Simplified command mapping
        command_mapping = {"0": 0, "control": 0, "1": 1, "calibrate": 1, "2": 2, "move": 2, "3": 3, "fingering": 3, "4": 4, "chord": 4}
        command_type_input = command_parts[1].lower()

        if len(command_parts) < 5:
            if command_type_input in command_mapping:
                command_type = command_mapping[command_type_input]
                target = int(command_parts[1])
                args = list(map(int, command_parts[2:]))
                send_motor_command(slave_address, command_type, target, *args)
            else:
                print("Invalid command type")
                return
        elif 5 <= len(command_parts) <= 6:
            # If 4 or 5 parts, it must be a control command
            command_type = 0  # Force command to be Control
            command_sub = len(command_parts) - 5
            target = int(command_parts[command_sub + 1])
            speed = int(command_parts[command_sub + 2])
            duration = int(command_parts[command_sub + 3]) 
            direction = int(command_parts[command_sub + 4]) 
            send_motor_command(slave_address, command_type, target, speed, duration, direction)
        else:
            print("Invalid command format: too many arguments")
            return
    except ValueError as e:
        print(f"Invalid command format: {e}")
    except IndexError as e:
        print(f"Invalid command format: {e}")
    except KeyError as e:
        print(f"Invalid slave address: {e}")
    except OSError as e:
        print(f"Failed to send command: {e}")
    pass
