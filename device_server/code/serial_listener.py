# serial_listener.py
import time

def listen_serial(serial_port):
    while True:
        try:
            if serial_port.in_waiting:
                line = serial_port.readline().decode('utf-8', errors='replace').strip()
                if line:
                    print(f"[UART RX] {line}")
                    # Optional: trigger callbacks based on line contents here
        except Exception as e:
            print(f"Serial listening error: {e}")
        time.sleep(0.1)
