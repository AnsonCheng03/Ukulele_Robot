import serial

# Dictionary of I2C slaves with motor configurations
slaves = {
    8: {"Name": "Right Motor 1"}, # Near Guitar
    9: {"Name": "Right Motor 2"}, 
    10: {"Name": "Left Second Motor"},
    11: {"Name": "Right Second Motor"},
    # 14: {"Name": "Control Motor"},
}

# Initialize I2C bus (1 for Raspberry Pi)
i2c_bus = smbus.SMBus(1)

note_mapping = {  # Address: Note: MoveDistance
    9: {'A': -1, '0': -1,
        'A#': 1, 'Bb': 1, "1": 1,
        'B': 5, "2": 5,
        'C': 8, "3": 8,
        'C#': 11, 'Db': 11, "4": 11,
        'D': 14, "5": 14,
        'D#': 17, 'Eb': 17, "6": 17,
        # 'E': 20, "7": 20,
        # 'F': 23, "8": 23,
        # 'F#': 26, 'Gb': 26, "9": 26,
        # 'G': 29, "10": 29,
        # 'G#': 32, 'Ab': 32, "11": 32,
        # 'A2': 36, "12": 36
    },
    8: {'E': -1, '0': -1,
        'F': 1, "1": 1,
        'F#': 5, 'Gb': 5, "2": 5,
        'G': 8, "3": 8,
        'G#': 11, 'Ab': 11, "4": 11,
        'A': 14, "5": 14,
        'A#': 17, 'Bb': 17, "6": 17
        # 'B': 81, "7": 81,
        # 'C': 93, "8": 93,
        # 'C#': 105, 'Db': 105, "9": 105,
        # 'D': 117, "10": 117,
        # 'D#': 129, 'Eb': 129, "11": 129,
        # 'E2': 141, "12": 141
    },
    11: {'C': -1, '0': -1,
            'C#': 0, 'Db': 0, "1": 0,
            'D': 9, "2": 9,
            'D#': 17, 'Eb': 17, "3": 17,
            'E': 25, "4": 25,
            'F': 32, "5": 32,
            'F#': 39, 'Gb': 39, "6": 39,
            'G': 45, "7": 45,
            'G#': 50, 'Ab': 50, "8": 50,
            'A': 55, "9": 55,
            'A#': 58, 'Bb': 58, "10": 58,
            'B': 62, "11": 62,
            'C2': 68, "12": 68
        },
    10: {'G': -1, '0': -1,
        'G#': 7, 'Ab': 7, "1": 7,
        'A': 15, "2": 15,
        'A#': 23, 'Bb': 23, "3": 23,
        'B': 29, "4": 29,
        'C': 36, "5": 36,
        'C#': 42, 'Db': 42, "6": 42,
        'D': 48, "7": 48,
        'D#': 53, 'Eb': 53, "8": 53
        # 'E': 55, "9": 55,
        # 'F': 58, "10": 58,
        # 'F#': 62, 'Gb': 62, "11": 62,
        # 'G2': 68, "12": 68
    },
}
   
chord_mapping = {  # Chord: [Note, Address]
    'Amaj': [['2', 8], ['1', 9], ['-1', 10], ['-1', 11]],
    # 'A6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'A7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'A9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Amaj7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    'Am': [['1', 8], ['-1', 9], ['-1', 10], ['-1', 11]],
    # 'Am6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Am7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Am9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Asus2': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Asus4': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Aaug': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Adim': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    
    # 'Bmaj': [['3', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'B6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'B7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'B9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bmaj7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bm': [['2', 8], ['-1', 9], ['-1', 10], ['-1', 11]],
    # 'Bm6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bm7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bm9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bsus2': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bsus4': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Baug': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Bdim': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    
    # 'Cmaj': [['3', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'C6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'C7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'C9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Cmaj7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Cm': [['2', 8], ['-1', 9], ['-1', 10], ['-1', 11]],
    # 'Cm6': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Cm7': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Cm9': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Csus2': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Csus4': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Caug': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    # 'Cdim': [['1', 8], ['2', 9], ['-1', 10], ['-1', 11]],
    'EM7': [['-1', 8], ['2', 9], ['2', 10], ['1', 11]],
    'GB9': [['-1', 8], ['1', 9], ['1', 10], ['1', 11]]
} 

def send_motor_command(slave_address, command_type, *args):
    try:
        motor_name = slaves[slave_address]['Name']
        print(f"Sending to {motor_name} via UART - Type {command_type}, Args: {args}")

        if command_type == 0:  # Control
            target = int(args[0])
            speed = int(args[1])
            duration = int(args[2])
            direction = int(args[3])
            msg = f"C,{command_type},{slave_address},{target},{speed},{duration},{direction}\n"

        elif command_type == 1:  # Calibrate
            if args:
                calib_target = int(args[0])
            else:
                calib_target = 0
            msg = f"A,{command_type},{slave_address},{calib_target}\n"

        elif command_type == 2:  # Move
            target = int(args[0]) if len(args) == 2 else 0
            distance = int(args[1]) if len(args) == 2 else int(args[0])
            msg = f"M,{command_type},{slave_address},{target},{distance}\n"

        elif command_type == 3:  # Fingering
            note = args[0].upper()
            if note not in note_mapping[slave_address]:
                print(f"Invalid note: {note}")
                return
            distance = note_mapping[slave_address][note]
            msg = f"F,{command_type},{slave_address},0,{distance}\n"

        elif command_type == 5:  # Debug
            action_type_input = args[0].lower()
            if action_type_input != "moveby":
                print(f"Invalid debug action: {action_type_input}")
                return
            target = int(args[1])
            position_mm = int(args[2])
            msg = f"D,{command_type},{slave_address},{target},{position_mm}\n"

        else:
            print("Unsupported command type")
            return

        # Send over UART
        serial_port.write(msg.encode('utf-8'))

    except Exception as e:
        print(f"Error sending command: {e}")


def handle_command_input(command):
    command_parts = command.split()
    if len(command_parts) < 2:
        print("Invalid command format")
        return

    try:
        command_mapping = {
            "0": 0, "control": 0,
            "1": 1, "calibrate": 1,
            "2": 2, "move": 2,
            "3": 3, "fingering": 3,
            "4": 4, "chord": 4,
            "debug": 5
        }

        command_type_input = command_parts[0].lower()

        if command_type_input == "chord":
            chord = command_parts[1].upper()
            if chord not in chord_mapping:
                print(f"Invalid chord: {chord}")
                return
            for note, address in chord_mapping[chord]:
                send_motor_command(address, 3, note)

        elif command_type_input in command_mapping:
            command_type = command_mapping[command_type_input]
            slave_address = int(command_parts[1])
            args = command_parts[2:]
            send_motor_command(slave_address, command_type, *args)

        elif 5 <= len(command_parts) <= 6:
            slave_address = int(command_parts[0])
            target = int(command_parts[1])
            speed = int(command_parts[2])
            duration = int(command_parts[3])
            direction = int(command_parts[4])
            send_motor_command(slave_address, 0, target, speed, duration, direction)

        else:
            print("Invalid command format: too many or too few arguments")

    except Exception as e:
        print(f"Command parse error: {e}") 