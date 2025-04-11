import * as React from "react";
import { useState } from "react";
import {
  TextInput,
  StyleSheet,
  ScrollView,
  Platform,
  TouchableOpacity,
  Text,
  View,
} from "react-native";
import { BleService } from "../../Services/BleService";
import { Device } from "react-native-ble-plx";
import { SafeAreaProvider } from "react-native-safe-area-context";
import DocumentPicker from "react-native-document-picker";
import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import Toast from "react-native-toast-message";

export default function PlayTabScreen({ device }: { device: Device }) {
  const [command, setCommand] = useState<string>("");
  const [receivedCommands, setReceivedCommands] = useState<string[]>([]);
  const bleService = BleService.getInstance();

  const sendCommand = () => {
    if (!command.trim()) return;
    bleService
      .sendCommandToDevice(command)
      .then(() => {
        setReceivedCommands((prev) => [...prev, command]);
        setCommand("");
      })
      .catch((error: Error) =>
        Toast.show({
          type: "error",
          text1: "Command Error",
          text2: error.message,
          position: "top",
          visibilityTime: 2000,
          autoHide: true,
          topOffset: 50,
        })
      );
  };

  const handleFileUpload = async () => {
    try {
      const res = await DocumentPicker.pick({
        type: [DocumentPicker.types.allFiles],
        allowMultiSelection: false,
      });

      for (const file of res) {
        if (!file.uri) continue;
        await bleService.sendFileToDevice(file.uri);
        setReceivedCommands((prev) => [...prev, "File sent"]);
      }
    } catch (err) {
      Toast.show({
        type: "error",
        text1: "File Selection Error",
        text2: "Please try again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
    }
  };

  return (
    <ThemedView style={styles.container}>
      <ThemedText style={styles.heading}>Bluetooth Device Control</ThemedText>
      <View style={styles.card}>
        <Text style={styles.deviceName}>{device?.name || "Unknown"}</Text>
        <Text style={styles.deviceId}>ID: {device?.id}</Text>

        <TextInput
          style={styles.input}
          placeholder="Enter command"
          value={command}
          onChangeText={setCommand}
          placeholderTextColor="#888"
        />

        <TouchableOpacity style={styles.button} onPress={sendCommand}>
          <Text style={styles.buttonText}>Send Command</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={styles.buttonSecondary}
          onPress={handleFileUpload}
        >
          <Text style={styles.buttonText}>Upload File</Text>
        </TouchableOpacity>

        <View style={styles.commandLog}>
          <Text style={styles.logTitle}>Command History:</Text>
          <ScrollView>
            {receivedCommands.map((cmd, index) => (
              <Text key={index} style={styles.logItem}>
                {cmd}
              </Text>
            ))}
          </ScrollView>
        </View>
      </View>
    </ThemedView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingVertical: 20,
    alignItems: "center",
    justifyContent: "flex-start",
  },
  heading: {
    fontSize: 24,
    fontWeight: "bold",
    marginBottom: 20,
  },
  card: {
    backgroundColor: "#fff",
    padding: 20,
    borderRadius: 16,
    minWidth: "90%",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 4 },
    shadowOpacity: 0.1,
    shadowRadius: 8,
    elevation: 3,
  },
  deviceName: {
    fontSize: 20,
    fontWeight: "600",
    marginBottom: 5,
  },
  deviceId: {
    fontSize: 14,
    color: "#888",
    marginBottom: 15,
  },
  input: {
    height: 45,
    borderColor: "#ccc",
    borderWidth: 1,
    borderRadius: 10,
    paddingHorizontal: 15,
    marginBottom: 15,
  },
  button: {
    backgroundColor: "#007aff",
    paddingVertical: 12,
    borderRadius: 10,
    marginBottom: 10,
    alignItems: "center",
  },
  buttonSecondary: {
    backgroundColor: "#5856d6",
    paddingVertical: 12,
    borderRadius: 10,
    marginBottom: 15,
    alignItems: "center",
  },
  buttonText: {
    color: "#fff",
    fontWeight: "600",
    fontSize: 16,
  },
  commandLog: {
    maxHeight: 200,
    borderTopWidth: 1,
    borderTopColor: "#eee",
    paddingTop: 10,
  },
  logTitle: {
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 5,
  },
  logItem: {
    fontSize: 14,
    marginVertical: 2,
    color: "#333",
  },
});
