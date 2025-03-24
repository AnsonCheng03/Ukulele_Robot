import React, { useEffect, useState, useRef } from "react";
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  SafeAreaView,
  ActivityIndicator,
} from "react-native";
import { Ionicons, MaterialCommunityIcons } from "@expo/vector-icons";
import { useNavigation } from "@react-navigation/native";
import ConnectedScreen from "./ConnectedScreen";
import BleService from "../Services/BleService";
import { Device } from "react-native-ble-plx";
import { ThemedSafeAreaView, ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";

export default function ControlLayout() {
  const navigation = useNavigation();
  const bleService = BleService.getInstance();
  const device = bleService.getDevice();
  const [loading, setLoading] = useState<boolean>(true);
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [currentDevice, setCurrentDevice] = useState<Device | null>(device);
  const loadingTimerRef = useRef<NodeJS.Timeout | null>(null);

  const checkConnectionStatus = async () => {
    if (!device) return false;
    try {
      return await device.isConnected();
    } catch {
      return false;
    }
  };

  const connectToDevice = () => {
    if (!device) {
      console.error("Device is not defined");
      setLoading(false);
      return;
    }

    const startTime = Date.now();
    setLoading(true);
    console.log("Connecting to device:", device);
    bleService
      .connectToDevice()
      .then((connectedDevice: Device | void) => {
        if (!connectedDevice) {
          console.error("No device connected");
          setLoading(false);
          return;
        }

        setCurrentDevice(connectedDevice);
        setIsConnected(true);
      })
      .catch((error: Error) => {
        console.error(error.message);
        setIsConnected(false);
      })
      .finally(() => {
        const elapsed = Date.now() - startTime;
        const delay = Math.max(1000 - elapsed, 0);
        if (loadingTimerRef.current) clearTimeout(loadingTimerRef.current);
        loadingTimerRef.current = setTimeout(() => {
          setLoading(false);
        }, delay);
      });
  };

  useEffect(() => {
    if (device) {
      connectToDevice();
    }
    return () => {
      if (loadingTimerRef.current) clearTimeout(loadingTimerRef.current);
    };
  }, [device]);

  useEffect(() => {
    const interval = setInterval(async () => {
      const connected = await checkConnectionStatus();
      setIsConnected(connected);
    }, 3000);
    return () => clearInterval(interval);
  }, [device]);

  const renderStatusIcon = () => {
    if (isConnected && !loading)
      return (
        <MaterialCommunityIcons
          name="bluetooth-connect"
          size={20}
          color="green"
        />
      );
    if (loading)
      return (
        <MaterialCommunityIcons
          name="bluetooth-connect"
          size={20}
          color="blue"
        />
      );
    if (!currentDevice) {
      return (
        <MaterialCommunityIcons
          name="alert-circle-outline"
          size={20}
          color="gray"
        />
      );
    }
    return (
      <MaterialCommunityIcons name="bluetooth-off" size={20} color="red" />
    );
  };

  return (
    <ThemedSafeAreaView style={styles.container}>
      <ThemedView style={styles.header}>
        <TouchableOpacity onPress={() => navigation.goBack()}>
          <Ionicons name="arrow-back" size={24} color="black" />
        </TouchableOpacity>
        <View style={styles.statusContainer}>
          {renderStatusIcon()}
          <ThemedText style={styles.statusText}>
            {loading
              ? "Connecting..."
              : isConnected
              ? "Connected"
              : currentDevice
              ? "Disconnected"
              : "No device found"}
          </ThemedText>
        </View>
      </ThemedView>

      {loading || !isConnected ? (
        <View style={styles.centered}>
          <ActivityIndicator size="large" color="#0000ff" />
          <ThemedText style={{ marginTop: 10 }}>Connecting...</ThemedText>
        </View>
      ) : (
        <ConnectedScreen device={device} />
      )}
    </ThemedSafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
  },
  header: {
    paddingHorizontal: 20,
    paddingTop: 10,
    paddingBottom: 10,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  statusContainer: {
    flexDirection: "row",
    alignItems: "center",
    gap: 6,
  },
  statusText: {
    fontSize: 14,
    fontWeight: "600",
  },
  centered: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    gap: 10,
  },
});
