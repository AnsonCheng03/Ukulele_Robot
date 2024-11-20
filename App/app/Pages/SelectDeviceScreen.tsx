import React, { useEffect, useState } from "react";
import {
  StyleSheet,
  ScrollView,
  Platform,
  PermissionsAndroid,
} from "react-native";
import { Text, View } from "@/components/Themed";
import { BleManager } from "react-native-ble-plx";
import { useNavigation } from "@react-navigation/native";
import { StackNavigationProp } from "@react-navigation/stack";
import { RootStackParamList } from "../(tabs)/index";

export default function TabOneScreen() {
  const [devices, setDevices] = useState<any[]>([]);
  const [scanning, setScanning] = useState(false);
  const manager = new BleManager();
  const navigation = useNavigation<StackNavigationProp<RootStackParamList>>();

  useEffect(() => {
    if (Platform.OS === "android") {
      PermissionsAndroid.requestMultiple([
        PermissionsAndroid.PERMISSIONS.ACCESS_FINE_LOCATION,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_SCAN,
        PermissionsAndroid.PERMISSIONS.BLUETOOTH_CONNECT,
      ]).then((result) => {
        if (
          result["android.permission.ACCESS_FINE_LOCATION"] !==
            PermissionsAndroid.RESULTS.GRANTED ||
          result["android.permission.BLUETOOTH_SCAN"] !==
            PermissionsAndroid.RESULTS.GRANTED ||
          result["android.permission.BLUETOOTH_CONNECT"] !==
            PermissionsAndroid.RESULTS.GRANTED
        ) {
          console.warn("Location or Bluetooth permission denied");
        } else {
          startBluetoothScan();
        }
      });
    } else {
      startBluetoothScan();
    }

    return () => {
      manager.stopDeviceScan();
      manager.destroy();
    };
  }, []);

  const startBluetoothScan = () => {
    const subscription = manager.onStateChange((state) => {
      if (state === "PoweredOn") {
        console.log("Bluetooth is PoweredOn, starting periodic scan...");
        startPeriodicScanning();
        subscription.remove();
      }
    }, true);
  };

  const startPeriodicScanning = () => {
    const scanInterval = setInterval(() => {
      if (scanning) {
        manager.stopDeviceScan();
        setScanning(false);
      } else {
        manager.startDeviceScan(null, null, (error, device) => {
          if (error) {
            console.error("Scan error:", error);
            return;
          }

          if (
            device &&
            (device.name?.includes("Guitar") || device.name?.includes("Robot"))
          ) {
            setDevices((prevDevices) => {
              if (!prevDevices.some((d) => d.id === device.id)) {
                console.log("Found new device:", device);
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

  const selectDevice = (device: { name: string; id: string }) => {
    navigation.navigate("Control Device", { device });
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>Available Devices</Text>
      <ScrollView>
        {devices.map((device) => (
          <View
            key={device.id}
            style={styles.device}
            onTouchEnd={() => selectDevice(device)}
          >
            <Text>{`Device: ${device.name || "Unnamed"} (ID: ${
              device.id
            })`}</Text>
          </View>
        ))}
      </ScrollView>
    </View>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: "center",
    justifyContent: "center",
  },
  title: {
    fontSize: 20,
    fontWeight: "bold",
    marginVertical: 20,
  },
  device: {
    marginVertical: 10,
    paddingHorizontal: 10,
    paddingVertical: 5,
    borderWidth: 1,
    borderColor: "#ccc",
    borderRadius: 5,
  },
});
