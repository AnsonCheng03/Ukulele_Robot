import {
  BleManager,
  Device,
  Characteristic,
  State,
} from "react-native-ble-plx";

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
      .then(() => {
        console.log("Device connected and services discovered");
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
    while (attempts < 3) {
      try {
        if (!device.isConnected) {
          console.log(`Attempt ${attempts + 1}: Connecting to device...`);
          await this.connectToDevice(device);
        }

        const services = await device.services();
        for (const service of services) {
          const characteristics = await service.characteristics();
          for (const characteristic of characteristics) {
            if (characteristic.isWritableWithResponse) {
              console.log(
                `Service UUID: ${service.uuid}, Characteristic UUID: ${characteristic.uuid}`
              );
              await device.writeCharacteristicWithResponseForService(
                service.uuid,
                characteristic.uuid,
                btoa(command)
              );
              return;
            }
          }
        }
        throw new Error("No writable characteristic found");
      } catch (error) {
        console.error("Error sending command: ", error);
        attempts++;
        if (attempts >= 3) {
          throw new Error(
            "Failed to connect and send command after 3 attempts"
          );
        }
      }
    }
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
    this.manager.startDeviceScan(null, null, (error, device) => {
      if (error) {
        console.error("Scan error:", error);
        return;
      }
      if (device) {
        onDeviceFound(device);
      }
    });
  }

  public stopScan() {
    this.manager.stopDeviceScan();
  }
}
