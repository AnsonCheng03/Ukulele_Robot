import threading
import serial
from loop_manager import start_background_loop
from bluetooth_server import start_bluetooth_server
from utils import manual_input_handler

import logging
logging.basicConfig(filename='ukulele_robot.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

logging.info("Starting Ukulele Robot Main Program")


# Setup UART Serial
serial_port = serial.Serial(
    port='/dev/serial0',  # or '/dev/ttyAMA0' depending on Pi model and config
    baudrate=115200,
    timeout=1
)

if serial_port.is_open:
    print("UART Serial connected on /dev/serial0 at 115200 baud")
else:
    try:
        serial_port.open()
    except Exception as e:
        print(f"Failed to open serial port: {e}")


# Start Bluetooth server for command input
bluetooth_thread = threading.Thread(target=start_bluetooth_server, daemon=True)
bluetooth_thread.start()

# Start it in background thread
threading.Thread(target=start_background_loop, daemon=True).start()

# Handle manual input in the main thread
manual_input_handler()

