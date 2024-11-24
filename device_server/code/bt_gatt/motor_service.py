from service import Service, Characteristic
import dbus
from constants import GATT_CHRC_IFACE
from random import randint
import exceptions

class MotorService(Service):
    """
    Motor Service with read and write characteristics only.
    """
    MOTOR_UUID = '0000180d-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index):
        Service.__init__(self, bus, index, self.MOTOR_UUID, True)
        self.add_characteristic(MotorReadChrc(bus, 0, self))
        self.add_characteristic(MotorWriteChrc(bus, 1, self))
        self.motor_status = 0


class MotorReadChrc(Characteristic):
    MOTOR_READ_UUID = '00002a37-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.MOTOR_READ_UUID,
                ['read'],
                service)

    def ReadValue(self, options):
        # Return current motor status.
        return [self.service.motor_status]


class MotorWriteChrc(Characteristic):
    MOTOR_WRITE_UUID = '00002a39-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.MOTOR_WRITE_UUID,
                ['write'],
                service)

    def WriteValue(self, value, options):
        print('WriteValue: ' + repr(value))

        if len(value) != 1:
            raise exceptions.InvalidValueLengthException()

        byte = value[0]
        print('Motor control value: ' + repr(byte))

        # Update motor status
        self.service.motor_status = byte
        print('Motor status updated to: ' + str(self.service.motor_status))

# data_buffers = {}

# Define command terminator
# COMMAND_TERMINATOR = "###"  # Use a specific symbol as the command terminator

# def data_received(data):
#     s.send(data)  # Echo back the received data immediately
#     # Get the client's MAC address
#     client_address = s.client_address
#     if client_address is None:
#         print("No client connected.")
#         return

#     # Initialize buffer for the client if not already present
#     if client_address not in data_buffers:
#         data_buffers[client_address] = ""

#     # Append received data to the client's buffer
#     data_buffers[client_address] += data

#     # Handle cases where COMMAND_TERMINATOR might be incomplete
#     data_buffers[client_address] = data_buffers[client_address].replace("#", "###")
    
#     # Check if a full command (ending with the terminator) is received
#     if COMMAND_TERMINATOR in data_buffers[client_address]:
#         # Split the buffer by the command terminator to get complete commands
#         commands = data_buffers[client_address].split(COMMAND_TERMINATOR)
#         # Keep any incomplete command in the buffer
#         data_buffers[client_address] = commands.pop()
        
#         # Process each complete command
#         for command in commands:
#             command = command.strip()
#             if command:
#                 print(f"Received command from {client_address}: {command}")
#                 handle_command_input(command)
#                 s.send(command)  # Echo back the received command if needed

# # Initialize the Bluetooth server
# s = BluetoothServer(data_received)
# print("Bluetooth pairing mode enabled. Always discoverable. Waiting for connections...")

# # Pause to keep the script running and handle connections
# pause()
