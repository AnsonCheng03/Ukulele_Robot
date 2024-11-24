import React, { useEffect, useState } from "react";
import {
  StyleSheet,
  ScrollView,
  Platform,
  PermissionsAndroid,
  Button,
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
          getConnectedDevices();
          startBluetoothScan();
        }
      });
    } else {
      getConnectedDevices();
      startBluetoothScan();
    }

    return () => {
      manager.stopDeviceScan();
      manager.destroy();
    };
  }, []);

  const getConnectedDevices = async () => {
    try {
      // Replace with valid UUIDs of your target services
      const serviceUUIDs = [
        "00001800-0000-1000-8000-00805f9b34fb", // Generic Access
        "0000180a-0000-1000-8000-00805f9b34fb", // Device Information
      ];

      const connectedDevices = await manager.connectedDevices(serviceUUIDs);

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

          if (device && device.name) {
            setDevices((prevDevices) => {
              if (!prevDevices.some((d) => d.id === device.id)) {
                console.log(`Found device: ${device.name}`, device);

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
      <View style={styles.titleContainer}>
        <Text style={styles.title}>Available Devices</Text>
        <View style={{ flexDirection: "row" }}>
          <Button title="Refresh" onPress={getConnectedDevices} />
          <Button
            title="Clear"
            onPress={() =>
              setDevices((prevDevices) =>
                prevDevices.filter((device) => device.isPaired)
              )
            }
          />
        </View>
      </View>
      <ScrollView>
        {devices
          .sort((a, b) => {
            const aPriority =
              a.name?.includes("Guitar") || a.name?.includes("Robot") ? -1 : 1;
            const bPriority =
              b.name?.includes("Guitar") || b.name?.includes("Robot") ? -1 : 1;
            return aPriority - bPriority;
          })
          .map((device) => (
            <View
              key={device.id}
              style={
                device.name?.includes("Guitar") ||
                device.name?.includes("Robot")
                  ? styles.highlightedDevice
                  : styles.device
              }
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
  titleContainer: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    width: "100%",
    paddingHorizontal: 20,
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
  highlightedDevice: {
    marginVertical: 10,
    paddingHorizontal: 10,
    paddingVertical: 5,
    borderWidth: 1,
    borderColor: "#ff9800",
    borderRadius: 5,
  },
});
