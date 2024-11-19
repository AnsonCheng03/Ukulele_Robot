from bluedot.btcomm import BluetoothServer
from signal import pause
from motor_control import handle_command_input

# Buffer to store incoming data for each client
data_buffers = {}

# Define command terminator
COMMAND_TERMINATOR = "###"  # Use a specific symbol as the command terminator

def data_received(data):
    # Get the client's MAC address
    client_address = s.client_address
    if client_address is None:
        print("No client connected.")
        return

    # Initialize buffer for the client if not already present
    if client_address not in data_buffers:
        data_buffers[client_address] = ""

    # Append received data to the client's buffer
    data_buffers[client_address] += data
    
    # Check if a full command (ending with the terminator) is received
    if COMMAND_TERMINATOR in data_buffers[client_address]:
        # Split the buffer by the command terminator to get complete commands
        commands = data_buffers[client_address].split(COMMAND_TERMINATOR)
        # Keep any incomplete command in the buffer
        data_buffers[client_address] = commands.pop()
        
        # Process each complete command
        for command in commands:
            command = command.strip()
            if command:
                # Handle cases where command is "a" or "aa" and treat as "aaa"
                if command == "a" or command == "aa":
                    command = "aaa"
                print(f"Received command from {client_address}: {command}")
                handle_command_input(command)
                s.send(command)  # Echo back the received command if needed

# Initialize the Bluetooth server
s = BluetoothServer(data_received)
print("Bluetooth pairing mode enabled. Always discoverable. Waiting for connections...")

# Pause to keep the script running and handle connections
pause()
