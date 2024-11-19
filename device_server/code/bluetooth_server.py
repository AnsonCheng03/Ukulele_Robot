import bluetooth
from motor_control import handle_command_input

def start_bluetooth_server():
    server_sock = bluetooth.BluetoothSocket(bluetooth.RFCOMM)
    server_sock.bind(("", bluetooth.PORT_ANY))
    server_sock.listen(1)
    bluetooth.advertise_service(
        server_sock,
        "MotorControlServer",
        service_classes=[bluetooth.SERIAL_PORT_CLASS],
        profiles=[bluetooth.SERIAL_PORT_PROFILE],
    )
    print("Bluetooth pairing mode enabled. Always discoverable. Waiting for connections...")

    while True:
        client_sock, client_info = server_sock.accept()
        print(f"Connected to {client_info}")

        try:
            while True:
                data = client_sock.recv(1024).decode("utf-8").strip()
                if data:
                    print(f"Received data: {data}")
                    handle_command_input(data)
        except bluetooth.BluetoothError as e:
            print(f"Bluetooth connection closed: {e}")
        finally:
            client_sock.close()
