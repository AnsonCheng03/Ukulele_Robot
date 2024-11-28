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

def send_motor_command(slave_address, command_type, *args):
    print(f"Sending command to slave {slaves[slave_address]['Name']} (address {hex(slave_address)}) with type {command_type}, target {target} and args {args}")
    try:
        control_data = []
        if command_type == 0:  # Control
            target = args[0]
            speed = args[1]
            duration = args[2]
            direction = args[3]
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
            if len(args) > 0:
                control_data.extend(args[0]) # Calibration target
            else:
                control_data.extend([0]) # Calibrate all
            pass
        elif command_type == 2:  # Move
            target = args[0]
            distance = args[1]
            control_data.extend([
                (distance >> 24) & 0xFF,
                (distance >> 16) & 0xFF,
                (distance >> 8) & 0xFF,
                distance & 0xFF
            ])
        elif command_type == 3:  # Fingering
            target = args[0]
            fingering_details = args[1]
            control_data.extend([target, fingering_details])
        elif command_type == 4:  # Chord
            chord_details = args[0]
            control_data.extend(chord_details)
        elif command_type == 5:  # Debug
            action_mapping = {"0": 0, "moveTo": 0}
            action = action_mapping[args[0].lower()]
            control_data.extend([action])
            if action == 0: # moveTo
                target = args[1]
                position_mm = args[2]
                control_data.extend([
                    target,
                    (position_mm >> 24) & 0xFF,
                    (position_mm >> 16) & 0xFF,
                    (position_mm >> 8) & 0xFF,
                    position_mm & 0xFF
                ])

        i2c_bus.write_i2c_block_data(slave_address, command_type, control_data) # 0x00 is control command
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
        command_mapping = {"0": 0, "control": 0, "1": 1, "calibrate": 1, "2": 2, "move": 2, "3": 3, "fingering": 3, "4": 4, "chord": 4, "debug": 5, "5": 5}
        command_type_input = command_parts[1].lower()

        if len(command_parts) < 5:
            if command_type_input in command_mapping:
                command_type = command_mapping[command_type_input]
                args = list(map(int, command_parts[2:]))
                send_motor_command(slave_address, command_type, *args)
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
