import os
from bt_gatt.service import Service, Characteristic
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import bt_gatt.exceptions as exceptions
import logging
import hashlib
import base64
import binascii
import traceback

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
        self.chunk_buffers = {}  # buffer chunk_data temporarily per client
        self.last_chunk_index = {}  # track last good index
        self.storage_dir = "RobotUserFiles"
        os.makedirs(self.storage_dir, exist_ok=True)  # Ensure folder exists

    def WriteValue(self, value, options):
        try: 
            client_address = options.get('client_address', 'default')
            byte_value = bytes(value)
            
            if byte_value.startswith(b'CONFIRM:'):
                filename = byte_value[len(b'CONFIRM:'):].decode('utf-8')
                temp_filename = filename + ".tmp"
                temp_filepath = os.path.join(self.storage_dir, temp_filename)
                final_filepath = os.path.join(self.storage_dir, filename)

                if os.path.exists(temp_filepath):
                    os.rename(temp_filepath, final_filepath)
                    print(f"[INFO] Confirmed and renamed {temp_filepath} -> {final_filepath}")
                else:
                    print(f"[WARN] Tried to confirm missing file: {temp_filepath}")
                return


            if byte_value.startswith(b'FILENAME:'):
                filename_raw = byte_value[len(b'FILENAME:'):].decode('utf-8')
                filename = filename_raw.replace(" ", "_")
                temp_filename = filename + ".tmp"
                filepath = os.path.join(self.storage_dir, temp_filename)
                self.open_files[client_address] = open(filepath, 'wb')
                self.transfer_states[client_address] = {
                    "filename": filename,
                    "temp_filename": temp_filename,
                    "path": filepath
                }
                self.transfer_states[client_address] = 0  # initialize expected index
                print(f"Receiving file {filename_raw} from {client_address}, saved to {filepath}")
                return

            if byte_value == b'EOF':
                if client_address in self.open_files:
                    # ✅ Write final buffered chunk
                    if client_address in self.chunk_buffers:
                        self.open_files[client_address].write(self.chunk_buffers[client_address])
                        self.open_files[client_address].flush()
                        
                    filepath = self.open_files[client_address].name
                    
                    self.open_files[client_address].close()
                    del self.open_files[client_address]
                    self.chunk_buffers.pop(client_address, None)
                    self.last_chunk_index.pop(client_address, None)

                    # Final checksum
                    with open(filepath, 'rb') as f:
                        file_data = f.read()
                        base64_str = base64.b64encode(file_data).decode()
                        sha1sum = hashlib.sha1(base64_str.encode()).digest()
                    self.last_checksum = dbus.Array(sha1sum, signature=dbus.Signature('y'))
                    
                    print(f"[INFO] File {filepath} received and checksum computed")
                    return

            # Chunk processing with sequence checking
            try:
                expected_index = self.last_chunk_index.get(client_address, 0)

                if len(byte_value) < 2:
                    raise ValueError("Chunk too short to contain index")
                chunk_index = int.from_bytes(byte_value[:2], byteorder='big')
                chunk_data = byte_value[2:]

                # Check if this is the next chunk
                if chunk_index == expected_index + 1:
                    # ✅ Commit the last buffered chunk to file
                    if client_address in self.chunk_buffers:
                        self.open_files[client_address].write(self.chunk_buffers[client_address])
                        self.open_files[client_address].flush()

                    # ✅ Buffer this new chunk
                    self.chunk_buffers[client_address] = chunk_data
                    self.last_chunk_index[client_address] = chunk_index

                    base64_str = base64.b64encode(chunk_data).decode()
                    checksum = hashlib.sha1(base64_str.encode()).digest()
                    self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))

                elif chunk_index == expected_index:
                    # 🌀 Resend or corrupted retry — replace buffer with new data
                    self.chunk_buffers[client_address] = chunk_data

                    base64_str = base64.b64encode(chunk_data).decode()
                    checksum = hashlib.sha1(base64_str.encode()).digest()
                    self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))
                    
                elif chunk_index == 0:
                    print(f"[INFO] {client_address} is resending file from beginning")

                    if client_address in self.open_files:
                        self.open_files[client_address].seek(0)
                        self.open_files[client_address].truncate(0)

                    self.chunk_buffers[client_address] = chunk_data
                    self.last_chunk_index[client_address] = 0

                    base64_str = base64.b64encode(chunk_data).decode()
                    checksum = hashlib.sha1(base64_str.encode()).digest()
                    self.last_checksum = dbus.Array(checksum, signature=dbus.Signature('y'))
                    return

                else:
                    print(f"[ERROR] Unexpected chunk index from {client_address}: got {chunk_index}, expected {expected_index} or {expected_index + 1}")
                    # Return fake checksum to force retry
                    self.last_checksum = dbus.Array(b'\x00' * 20, signature=dbus.Signature('y'))



            except Exception as e:
                print(f"Error writing chunk from {client_address}: {e}")
                raise exceptions.InvalidValueError(f"Write error: {e}")
        except Exception as e:
            print(f"[EXCEPTION] Error writing chunk from {client_address}: {e}")
            traceback.print_exc()  # Prints full stack trace to console/log
            raise exceptions.InvalidValueError(f"Write error: {e}")

    def ReadValue(self, options):
        return self.last_checksum or dbus.Array([], signature=dbus.Signature('y'))