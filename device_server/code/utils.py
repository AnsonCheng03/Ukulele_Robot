from motor_control import handle_command_input

def manual_input_handler():
    while True:
        print("Enter 'control' or 'config' followed by the parameters.")
        print("Control format: 'control [motor_ID] [target] [speed] [direction] [duration]'")
        print("Config format: 'config [motor_ID]'")
        command = input("Enter command \n").strip()
        handle_command_input(command)

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
