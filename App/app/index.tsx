import React, { useEffect, useState } from "react";
import {
  StyleSheet,
  Platform,
  PermissionsAndroid,
  TouchableOpacity,
  View,
} from "react-native";
import * as SplashScreen from "expo-splash-screen";
import "react-native-reanimated";
import { useRouter } from "expo-router";

import RippleBluetooth from "./Component/bluetoothRipple";
import BottomDrawer from "./Component/bottomDrawer";

import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import { BleService } from "./Services/BleService";
import FoundDeviceModal from "./Component/FoundDeviceModal";

// Prevent splash screen from auto-hiding before setup is done
SplashScreen.preventAutoHideAsync();

export default function BluetoothSearch() {
  const [devices, setDevices] = useState<any[]>([]);
  const [scanning, setScanning] = useState(false);
  const [foundDevice, setFoundDevice] = useState<any | null>(null);
  const [showModal, setShowModal] = useState(false);
  const bleService = BleService.getInstance();
  const router = useRouter();

  useEffect(() => {
    const initBluetooth = async () => {
      if (Platform.OS === "android") {
        const granted = await requestPermissions();
        if (!granted) return;
      }

      getConnectedDevices();
      startBluetoothScan();
    };

    initBluetooth();
    return () => bleService.stopScan();
  }, []);

  const requestPermissions = async () => {
    try {
      const result = await PermissionsAndroid.requestMultiple([
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
      ]);

      const granted =
        result["android.permission.ACCESS_FINE_LOCATION"] ===
          PermissionsAndroid.RESULTS.GRANTED &&
        result["android.permission.BLUETOOTH_SCAN"] ===
          PermissionsAndroid.RESULTS.GRANTED &&
        result["android.permission.BLUETOOTH_CONNECT"] ===
          PermissionsAndroid.RESULTS.GRANTED;

      if (!granted) {
        console.warn("Location or Bluetooth permission denied");
      }

      return granted;
    } catch (error) {
      console.error("Permission request failed", error);
      return false;
    }
  };

  const getConnectedDevices = async () => {
    try {
      const serviceUUIDs = [
        "00001800-0000-1000-8000-00805f9b34fb",
        "0000180a-0000-1000-8000-00805f9b34fb",
      ];

      const connectedDevices = await bleService.getConnectedDevices(
        serviceUUIDs
      );

      setDevices((prevDevices) => {
        const newDevices = connectedDevices.filter(
          (device) => !prevDevices.some((d) => d.id === device.id)
        );
        return [...prevDevices, ...newDevices];
      });

      connectedDevices.forEach((device) => {
        console.log(`Already connected device: ${device.name}`, device);
      });
    } catch (error) {
      console.error("Failed to get connected devices:", error);
    }
  };

  const startBluetoothScan = () => {
    bleService.onStateChange((state) => {
      if (state === "PoweredOn") {
        console.log("Bluetooth is PoweredOn, starting periodic scan...");
        startPeriodicScanning();
      }
    });
  };

  const startPeriodicScanning = () => {
    const scanInterval = setInterval(() => {
      if (scanning) {
        bleService.stopScan();
        setScanning(false);
      } else {
        bleService.startScan((device) => {
          if (device && device.name) {
            const normalized = device.name.toLowerCase().replace(/\s+/g, "");
            if (
              normalized.includes("guitarrobot") ||
              normalized.includes("ukulelerobot")
            ) {
              setFoundDevice(device);
              setShowModal(true);
            }

            setDevices((prevDevices) => {
              if (!prevDevices.some((d) => d.id === device.id)) {
                return [...prevDevices, device];
              }
              return prevDevices;
            });
          }
        });
        setScanning(true);
      }
    }, 1000);

    return () => clearInterval(scanInterval);
  };

  return (
    <>
      <ThemedView style={styles.container}>
        <ThemedText style={styles.header}>Bluetooth Pairing</ThemedText>
        <ThemedView style={styles.centerContainer}>
          <RippleBluetooth />
        </ThemedView>
      </ThemedView>

      <BottomDrawer>
        {devices.map((device) => (
          <View key={device.id} style={styles.deviceContainer}>
            <View>
              <ThemedText style={styles.deviceName}>
                {device.name || "Unnamed"}
              </ThemedText>
              <ThemedText style={styles.deviceId}>{device.id}</ThemedText>
            </View>
            <TouchableOpacity
              style={styles.pairButton}
              onPress={() => {
                BleService.getInstance().setDevice(device);
                router.push("/Control");
              }}
            >
              <ThemedText style={styles.pairButtonText}>Pair</ThemedText>
            </TouchableOpacity>
          </View>
        ))}
      </BottomDrawer>

      <FoundDeviceModal
        visible={showModal}
        onClose={() => setShowModal(false)}
        onPair={() => {
          if (foundDevice) {
            BleService.getInstance().setDevice(foundDevice);
            setShowModal(false);
            router.push("/Control");
          }
        }}
        device={foundDevice}
      />
    </>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingTop: 60,
    alignItems: "center",
    justifyContent: "space-between",
  },
  header: {
    fontSize: 20,
    fontWeight: "bold",
  },
  centerContainer: {
    alignItems: "center",
    justifyContent: "center",
    flex: 1,
  },
  deviceContainer: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    paddingVertical: 10,
    paddingHorizontal: 20,
  },
  deviceName: {
    fontSize: 14,
    color: "#000",
    fontWeight: "600",
  },
  deviceId: {
    fontSize: 10,
    color: "#777",
  },
  pairButton: {
    borderWidth: 2,
    borderColor: "#007AFF",
    paddingHorizontal: 16,
    paddingVertical: 3,
    borderRadius: 50,
  },
  pairButtonText: {
    color: "#007AFF",
    fontWeight: "600",
  },
});
