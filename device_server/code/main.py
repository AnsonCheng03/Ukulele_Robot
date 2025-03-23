import threading
import serial
from bluetooth_server import start_bluetooth_server
from utils import manual_input_handler
from motor_control import slaves

# Setup UART Serial
# serial_port = serial.Serial(
#     port='/dev/serial0',  # or '/dev/ttyAMA0' depending on Pi model and config
#     baudrate=115200,
#     timeout=1
# )

# print("UART Serial connected on /dev/serial0 at 115200 baud")

# def send_motor_config():
#     for address, motor in slaves.items():
#         # You can customize the config format
#         msg = f"CONFIG {motor.name},{motor.max_position},{motor.speed}\n"
#         serial_port.write(msg.encode('utf-8'))
#         print(f"Sent config to motor {motor.name}")

# Start Bluetooth server for command input
bluetooth_thread = threading.Thread(target=start_bluetooth_server, daemon=True)
bluetooth_thread.start()

# Send config (optional, or you can send later on demand)
# send_motor_config()

# Handle manual input in the main thread
manual_input_handler()
