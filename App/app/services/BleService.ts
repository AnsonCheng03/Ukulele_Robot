import {
  BleManager,
  Device,
  Characteristic,
  State,
} from "react-native-ble-plx";
import RNFS from "react-native-fs";

export class BleService {
  private static instance: BleService;
  private manager: BleManager;

  private constructor() {
    this.manager = new BleManager();
  }

  public static getInstance(): BleService {
    if (!BleService.instance) {
      BleService.instance = new BleService();
    }
    return BleService.instance;
  }

  public connectToDevice(device: Device): Promise<void> {
    return device
      .connect()
      .then((connectedDevice: Device) => {
        console.log("Discovering services and characteristics");
        return connectedDevice.discoverAllServicesAndCharacteristics();
      })
      .then((results) => {
        console.log("Device connected and services discovered", results);
      })
      .catch((error) => {
        console.error("Error connecting to device: ", error);
        throw error;
      });
  }

  public async sendCommandToDevice(
    device: Device,
    command: string
  ): Promise<void> {
    let attempts = 0;
    try {
      while (attempts < 3) {
        console.log(
          "sent",
          await device.writeCharacteristicWithResponseForService(
            "0000180d-0000-1000-8000-00805f9b34fb",
            "00002a39-0000-1000-8000-00805f9b34fb",
            btoa(command)
          )
        );
        return;
      }
    } catch (error) {
      console.log(`Attempt ${attempts + 1}: Connecting to device...`);
      await this.connectToDevice(device);
      attempts++;
      console.error("Error sending command: ", error);
      if (attempts >= 3) {
        throw new Error("Failed to connect and send command after 3 attempts");
      }
    }
  }

  public async readCommandResponseFromDevice(
    device: Device
  ): Promise<Characteristic | null> {
    return device.readCharacteristicForService(
      "0000180d-0000-1000-8000-00805f9b34fb",
      "00002a37-0000-1000-8000-00805f9b34fb"
    );
  }

  public async sendFileToDevice(device: Device, file: string): Promise<void> {
    console.log("Sending file to device", file);
    const fileContent = await RNFS.readFile(file, "base64");
    const chunks = fileContent.match(/.{1,20}/g) || [];

    for (let i = 0; i < chunks.length; i++) {
      console.log("Sending chunk", i);
      await device.writeCharacteristicWithResponseForService(
        "0000180e-0000-1000-8000-00805f9b34fb",
        "00002a3b-0000-1000-8000-00805f9b34fb",
        chunks[i]
      );
    }
  }

  public async readFileResponseFromDevice(
    device: Device
  ): Promise<Characteristic | null> {
    return device.readCharacteristicForService(
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
      {
        allowDuplicates: false,
      },
      (error, device) => {
        if (error) {
          console.error("Scan error:", error);
          return;
        }
        if (device) {
          onDeviceFound(device);
        }
      }
    );
  }

  public stopScan() {
    this.manager.stopDeviceScan();
  }
}

export default BleService;
