// Services/BleService.ts
import {
  BleManager,
  Device,
  Characteristic,
  State,
} from "react-native-ble-plx";
import RNFS from "react-native-fs";
import { basename } from "path-browserify";
import { sha1 } from "react-native-sha256";
import { Buffer } from "buffer";
import Toast from "react-native-toast-message";

export class BleService {
  private static instance: BleService;
  private manager: BleManager;
  private device: Device | null = null;

  private constructor() {
    this.manager = new BleManager();
  }

  public static getInstance(): BleService {
    if (!BleService.instance) {
      BleService.instance = new BleService();
    }
    return BleService.instance;
  }

  // Device setter and getter
  public setDevice(device: Device) {
    this.device = device;
  }

  public getDevice(): Device | null {
    return this.device;
  }

  public async checkConnection(): Promise<boolean> {
    const device = this.device;
    if (!device) return false;
    return await device.isConnected();
  }

  public async connectToDevice(device?: Device): Promise<void> {
    const dev = device || this.device;
    if (!dev) throw new Error("No device set to connect to.");

    return dev
      .connect()
      .then((connectedDevice: Device) => {
        this.device = connectedDevice; // update to connected instance
        return connectedDevice.discoverAllServicesAndCharacteristics();
      })
      .then(() => {
        console.log("Device connected and services discovered");
      })
      .catch((error) => {
        Toast.show({
          type: "error",
          text1: "Connection Error",
          text2: error.message,
          position: "top",
          visibilityTime: 2000,
          autoHide: true,
          topOffset: 50,
        });
        throw error;
      });
  }

  public async sendCommandToDevice(command: string): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    const [serviceUUID, characteristicUUID] = [
      "0000180d-0000-1000-8000-00805f9b34fb",
      "00002a39-0000-1000-8000-00805f9b34fb",
    ];

    let attempts = 0;
    while (attempts < 3) {
      try {
        const response =
          await this.device.writeCharacteristicWithResponseForService(
            serviceUUID,
            characteristicUUID,
            btoa(command)
          );

        const value = await response.read();
        return;
      } catch (error) {
        Toast.show({
          type: "error",
          text1: "Device disconnected. Reconnecting...",
          position: "top",
          visibilityTime: 2000,
          autoHide: true,
          topOffset: 50,
        });
        console.error("Error sending command:", error);
        await this.connectToDevice();
        attempts++;
      }
    }
    throw new Error("Failed to connect and send command after 3 attempts");
  }

  public async sendFileToDevice(
    file: string,
    onProgress?: (current: number, total: number) => void
  ): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    if (
      !file.endsWith(".mid") &&
      !file.endsWith(".mxl") &&
      !file.endsWith(".musicxml")
    ) {
      throw new Error("Only .mid/.mxl/.musicxml files are supported");
    }

    const [serviceUUID, characteristicUUID] = [
      "0000180e-0000-1000-8000-00805f9b34fb",
      "00002a3b-0000-1000-8000-00805f9b34fb",
    ];

    const fileName = basename(file);
    const fileContent = await RNFS.readFile(file, "base64");
    const chunks = fileContent.match(/.{1,20}/g) || [];

    // Send file name
    const fileNameChunk = Buffer.from("FILENAME:" + fileName).toString(
      "base64"
    );
    await this.device.writeCharacteristicWithResponseForService(
      serviceUUID,
      characteristicUUID,
      fileNameChunk
    );

    for (let i = 0; i < chunks.length; i++) {
      const chunk = chunks[i];
      const chunkBuffer = Buffer.from(chunk, "base64");
      const indexBuffer = Buffer.alloc(2); // 2 bytes = up to 65535 chunks
      indexBuffer.writeUInt16BE(i, 0);
      const combinedBuffer = Buffer.concat([indexBuffer, chunkBuffer]);
      const base64Payload = combinedBuffer.toString("base64");

      const checksum = await sha1(chunk);
      let success = false;
      let attempts = 0;
      const maxRetries = 3;

      while (!success && attempts < maxRetries) {
        try {
          const response =
            await this.device.writeCharacteristicWithResponseForService(
              serviceUUID,
              characteristicUUID,
              base64Payload
            );

          const value = await response.read();
          if (!value?.value) throw new Error("No checksum response received");

          const serverChecksum = Buffer.from(value.value, "base64").toString(
            "hex"
          );

          if (serverChecksum === checksum) {
            success = true;

            if (onProgress) {
              const progress =
                Math.floor(((i + 1) / chunks.length) * 10000) / 100;
              onProgress(progress, 100);
            }
          } else {
            throw new Error(
              `Checksum mismatch: expected ${checksum}, got ${serverChecksum}`
            );
          }
        } catch (err) {
          attempts++;
          Toast.show({
            type: "error",
            text1: `Retrying chunk ${i} (attempt ${attempts})`,
            position: "top",
            visibilityTime: 2000,
            autoHide: true,
            topOffset: 50,
          });
          if (attempts >= maxRetries) throw err;
        }
      }
    }

    // Send EOF
    const eof = Buffer.from("EOF").toString("base64");
    await this.device.writeCharacteristicWithResponseForService(
      serviceUUID,
      characteristicUUID,
      eof
    );

    // Read back full file checksum
    const finalChecksumChar = await this.readFileResponse();
    if (!finalChecksumChar?.value) {
      throw new Error("No final checksum received from device");
    }

    const serverChecksum = Buffer.from(
      finalChecksumChar.value,
      "base64"
    ).toString("hex");

    // Calculate local file checksum
    const localChecksum = await sha1(fileContent); // fileContent is base64

    if (serverChecksum !== localChecksum) {
      Toast.show({
        type: "error",
        text1: "File checksum mismatch",
        text2: "Please try sending the file again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
      throw new Error("Final file checksum mismatch");
    }

    Toast.show({
      type: "success",
      text1: "File sent and verified successfully",
      position: "top",
      visibilityTime: 2000,
      autoHide: true,
      topOffset: 50,
    });
    const confirmChunk = Buffer.from("CONFIRM:" + fileName).toString("base64");
    await this.device.writeCharacteristicWithResponseForService(
      serviceUUID,
      characteristicUUID,
      confirmChunk
    );
  }

  public async getAudioFileList(): Promise<
    { name: string; modified: string }[]
  > {
    if (!this.device) throw new Error("No device connected");

    const characteristic = await this.device.readCharacteristicForService(
      "0000180f-0000-1000-8000-00805f9b34fb",
      "00002a3c-0000-1000-8000-00805f9b34fb"
    );

    if (!characteristic.value) {
      throw new Error("No value received from characteristic");
    }

    const decoded = atob(characteristic.value ?? "");
    const lines = decoded.split("\n");

    return lines
      .map((line) => {
        const [name, modified] = line.split("::");
        return { name, modified };
      })
      .filter((entry) => !!entry.name);
  }

  public async playAudioFile(
    filename: string,
    startTime: number = 0
  ): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    const command = `${filename}:${startTime}`;
    await this.device.writeCharacteristicWithResponseForService(
      "0000180f-0000-1000-8000-00805f9b34fb",
      "00002a3d-0000-1000-8000-00805f9b34fb",
      btoa(command)
    );
  }

  private readonly PAUSE_AUDIO_UUID = "00002a3f-0000-1000-8000-00805f9b34fb";

  public async pauseAudioFile(): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    await this.device.writeCharacteristicWithResponseForService(
      "0000180f-0000-1000-8000-00805f9b34fb",
      "00002a3f-0000-1000-8000-00805f9b34fb",
      btoa("PAUSE")
    );
  }

  public async deleteAudioFiles(filenames: string[]): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    for (const filename of filenames) {
      await this.device.writeCharacteristicWithResponseForService(
        "0000180f-0000-1000-8000-00805f9b34fb",
        "00002a3e-0000-1000-8000-00805f9b34fb",
        btoa(filename)
      );
    }
  }

  public async readCommandResponse(): Promise<Characteristic | null> {
    if (!this.device) throw new Error("No device connected");
    return this.device.readCharacteristicForService(
      "0000180d-0000-1000-8000-00805f9b34fb",
      "00002a37-0000-1000-8000-00805f9b34fb"
    );
  }

  public async readFileResponse(): Promise<Characteristic | null> {
    if (!this.device) throw new Error("No device connected");
    return this.device.readCharacteristicForService(
      "0000180e-0000-1000-8000-00805f9b34fb",
      "00002a3b-0000-1000-8000-00805f9b34fb"
    );
  }

  public getConnectedDevices(serviceUUIDs: string[]): Promise<Device[]> {
    return this.manager.connectedDevices(serviceUUIDs);
  }

  public onStateChange(callback: (state: State) => void) {
    const subscription = this.manager.onStateChange((state) => {
      callback(state);
      if (state === "PoweredOn" || state === "PoweredOff") {
        subscription.remove();
      }
    }, true);
  }

  public startScan(onDeviceFound: (device: Device) => void) {
    this.manager.startDeviceScan(
      null,
      { allowDuplicates: false },
      (error, device) => {
        if (error) {
          Toast.show({
            type: "error",
            text1: "Scan Error",
            text2: error.message,
            position: "top",
            visibilityTime: 2000,
            autoHide: true,
            topOffset: 50,
          });
          return;
        }
        if (device) onDeviceFound(device);
      }
    );
  }

  public stopScan() {
    this.manager.stopDeviceScan();
  }
}

export default BleService;
