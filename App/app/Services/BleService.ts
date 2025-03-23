// Services/BleService.ts
import {
  BleManager,
  Device,
  Characteristic,
  State,
} from "react-native-ble-plx";
import RNFS from "react-native-fs";
import { sha1 } from "react-native-sha256";

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
        console.log("Discovering services and characteristics");
        this.device = connectedDevice; // update to connected instance
        return connectedDevice.discoverAllServicesAndCharacteristics();
      })
      .then(() => {
        console.log("Device connected and services discovered");
      })
      .catch((error) => {
        console.error("Error connecting to device: ", error);
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
        console.log("Received response:", value.value);
        return;
      } catch (error) {
        console.log(`Attempt ${attempts + 1}: Reconnecting...`, error);
        await this.connectToDevice();
        attempts++;
      }
    }
    throw new Error("Failed to connect and send command after 3 attempts");
  }

  public async sendFileToDevice(file: string): Promise<void> {
    if (!this.device) throw new Error("No device connected");

    const [serviceUUID, characteristicUUID] = [
      "0000180e-0000-1000-8000-00805f9b34fb",
      "00002a3b-0000-1000-8000-00805f9b34fb",
    ];

    console.log("Sending file to device", file);
    const fileContent = await RNFS.readFile(file, "base64");
    const chunks = fileContent.match(/.{1,20}/g) || [];

    for (let chunk of chunks) {
      const checksum = await sha1(chunk);

      try {
        const response =
          await this.device.writeCharacteristicWithResponseForService(
            serviceUUID,
            characteristicUUID,
            chunk
          );

        const value = await response.read();

        if (value.value) {
          const base64Value = atob(value.value);
          const byteArr = new Uint8Array(base64Value.length);
          for (let j = 0; j < base64Value.length; j++) {
            byteArr[j] = base64Value.charCodeAt(j);
          }
          console.log("Received response:", byteArr);
        } else {
          console.error("Received null value from device");
        }
      } catch (error) {
        console.error("Error sending chunk:", error);
      }
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
          console.error("Scan error:", error);
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
