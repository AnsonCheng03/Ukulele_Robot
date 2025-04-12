import threading
import serial
import asyncio
import loop_manager 
from bluetooth_server import start_bluetooth_server
from utils import manual_input_handler
from serial_listener import listen_serial  # ← Import your listener

# Setup UART Serial
serial_port = serial.Serial(
    port='/dev/serial0',
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

# Start Bluetooth server in background
bluetooth_thread = threading.Thread(target=start_bluetooth_server, daemon=True)
bluetooth_thread.start()

# Start serial listener in background
serial_listener_thread = threading.Thread(target=listen_serial, args=(serial_port,), daemon=True)
serial_listener_thread.start()

# Handle manual input in the main thread
manual_input_handler()
