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
    const [serviceUUID, characteristicUUID] = [
      "0000180d-0000-1000-8000-00805f9b34fb",
      "00002a39-0000-1000-8000-00805f9b34fb",
    ];

    let attempts = 0;
    try {
      while (attempts < 3) {
        const response = await device.writeCharacteristicWithResponseForService(
          serviceUUID,
          characteristicUUID,
          btoa(command)
        );

        const value = await response.read();
        console.log("Received response:", value.value);

        return;
      }
    } catch (error) {
      console.log(`Attempt ${attempts + 1}: Connecting to device...`, error);
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
    const [serviceUUID, characteristicUUID] = [
      "0000180e-0000-1000-8000-00805f9b34fb",
      "00002a3b-0000-1000-8000-00805f9b34fb",
    ];

    console.log("Sending file to device", file);
    const fileContent = await RNFS.readFile(file, "base64");
    const chunks = fileContent.match(/.{1,20}/g) || [];

    for (let i = 0; i < chunks.length; i++) {
      const chunk = chunks[i];
      const checksum = await sha1(chunk);

      try {
        // Send the chunk and wait for the response
        const response = await device.writeCharacteristicWithResponseForService(
          serviceUUID,
          characteristicUUID,
          chunks[i]
        );

        const value = await response.read();

        if (value.value) {
          const base64Value = atob(value.value);
          const byteArr = new Uint8Array(base64Value.length);
          for (let j = 0; j < base64Value.length; j++) {
            byteArr[j] = base64Value.charCodeAt(j);
          }
          console.log("Received response:", byteArr);
          // console.log("Checksum response:", checksumResponse, checksum);
          // if (checksumResponse === checksum) {
          //   console.log("Checksums match, sending next chunk");
          // } else {
          //   console.error("Checksums do not match, resending chunk");
          //   i--;
          // }
        } else {
          console.error("Received null value from device");
        }
      } catch (error) {
        console.error("Error sending chunk:", error);
      }
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