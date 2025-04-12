import React, { useEffect, useState, useRef } from "react";
import {
  View,
  StyleSheet,
  TouchableOpacity,
  ActivityIndicator,
} from "react-native";
import { Ionicons, MaterialCommunityIcons } from "@expo/vector-icons";
import { useNavigation, useTheme } from "@react-navigation/native";
import ConnectedScreen from "./ConnectedScreen";
import BleService from "../Services/BleService";
import { Device } from "react-native-ble-plx";
import { ThemedSafeAreaView, ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import Toast from "react-native-toast-message";

export default function ControlLayout() {
  const navigation = useNavigation();
  const bleService = BleService.getInstance();
  const device = bleService.getDevice();

  const { colors } = useTheme();

  const [loading, setLoading] = useState(true);
  const [isConnected, setIsConnected] = useState(false);
  const [showConnectedUI, setShowConnectedUI] = useState(false);

  const timeoutRef = useRef<NodeJS.Timeout | null>(null);

  const connectToDevice = async () => {
    if (!device) {
      setLoading(false);
      return;
    }
    try {
      setLoading(true);
      await bleService.connectToDevice();
      setIsConnected(true);
    } catch (e) {
      Toast.show({
        type: "error",
        text1: "Connection Failed",
        text2: "Please try again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
    } finally {
      setLoading(false);
    }
  };

  const checkConnectionStatus = async () => {
    if (!device) return;
    try {
      const connected = await device.isConnected();
      setIsConnected(connected);
    } catch {
      setIsConnected(false);
    }
  };

  useEffect(() => {
    connectToDevice();
    return () => timeoutRef.current && clearTimeout(timeoutRef.current);
  }, []);

  useEffect(() => {
    const interval = setInterval(checkConnectionStatus, 3000);
    return () => clearInterval(interval);
  }, [device]);

  useEffect(() => {
    if (isConnected) {
      timeoutRef.current = setTimeout(() => setShowConnectedUI(true), 1000);
    } else {
      setShowConnectedUI(false);
    }
  }, [isConnected]);

  const renderStatusIcon = () => {
    if (loading)
      return (
        <MaterialCommunityIcons
          name="bluetooth-connect"
          size={20}
          color="blue"
        />
      );
    if (isConnected)
      return (
        <MaterialCommunityIcons
          name="bluetooth-connect"
          size={20}
          color="green"
        />
      );
    if (!device)
      return (
        <MaterialCommunityIcons
          name="alert-circle-outline"
          size={20}
          color="gray"
        />
      );
    return (
      <MaterialCommunityIcons name="bluetooth-off" size={20} color="red" />
    );
  };

  return (
    <ThemedSafeAreaView style={styles.container}>
      <ThemedView style={styles.header}>
        <TouchableOpacity onPress={() => navigation.goBack()}>
          <Ionicons name="arrow-back" size={24} color={colors.text} />
        </TouchableOpacity>
        <View style={styles.statusContainer}>
          {renderStatusIcon()}
          <ThemedText style={styles.statusText}>
            {loading
              ? "Connecting..."
              : isConnected
              ? "Connected"
              : device
              ? "Disconnected"
              : "No device"}
          </ThemedText>
        </View>
      </ThemedView>

      {loading || !showConnectedUI ? (
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
