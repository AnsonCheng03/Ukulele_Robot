import os
import time
from bt_gatt.service import Service, Characteristic
import logging

logging.basicConfig(level=logging.DEBUG)

class PlayAudioService(Service):
    PLAY_AUDIO_UUID = '0000181a-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index):
        super().__init__(bus, index, self.PLAY_AUDIO_UUID, True)
        self.storage_dir = "RobotUserFiles"
        os.makedirs(self.storage_dir, exist_ok=True)

        self.add_characteristic(ListFilesChrc(bus, 0, self))
        self.add_characteristic(PlayFileChrc(bus, 1, self))
        self.add_characteristic(DeleteFileChrc(bus, 2, self))

        logging.info("PlayAudioService initialized")

class ListFilesChrc(Characteristic):
    LIST_FILES_UUID = '00002a50-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.LIST_FILES_UUID, ['read'], service)

    def ReadValue(self, options):
        logging.debug("Listing all files...")
        entries = []
        for filename in os.listdir(self.service.storage_dir):
            filepath = os.path.join(self.service.storage_dir, filename)
            if os.path.isfile(filepath):
                mtime = os.path.getmtime(filepath)
                entries.append(f"{filename}|{time.ctime(mtime)}")

        output = "\n".join(entries).encode('utf-8')
        return list(output)

class PlayFileChrc(Characteristic):
    PLAY_FILE_UUID = '00002a51-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.PLAY_FILE_UUID, ['write'], service)

    def WriteValue(self, value, options):
        filename = bytes(value).decode('utf-8').strip()
        filepath = os.path.join(self.service.storage_dir, filename)

        if not os.path.isfile(filepath):
            logging.warning(f"File {filename} not found.")
            return

        # This is the placeholder for actual playback logic
        logging.info(f"Playing audio file: {filename}")
        print(f"DEBUG: [Play] {filename}")  # Debug message

class DeleteFileChrc(Characteristic):
    DELETE_FILE_UUID = '00002a52-0000-1000-8000-00805f9b34fb'

    def __init__(self, bus, index, service):
        super().__init__(bus, index, self.DELETE_FILE_UUID, ['write'], service)

    def WriteValue(self, value, options):
        filename = bytes(value).decode('utf-8').strip()
        filepath = os.path.join(self.service.storage_dir, filename)

        if os.path.isfile(filepath):
            os.remove(filepath)
            logging.info(f"Deleted file: {filename}")
        else:
            logging.warning(f"Tried to delete non-existent file: {filename}")
