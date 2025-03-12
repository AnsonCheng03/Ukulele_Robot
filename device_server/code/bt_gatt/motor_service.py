from bt_gatt.service import Service, Characteristic
from motor_control import handle_command_input
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import bt_gatt.exceptions as exceptions

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

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.MOTOR_WRITE_UUID,
                ['write'],
                service)

    def WriteValue(self, value, options):
        command = ''.join(chr(b) for b in value).strip()
        client_address = options.get('client_address', 'default')
        handle_command_input(command)
        print(f"Received command from {client_address}: {command}")

        if len(value) == 1:
            byte = value[0]
            print('Motor control value: ' + repr(byte))
 
            self.service.motor_status = byte
            print('Motor status updated to: ' + str(self.service.motor_status))
        else:
            raise exceptions.InvalidValueLengthException()
        return dbus.Array([1], signature=dbus.Signature('y'))