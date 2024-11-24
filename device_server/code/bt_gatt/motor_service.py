from service import Service, Characteristic
from ..motor_control import handle_command_input
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
        self.data_buffers = {}  # Buffer to accumulate data from different clients


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
    COMMAND_TERMINATOR = "###"  # Use a specific symbol as the command terminator

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.MOTOR_WRITE_UUID,
                ['write'],
                service)

    def WriteValue(self, value, options):
        print('WriteValue: ' + repr(value))

        # Convert value to string for easier command processing
        data = ''.join(chr(b) for b in value)
        client_address = options.get('client_address', 'default')

        # Initialize buffer for the client if not already present
        if client_address not in self.service.data_buffers:
            self.service.data_buffers[client_address] = ""

        # Append received data to the client's buffer
        self.service.data_buffers[client_address] += data

        # Handle cases where COMMAND_TERMINATOR might be incomplete
        self.service.data_buffers[client_address] = self.service.data_buffers[client_address].replace("#", "###")

        # Check if a full command (ending with the terminator) is received
        if self.COMMAND_TERMINATOR in self.service.data_buffers[client_address]:
            # Split the buffer by the command terminator to get complete commands
            commands = self.service.data_buffers[client_address].split(self.COMMAND_TERMINATOR)
            # Keep any incomplete command in the buffer
            self.service.data_buffers[client_address] = commands.pop()

            # Process each complete command
            for command in commands:
                command = command.strip()
                if command:
                    print(f"Received command from {client_address}: {command}")
                    handle_command_input(command)
                    print(f"Processed command: {command}")

        # Update motor status with the last received byte if it is a single-byte command
        if len(value) == 1:
            byte = value[0]
            print('Motor control value: ' + repr(byte))

            # Update motor status
            self.service.motor_status = byte
            print('Motor status updated to: ' + str(self.service.motor_status))
        else:
            raise exceptions.InvalidValueLengthException()
