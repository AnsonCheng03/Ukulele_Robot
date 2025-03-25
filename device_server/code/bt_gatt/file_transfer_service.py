import os
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
        print("FileTransferService initialized")


class FileReadChrc(Characteristic):
    FILE_READ_UUID = '00002a3a-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        Characteristic.__init__(
                self, bus, index,
                self.FILE_READ_UUID,
                ['read'],
                service)
        print("FileReadChrc initialized")

    def ReadValue(self, options):
        client_address = options.get('client_address', 'default')
        file_data = self.service.file_data.get(client_address, b'')
        print(f"Read request from {client_address}, data: {file_data}")
        return list(file_data)


class FileWriteChrc(Characteristic):
    FILE_WRITE_UUID = '00002a3b-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        Characteristic.__init__(
            self, bus, index,
            self.FILE_WRITE_UUID,
            ['read', 'write'],
            service)
        self.last_checksum = None
        self.open_files = {}
        self.storage_dir = "RobotUserFiles"
        os.makedirs(self.storage_dir, exist_ok=True)  # Ensure folder exists

    def WriteValue(self, value, options):
        client_address = options.get('client_address', 'default')
        byte_value = bytes(value)

        if byte_value.startswith(b'FILENAME:'):
            filename_raw = byte_value[len(b'FILENAME:'):].decode('utf-8')
            filename = filename_raw.replace(" ", "_")
            filepath = os.path.join(self.storage_dir, filename)
            self.open_files[client_address] = open(filepath, 'wb')
            logging.info(f"Receiving file {filename_raw} from {client_address}, saved to {filepath}")
            return

        if byte_value == b'EOF':
            if client_address in self.open_files:
                self.open_files[client_address].close()
                del self.open_files[client_address]
            logging.info(f"Completed file transfer from {client_address}")
            self.last_checksum = dbus.Array([], signature=dbus.Signature('y'))
            return

        try:
            self.open_files[client_address].write(byte_value)
            self.open_files[client_address].flush()
            logging.debug(f"Wrote {len(byte_value)} bytes for {client_address}")

            checksum = hashlib.sha1(byte_value).digest()
            self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))

        except Exception as e:
            logging.error(f"Error writing chunk from {client_address}: {e}")
            raise exceptions.InvalidValueError(f"Write error: {e}")

    def ReadValue(self, options):
        return self.last_checksum or dbus.Array([], signature=dbus.Signature('y'))