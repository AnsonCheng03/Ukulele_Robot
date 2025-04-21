import serial
import time

serial_port = serial.Serial(
    port='/dev/serial0',  # or '/dev/ttyAMA0'
    baudrate=115200,
    timeout=1
)

fretPosition = [0,5,40,80,120,160,190,220,250,280,310,330,350,380,400,420,450,470]

# fretPositions_scale = [
#     0.0, 19.06, 38.1, 55.57, 72.05, 87.61, 102.29, 116.14,
#     129.24, 141.59, 153.24, 164.24, 174.83, 184.43
# ]


fretScaler = 1.9

note_mapping = {
    4: {
        'G': [[4, 0], [3, 7], [2, 3]],
        'G#': [[4, 1], [3, 8], [2, 4]],
        'Ab': [[4, 1], [3, 8], [2, 4]],
        'A': [[4, 2], [3, 9], [2, 5], [1, 0]],
        'A#': [[4, 3], [3, 10], [2, 6], [1, 1]],
        'Bb': [[4, 3], [3, 10], [2, 6], [1, 1]],
        'B': [[4, 4], [3, 11], [2, 7], [1, 2]],
        'C': [[3, 0]],
        'C#': [[3, 1]],
        'Db': [[3, 1]],
        'D': [[3, 2]],
        'D#': [[3, 3]],
        'Eb': [[3, 3]],
        'E': [[3, 4], [2, 0]],
        'F': [[3, 5], [2, 1]],
        'F#': [[3, 6], [2, 2]],
        'Gb': [[3, 6], [2, 2]],
    },
    5: {
        'C': [[4, 5], [3, 12], [2, 8], [1, 3]],
        'C#': [[4, 6], [2, 9], [1, 4]],
        'Db': [[4, 6], [2, 9], [1, 4]],
        'D': [[4, 7], [2, 10], [1, 5]],
        'D#': [[4, 8], [2, 11], [1, 6]],
        'Eb': [[4, 8], [2, 11], [1, 6]],
        'E': [[4, 9], [2, 12], [1, 7]],
        'F': [[4, 10], [1, 8]],
        'F#': [[4, 11], [1, 9]],
        'Gb': [[4, 11], [1, 9]],
        'G': [[4, 12], [1, 10]],
        'G#': [[1, 11]],
        'Ab': [[1, 11]],
        'A': [[1, 12]]
    }
}

   
chord_mapping = {  # Chord: [Note, Address]
    'Amaj': [['2', 1], ['1', 2], ['-1', 3], ['-1', 4]],
    'Am': [['1', 1], ['-1', 2], ['-1', 3], ['-1', 4]],
    'EM7': [['-1', 1], ['2', 2], ['2', 3], ['1', 4]],
    'GB9': [['-1', 1], ['1', 2], ['1', 3], ['1', 4]]
} 

def calculate_distance_from_fret(fret):
    if fret == 0:
        return -1  # empty string: skip rack down
    elif fret == 1:
        return 0  # exact base for calibration
    elif fret + 1 >= len(fretPosition):
        return None
    else:
        #return ((fretPositions_scale[fret] + fretPositions_scale[fret + 1]) / 2) * fretScaler
        return fretPosition[fret]

def send_motor_command(motor_id, command_type, *args):
    try:
        # print(f"Sending to {motor_id} via UART - Type {command_type}, Args: {args}")

        if command_type == 0:  # Control
            target = int(args[0])
            speed = int(args[1])
            direction = int(args[2])
            duration = int(args[3])
            msg = f"S {motor_id} {target} {speed} {direction} {duration}\n"

        elif command_type == 1:  # Calibrate
            calib_target = int(args[0]) if args else 0
            msg = f"C {motor_id} {calib_target}\n"

        elif command_type == 2:  # Move
            target = int(args[0]) if len(args) == 2 else 0
            distance = int(args[1]) if len(args) == 2 else int(args[0])
            msg = f"M {motor_id} {target} {distance}\n"
            
        elif command_type == 3:  # Fingering by string + fret
            try:
                string = int(args[0])
                fret = int(args[1])

                distance = calculate_distance_from_fret(fret)
                if distance is None:
                    print(f"Fret {fret} out of range for distance calculation")
                    return
                distance = int(distance)

                msg = f"M {string} 0 {distance}\n"
                print(f"Sending command: {msg.strip()}")

            except (IndexError, ValueError) as e:
                print(f"Invalid string/fret arguments: {args} — {e}")
                return


        elif command_type == 4:  # Fingering
            raw_note = args[0].upper()

            if len(args) < 1:
                print("Missing note argument.")
                return

            if len(args) >= 2:
                try:
                    selected_string = int(args[1])
                except ValueError:
                    print("Invalid string number.")
                    return
            else:
                selected_string = 0  # default: auto-select

            # Extract note and optional octave
            if raw_note[-1].isdigit() and len(raw_note) > 1:
                note = raw_note[:-1]
                try:
                    octave = int(raw_note[-1])
                except ValueError:
                    print(f"Invalid octave in note: {raw_note}")
                    return
                octaves_to_check = [octave]
            else:
                note = raw_note
                octaves_to_check = note_mapping.keys()

            for octave in octaves_to_check:
                if note in note_mapping.get(octave, {}):
                    for string, fret in note_mapping[octave][note]:
                        if selected_string == 0 or string == selected_string:
                            distance = calculate_distance_from_fret(fret)
                            if distance is None:
                                print(f"Fret {fret} out of range for distance calculation")
                                return
                            distance = int(distance)

                            msg = f"M {string} 0 {distance}\n"
                            print(f"Sending command: {msg.strip()}")
                            return

            print(f"⚠️ Note {note} not playable on string {selected_string}" if selected_string else f"⚠️ Invalid note: {raw_note}")

        elif command_type == 5:  # Debug
            action_type_input = args[0].lower()
            if action_type_input != "moveby":
                print(f"Invalid debug action: {action_type_input}")
                return
            target = int(args[1])
            position_mm = int(args[2])
            msg = f"D {motor_id} {target} {position_mm}\n"
            
        elif command_type == 6:  # Multi-finger MF command
            distances = args[0]  # should be a list of 4 ints or -1
            if len(distances) != 4:
                print("MF command requires exactly 4 distances")
                return

            msg = f"MF {' '.join(str(d) for d in distances)}\n"

        else:
            print("Unsupported command type")
            return

        # Send over UART
        print(f"Sending command: {msg.strip()}")

        for attempt in range(3):
            try:
                if not serial_port.is_open:
                    serial_port.open()
                    # print(f"[Attempt {attempt+1}] Serial port opened")

                serial_port.flushInput()
                serial_port.write(msg.encode('utf-8'))
                # print(f"[Attempt {attempt+1}] Command sent successfully")
                break  # Success, exit retry loop

            except Exception as e:
                # print(f"[Attempt {attempt+1}] Serial write error: {e}")

                try:
                    serial_port.close()
                    # print(f"[Attempt {attempt+1}] Serial port closed for reset")
                except Exception:
                    pass

                time.sleep(0.1)  # brief pause before retry

                try:
                    serial_port.open()
                    # print(f"[Attempt {attempt+1}] Serial port reopened")
                except Exception as open_err:
                    print(f"[Attempt {attempt+1}] Failed to reopen serial port: {open_err}")


    except Exception as e:
        print(f"Error sending command: {e}")


def handle_command_input(command):
    command_parts = command.split()
    if len(command_parts) < 2:
        print("Invalid command format")
        return

    try:
        command_mapping = {
            "0": 0, "control": 0, "S": 0,
            "1": 1, "calibrate": 1, "C": 1,
            "2": 2, "move": 2, "M": 2,
            "3": 3, "note": 3, "N": 3,
            "4": 4, "fingering": 4, "F": 4,
            "debug": 5, "D": 5
        }

        command_type_input = command_parts[0].lower()

        if command_type_input in command_mapping:
            command_type = command_mapping[command_type_input]
            motor_id = int(command_parts[1])
            args = command_parts[2:]
            send_motor_command(motor_id, command_type, *args)

        else:
            print("Invalid command format")

    except Exception as e:
        print(f"Command parse error: {e}")
