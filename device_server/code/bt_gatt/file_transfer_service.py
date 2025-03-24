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
        self.open_files = {}  # Open file handles per client

    def WriteValue(self, value, options):
        client_address = options.get('client_address', 'default')
        byte_value = bytes(value)

        if client_address not in self.open_files:
            filename = f"received_file_from_{client_address.replace(':', '_')}.bin"
            self.open_files[client_address] = open(filename, 'wb')
            print(f"Started receiving file from {client_address}, saving to {filename}")

        # Check for EOF signal
        if byte_value == b'EOF':
            self.open_files[client_address].close()
            del self.open_files[client_address]
            print(f"Completed file transfer from {client_address}")
            self.last_checksum = dbus.Array([], signature=dbus.Signature('y'))
            return

        # Write chunk to file
        try:
            self.open_files[client_address].write(byte_value)
            print(f"Wrote {len(byte_value)} bytes to file for {client_address}")
            self.open_files[client_address].flush()

            # Calculate checksum of the chunk
            checksum = hashlib.sha1(byte_value).digest()
            self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))
        except Exception as e:
            logging.error(f"Error writing data from {client_address}: {e}")
            raise exceptions.InvalidValueError(f"Write error: {e}")

    def ReadValue(self, options):
        return self.last_checksum or dbus.Array([], signature=dbus.Signature('y'))