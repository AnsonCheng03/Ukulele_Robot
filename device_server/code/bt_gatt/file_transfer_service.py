from bt_gatt.service import Service, Characteristic
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import bt_gatt.exceptions as exceptions
import logging

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
        logging.info("FileWriteChrc initialized")

    def WriteValue(self, value, options):
        client_address = options.get('client_address', 'default')
        file_content = bytes(value)
        
        if client_address not in self.service.file_data:
            self.service.file_data[client_address] = b''
        
        self.service.file_data[client_address] += file_content
        logging.info(f"Received file data from {client_address}: {file_content}")
        print(f"Received file data from {client_address}: {file_content}")
        
        with open(f"received_{client_address}.bin", "ab") as f:
            f.write(file_content)
        logging.info(f"File data saved for {client_address}")
