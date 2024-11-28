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

note_mapping = {  # Address: Note: MoveDistance
    5: {'A': 10, 'A#': 29, 'B': 48},
    6: {'C': 10, 'C#': 29, 'D': 48},
    10: {'D#': 10, 'E': 29, 'F': 48},
    11: {'F#': 10, 'G': 29, 'G#': 48},
    14: {'A': 10, 'A#': 29, 'B': 48}
}
   
chord_mapping = {  # Chord: [Note, Address]
    'A': [['A', 5], ['C#', 6], ['E', 10]],
    'Am': [['A', 5], ['C', 6], ['E', 10]],
    'B': [['B', 5], ['D#', 6], ['F#', 10]],
} 

def send_motor_command(slave_address, command_type, *args):
    try:
        print(f"Sending command to slave {slaves[slave_address]['Name']} (address {hex(slave_address)}) with type {command_type}, and args {args}")
        control_data = []
        if command_type == 0:  # Control
            target = int(args[0])
            speed = int(args[1])
            duration = int(args[2])
            direction = int(args[3])
            control_data.extend([
                target,
                (speed >> 8) & 0xFF,
                speed & 0xFF,
                (duration >> 24) & 0xFF,
                (duration >> 16) & 0xFF,
                (duration >> 8) & 0xFF,
                duration & 0xFF,
                direction & 0xFF
            ])
            i2c_bus.write_i2c_block_data(slave_address, command_type, control_data)
        elif command_type == 1:  # Calibrate
            if len(args) > 0:
                control_data.extend(args[0]) # Calibration target
            else:
                control_data.extend([0]) # Calibrate all
            i2c_bus.write_i2c_block_data(slave_address, command_type, control_data)
            pass
        elif command_type == 2:  # Move
            target = int(args[0]) if len(args) == 2 else 0
            distance = int(args[1]) if len(args) == 2 else int(args[0])
            control_data.extend([
                target,
                (distance >> 24) & 0xFF,
                (distance >> 16) & 0xFF,
                (distance >> 8) & 0xFF,
                distance & 0xFF
            ])
            i2c_bus.write_i2c_block_data(slave_address, command_type, control_data)
        elif command_type == 3:  # Fingering
            note = args[0].upper()
            if note not in note_mapping[slave_address]:
                print(f"Invalid note: {note}")
                return
            distance = note_mapping[slave_address][note]
            control_data.extend([
                0,  # Target
                (distance >> 24) & 0xFF,
                (distance >> 16) & 0xFF,
                (distance >> 8) & 0xFF,
                distance & 0xFF
            ])
            i2c_bus.write_i2c_block_data(slave_address, 2, control_data)
        elif command_type == 5:  # Debug
            action_mapping = {"moveby": 0}
            action_type_input = args[0].lower()
            if action_type_input not in action_mapping:
                print(f"Invalid debug action: {action_type_input}")
                return
            action = action_mapping[action_type_input]
            control_data.extend([action])
            if action == 0: 
                target = int(args[1])
                position_mm = int(args[2])
                control_data.extend([
                    target,
                    (position_mm >> 24) & 0xFF,
                    (position_mm >> 16) & 0xFF,
                    (position_mm >> 8) & 0xFF,
                    position_mm & 0xFF
                ])
            i2c_bus.write_i2c_block_data(slave_address, command_type, control_data)
    except OSError as e:
        print(f"Failed to communicate with slave {slaves[slave_address]['Name']} (address {hex(slave_address)}): {e}")
    except Exception as e:
        print(f"Failed to send command: {e}")

def handle_command_input(command):
    command_parts = command.split()
    if len(command_parts) < 2:
        print("Invalid command format")
        return
    try:
        # Simplified command mapping
        command_mapping = {"0": 0, "control": 0, "1": 1, "calibrate": 1, "2": 2, "move": 2, "3": 3, "fingering": 3, "4": 4, "chord": 4, "debug": 5}
        command_type_input = command_parts[0].lower()
        
        slave_address = command_parts[1] if len(command_parts) != 5 else command_parts[0]

        if len(command_parts) < 5 or command_type_input == "debug":
            if command_type_input in command_mapping:
                command_type = command_mapping[command_type_input]
                
                if command_type == 4:
                    chord = command_parts[1].upper()
                    if chord not in chord_mapping:
                        print(f"Invalid chord: {chord}")
                        return
                    for note, address in chord_mapping[chord]:
                        send_motor_command(address, 3, note)
                else:
                    args = command_parts[2:]
                    send_motor_command(int(slave_address), command_type, *args)
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
    except Exception as e:
        print(f"Failed to handle command: {e}")
    pass
