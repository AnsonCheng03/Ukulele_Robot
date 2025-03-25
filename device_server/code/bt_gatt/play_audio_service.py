import os
import time
from bt_gatt.service import Service, Characteristic
import dbus
from bt_gatt.constants import GATT_CHRC_IFACE
import logging

logging.basicConfig(filename='audio_service.log', level=logging.DEBUG, format='%(asctime)s - %(levelname)s - %(message)s')

class PlayAudioService(Service):
    AUDIO_SERVICE_UUID = '0000180f-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, storage_dir="RobotUserFiles"):
        super().__init__(bus, index, self.AUDIO_SERVICE_UUID, True)
        self.storage_dir = storage_dir
        self.add_characteristic(ListFilesChrc(bus, 0, self))
        self.add_characteristic(PlayAudioChrc(bus, 1, self))
        self.add_characteristic(DeleteFileChrc(bus, 2, self))

        if not os.path.exists(self.storage_dir):
            os.makedirs(self.storage_dir)

        print("PlayAudioService initialized")


class ListFilesChrc(Characteristic):
    LIST_FILES_UUID = '00002a3c-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.LIST_FILES_UUID, ['read'], service)

    def ReadValue(self, options):
        files_info = []
        try:
            for f in os.listdir(self.service.storage_dir):
                full_path = os.path.join(self.service.storage_dir, f)
                if os.path.isfile(full_path):
                    mod_time = os.path.getmtime(full_path)
                    mod_time_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(mod_time))
                    files_info.append(f"{f}::{mod_time_str}")
            joined = "\n".join(files_info)
            logging.info(f"Listing files: {joined}")
            return list(joined.encode('utf-8'))
        except Exception as e:
            logging.error(f"Error listing files: {e}")
            return list(f"Error: {e}".encode('utf-8'))


class PlayAudioChrc(Characteristic):
    PLAY_AUDIO_UUID = '00002a3d-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.PLAY_AUDIO_UUID, ['write'], service)

    def WriteValue(self, value, options):
        try:
            cmd = bytes(value).decode('utf-8')  # Expected format: "filename.mp3:30" (play from 30s)
            if ":" in cmd:
                filename, start_time = cmd.split(":")
                start_time = int(start_time)
            else:
                filename = cmd
                start_time = 0

            filepath = os.path.join(self.service.storage_dir, filename)
            if not os.path.exists(filepath):
                raise FileNotFoundError(f"File {filename} not found")

            logging.info(f"Playing '{filename}' from {start_time}s")
            print(f"DEBUG: Playing '{filename}' from {start_time}s... (not implemented)")

        except Exception as e:
            logging.error(f"Error in play request: {e}")
            raise


class DeleteFileChrc(Characteristic):
    DELETE_FILE_UUID = '00002a3e-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.DELETE_FILE_UUID, ['write'], service)

    def WriteValue(self, value, options):
        try:
            filename = bytes(value).decode('utf-8').strip()
            filepath = os.path.join(self.service.storage_dir, filename)
            if os.path.exists(filepath):
                os.remove(filepath)
                logging.info(f"Deleted file: {filename}")
                print(f"File '{filename}' deleted.")
            else:
                logging.warning(f"File not found for deletion: {filename}")
        except Exception as e:
            logging.error(f"Error deleting file: {e}")
            raise
