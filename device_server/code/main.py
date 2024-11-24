import threading
from i2c_communication import scan_i2c, receive_motor_config
from bluetooth_server import start_bluetooth_server
from utils import manual_input_handler
from motor_control import slaves

# Initial scan and configuration
print("Starting I2C scan...", flush=True)
found_devices = scan_i2c()
print("Found devices: %s" % found_devices, flush=True)

for address, motor in slaves.items():
    if address in found_devices:
        receive_motor_config(address, motor)

# Start Bluetooth server for command input
print("Starting Bluetooth server...", flush=True)
bluetooth_thread = threading.Thread(target=start_bluetooth_server, daemon=True)
bluetooth_thread.start()

# Manual input handler started
print("Starting manual input handler...", flush=True)
manual_input_handler()