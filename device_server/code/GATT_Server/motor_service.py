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
