import os
from bt_gatt.service import Service, Characteristic
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import bt_gatt.exceptions as exceptions
import logging
import hashlib
import base64
import binascii

logging.basicConfig(filename='file_transfer.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

class FileTransferService(Service):
    """
    File Transfer Service with read and write characteristics.
    """
    FILE_TRANSFER_UUID = '0000180e-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index):
        Service.__init__(self, bus, index, self.FILE_TRANSFER_UUID, True)
        self.add_characteristic(FileWriteChrc(bus, 0, self))
        self.file_data = {}  # Dictionary to store file data from different clients
        print("FileTransferService initialized")


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
        self.transfer_states = {}
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
            self.transfer_states[client_address] = 0  # initialize expected index
            print(f"Receiving file {filename_raw} from {client_address}, saved to {filepath}")
            return

        if byte_value == b'EOF':
            if client_address in self.open_files:
                self.open_files[client_address].close()
                filepath = self.open_files[client_address].name
                del self.open_files[client_address]
                self.transfer_states.pop(client_address, None)

                # Calculate file checksum (SHA-1)
                with open(filepath, 'rb') as f:
                    file_data = f.read()
                sha1sum = hashlib.sha1(file_data).digest()
                self.last_checksum = dbus.Array(sha1sum, signature=dbus.Signature('y'))

                print(f"[INFO] File {filepath} received and checksum computed")
                return


        # Chunk processing with sequence checking
        try:
            if len(byte_value) < 2:
                raise ValueError("Chunk too short to contain index")
            chunk_index = int.from_bytes(byte_value[:2], byteorder='big')
            chunk_data = byte_value[2:]

            expected_index = self.transfer_states.get(client_address, 0)
            print(f"Expected index: {expected_index}")
            if chunk_index != expected_index:
                print(f"[ERROR] Out-of-order chunk from {client_address}: expected {expected_index}, got {chunk_index}")
                raise exceptions.InvalidValueError("Chunk sequence mismatch")

            self.open_files[client_address].write(chunk_data)
            self.open_files[client_address].flush()
            self.transfer_states[client_address] += 1

            base64_str = base64.b64encode(chunk_data).decode()
            checksum = hashlib.sha1(base64_str.encode()).digest()
            self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))

        except Exception as e:
            print(f"Error writing chunk from {client_address}: {e}")
            raise exceptions.InvalidValueError(f"Write error: {e}")

    def ReadValue(self, options):
        return self.last_checksum or dbus.Array([], signature=dbus.Signature('y'))