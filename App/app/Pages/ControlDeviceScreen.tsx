import * as React from "react";
import { useState, useEffect } from "react";
import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import { TextInput, Button, StyleSheet, ScrollView } from "react-native";
import { RouteProp } from "@react-navigation/native";
import { RootStackParamList } from "../(tabs)";
import { BleService } from "../services/BleService";
import { Device } from "react-native-ble-plx";
import { SafeAreaProvider } from "react-native-safe-area-context";

interface ControlDeviceScreenProps {
  route: RouteProp<RootStackParamList, "Control Device">;
}

export default function ControlDeviceScreen({
  route,
}: ControlDeviceScreenProps) {
  const [command, setCommand] = useState<string>("");
  const [receivedCommands, setReceivedCommands] = useState<string[]>([]);
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [loading, setLoading] = useState<boolean>(true);

  if (!route || !route.params)
    return <ThemedText>route.params is undefined</ThemedText>;

  const { device } = route.params as { device: Device };
  const bleService = BleService.getInstance();

  const connectToDevice = () => {
    setLoading(true);
    bleService
      .connectToDevice(device)
      .then(() => {
        setIsConnected(true);
      })
      .catch((error: Error) => {
        console.error(error.message);
        setIsConnected(false);
      })
      .finally(() => {
        setLoading(false);
      });
  };

  useEffect(() => {
    connectToDevice();
  }, [device]);

  useEffect(() => {
    return () => {
      console.log("Cleaning up ControlDeviceScreen");
      if (isConnected) {
        console.log("Disconnecting from device");
        device.cancelConnection();
      }
    };
  }, [isConnected]);

  const sendCommand = () => {
    if (!isConnected) return;

    bleService
      .sendCommandToDevice(device, command)
      .then(() => {
        setReceivedCommands((prevCommands) => [...prevCommands, command]);
      })
      .catch((error: Error) => {
        console.error(error.message);
        setIsConnected(false); // Assume connection lost if sending command fails
      });
  };

  return (
    <SafeAreaProvider>
      <ThemedView style={styles.container}>
        <ThemedText
          style={styles.title}
        >{`Controlling Device: ${device.name}`}</ThemedText>
        <ThemedText style={styles.text}>{`Device ID: ${device.id}`}</ThemedText>
        {loading ? (
          <ThemedText style={styles.loadingText}>Connecting...</ThemedText>
        ) : isConnected ? (
          <ThemedView>
            <TextInput
              placeholder="Test"
              onChangeText={(text) => console.log(text)}
            />
            <Button title="Send Command" onPress={sendCommand} />
            <ScrollView style={styles.commandBox}>
              <ThemedText style={styles.commandTitle}>
                Commands Received:
              </ThemedText>
              {receivedCommands.map((cmd, index) => (
                <ThemedText key={index} style={styles.commandText}>
                  {cmd}
                </ThemedText>
              ))}
            </ScrollView>
          </ThemedView>
        ) : (
          <ThemedView>
            <ThemedText style={styles.errorText}>
              Failed to connect to device
            </ThemedText>
            <Button title="Retry" onPress={connectToDevice} />
          </ThemedView>
        )}
      </ThemedView>
    </SafeAreaProvider>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    alignItems: "center",
    justifyContent: "center",
  },
  title: {
    fontSize: 24,
    fontWeight: "bold",
    margin: 20,
  },
  text: {
    margin: 5,
  },
  textInput: {
    height: 40,
    borderColor: "gray",
    borderWidth: 1,
    margin: 20,
    width: "80%",
    paddingHorizontal: 10,
  },
  commandBox: {
    marginTop: 20,
    width: "80%",
    maxHeight: 200,
    borderColor: "gray",
    borderWidth: 1,
    padding: 10,
  },
  commandTitle: {
    fontWeight: "bold",
    margin: 10,
  },
  commandText: {
    fontSize: 16,
    margin: 5,
  },
  loadingText: {
    fontSize: 18,
    margin: 20,
  },
  errorText: {
    fontSize: 18,
    color: "gray",
  },
});
