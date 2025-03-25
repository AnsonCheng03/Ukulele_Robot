import threading
import serial
import asyncio
from bluetooth_server import start_bluetooth_server
from utils import manual_input_handler

global_asyncio_loop = asyncio.new_event_loop()

def start_background_loop():
    loop = asyncio.new_event_loop()
    asyncio.set_event_loop(loop)
    print("✅ AsyncIO background loop started")
    loop.run_forever()
    

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

threading.Thread(
    target=start_background_loop,
    args=(global_asyncio_loop,),
    daemon=True
).start()

# Start Bluetooth server for command input
bluetooth_thread = threading.Thread(target=start_bluetooth_server, daemon=True)
bluetooth_thread.start()

# Handle manual input in the main thread
manual_input_handler()

