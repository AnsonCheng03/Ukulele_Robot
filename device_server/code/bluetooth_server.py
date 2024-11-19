import subprocess
from bluedot.btcomm import BluetoothServer
from signal import pause
from motor_control import handle_command_input

# Run setup commands before starting the server and ensure they complete successfully
def run_setup_command(command):
    result = subprocess.run(command, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Error running command {' '.join(command)}: {result.stderr}")
        exit(1)
    else:
        print(f"Successfully ran command: {' '.join(command)}")

run_setup_command(["/usr/bin/btmgmt", "power", "on"])
run_setup_command(["/usr/bin/btmgmt", "connectable", "on"])
run_setup_command(["/usr/bin/btmgmt", "pairable", "on"])
run_setup_command(["/usr/bin/btmgmt", "discov", "on"])
run_setup_command(["/usr/bin/btmgmt", "io-cap", "3"])
run_setup_command(["/usr/bin/btmgmt", "le", "on"])
run_setup_command(["/usr/bin/btmgmt", "bredr", "off"])
# run_setup_command(["/usr/bin/hciconfig", "hci0", "sspmode", "disable"])

# Buffer to store incoming data for each client
data_buffers = {}

# Define command terminator
COMMAND_TERMINATOR = "###"  # Use a specific symbol as the command terminator

def data_received(data):
    s.send(data)  # Echo back the received data immediately
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

    # Handle cases where COMMAND_TERMINATOR might be incomplete
    data_buffers[client_address] = data_buffers[client_address].replace("#", "###")
    
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
                print(f"Received command from {client_address}: {command}")
                handle_command_input(command)
                s.send(command)  # Echo back the received command if needed

# Initialize the Bluetooth server
s = BluetoothServer(data_received)
print("Bluetooth pairing mode enabled. Always discoverable. Waiting for connections...")

# Pause to keep the script running and handle connections
pause()
