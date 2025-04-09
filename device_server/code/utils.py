from motor_control import handle_command_input
from prompt_toolkit import PromptSession
from prompt_toolkit.history import InMemoryHistory
from prompt_toolkit.completion import WordCompleter
from motor_control import handle_command_input

# Define autocomplete words (can be dynamic too)
command_words = [
    'control', '
    '0', '1', '2', '3',
    'forward', 'backward',
    '100', '50', '200', '1', '0', '5', '10'
]

completer = WordCompleter(command_words, ignore_case=True)

def manual_input_handler():
    history = InMemoryHistory()
    session = PromptSession(history=history, completer=completer)

    print("Full arrow key & autocomplete support enabled (↑ ↓ ← → Tab)")

    while True:
        print("\nEnter commands followed by the parameters.")
        print("Control format: 'control [motor_ID] [target] [speed] [direction] [duration]'")
        print("Calibration format: 'C [motor_ID] [calib_target]'")
        print("Move format: 'M [motor_ID] [target] [distance]'")
        print("Fingering format: 'F [motor_ID] [note]'")
        print("Debug format: 'D [motor_ID] [action_type] [target] [position_mm]'")

        try:
            command = session.prompt("Enter command\n").strip()
            if command:
                handle_command_input(command)
        except (KeyboardInterrupt, EOFError):
            print("\nExiting input handler.")
            break

def parse_input(input_str):
    try:
        parts = input_str.replace(' ', '').split(',')
        if len(parts) != 4:
            raise ValueError("Invalid input format")
        
        motor_ID = int(parts[0])
        speed = int(parts[1])
        direction = int(parts[2])
        duration = float(parts[3])
        
        return motor_ID, speed, direction, duration
    except Exception as e:
        print(f"Error parsing input: {str(e)}")
        print("Please use the format: (address, Speed-Hz, 1/0, second)")
        return None
