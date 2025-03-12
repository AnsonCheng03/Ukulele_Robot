from bt_gatt.service import Service, Characteristic
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import bt_gatt.exceptions as exceptions
import logging
import hashlib

logging.basicConfig(filename='file_transfer.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

class FileTransferService(Service):
    """
    File Transfer Service with read and write characteristics.
    """
    FILE_TRANSFER_UUID = '0000180e-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index):
        Service.__init__(self, bus, index, self.FILE_TRANSFER_UUID, True)
        self.add_characteristic(FileReadChrc(bus, 0, self))
        self.add_characteristic(FileWriteChrc(bus, 1, self))
        self.file_data = {}  # Dictionary to store file data from different clients
        logging.info("FileTransferService initialized")


class FileReadChrc(Characteristic):
    FILE_READ_UUID = '00002a3a-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.FILE_READ_UUID,
                ['read'],
                service)
        logging.info("FileReadChrc initialized")

    def ReadValue(self, options):
        client_address = options.get('client_address', 'default')
        file_data = self.service.file_data.get(client_address, b'')
        logging.info(f"Read request from {client_address}, data: {file_data}")
        return list(file_data)


class FileWriteChrc(Characteristic):
    FILE_WRITE_UUID = '00002a3b-0000-1000-8000-00805f9b34fb'
    
    def __init__(self, bus, index, service):
        Characteristic.__init__(
            self, bus, index,
            self.FILE_WRITE_UUID,
            ['write'],
            service)
        self.last_checksum = None
        
    def WriteValue(self, value, options):
        client_address = options.get('client_address', 'default')
        print(f"Write request from {client_address}, data: {value}")
        
        # Store the written data
        self.service.file_data[client_address] = value
        
        # Convert dbus.Array to bytes
        byte_value = bytes(value)  # Assuming value is a dbus.Array of dbus.Byte

        try:
            # Calculate the checksum
            checksum = hashlib.sha256(byte_value).digest()  
            checksum_response = dbus.Array(checksum, signature=dbus.Signature('y'))
            print(f"Checksum calculated: {checksum}")
        except Exception as e:
            print(f"Error calculating checksum: {e}")
            raise exceptions.InvalidValueError(f"Error calculating checksum: {e}")
        
        # Return the checksum in the response
        # return checksum_response
        self.last_checksum = checksum_response
        
    def ReadValue(self, options):
        return self.last_checksum