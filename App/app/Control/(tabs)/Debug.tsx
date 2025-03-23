import * as React from "react";
import { useState } from "react";
import {
  Text,
  View,
  StyleSheet,
  TouchableOpacity,
  ScrollView,
  TextInput,
} from "react-native";
import { Device } from "react-native-ble-plx";
import { SafeAreaProvider } from "react-native-safe-area-context";
import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import { BleService } from "../../Services/BleService";

export default function PlayTabScreen({ device }: { device: Device }) {
  const bleService = BleService.getInstance();

  const [mode, setMode] = useState<"control" | "move">("control");
  const [motorID, setMotorID] = useState(0);
  const [target, setTarget] = useState(0);
  const [speed, setSpeed] = useState(0);
  const [direction, setDirection] = useState(0);
  const [duration, setDuration] = useState(0);
  const [distance, setDistance] = useState(0);
  const [receivedCommands, setReceivedCommands] = useState<string[]>([]);

  const increase = (
    val: number,
    setVal: (v: number) => void,
    step = 1,
    max = 999
  ) => {
    setVal(Math.min(val + step, max));
  };
  const decrease = (
    val: number,
    setVal: (v: number) => void,
    step = 1,
    min = 0
  ) => {
    setVal(Math.max(val - step, min));
  };

  const sendControlCommand = () => {
    const cmd = `control ${motorID} ${target} ${speed} ${direction} ${duration}`;
    bleService
      .sendCommandToDevice(cmd)
      .then(() => setReceivedCommands((prev) => [...prev, cmd]))
      .catch((err) => console.error(err));
  };

  const sendMoveCommand = () => {
    const cmd = `move ${motorID} ${target} ${distance}`;
    bleService
      .sendCommandToDevice(cmd)
      .then(() => setReceivedCommands((prev) => [...prev, cmd]))
      .catch((err) => console.error(err));
  };

  const renderStepper = (
    label: string,
    value: number,
    setValue: (v: number) => void,
    step = 1,
    max = 999
  ) => (
    <View style={styles.stepperContainer}>
      <ThemedText style={styles.label}>{label}</ThemedText>
      <View style={styles.stepperRow}>
        <TouchableOpacity
          onPress={() => decrease(value, setValue, step)}
          style={styles.arrowBtn}
        >
          <Text style={styles.arrowText}>-</Text>
        </TouchableOpacity>
        <ThemedText style={styles.valueText}>{value}</ThemedText>
        <TouchableOpacity
          onPress={() => increase(value, setValue, step, max)}
          style={styles.arrowBtn}
        >
          <Text style={styles.arrowText}>+</Text>
        </TouchableOpacity>
      </View>
    </View>
  );

  return (
    <SafeAreaProvider>
      <ThemedView style={styles.container}>
        <ThemedText style={styles.heading}>Debug</ThemedText>

        <ThemedView style={styles.modeSelector}>
          <TouchableOpacity
            style={[styles.modeButton, mode === "control" && styles.activeMode]}
            onPress={() => setMode("control")}
          >
            <ThemedText style={styles.modeText}>Control Mode</ThemedText>
          </TouchableOpacity>
          <TouchableOpacity
            style={[styles.modeButton, mode === "move" && styles.activeMode]}
            onPress={() => setMode("move")}
          >
            <ThemedText style={styles.modeText}>Move Mode</ThemedText>
          </TouchableOpacity>
        </ThemedView>

        <View style={styles.separator} />

        <ScrollView contentContainerStyle={styles.formContainer}>
          {renderStepper("Motor ID", motorID, setMotorID, 1, 5)}
          {renderStepper("Target", target, setTarget, 1, 3)}

          {mode === "control" && (
            <>
              {renderStepper("Speed", speed, setSpeed, 10, 500)}
              {renderStepper("Direction", direction, setDirection, 1, 1)}
              {renderStepper("Duration", duration, setDuration, 1, 999)}
              <TouchableOpacity
                style={styles.button}
                onPress={sendControlCommand}
              >
                <Text style={styles.buttonText}>Send Control</Text>
              </TouchableOpacity>
            </>
          )}

          {mode === "move" && (
            <>
              {renderStepper("Distance", distance, setDistance, 10, 999)}
              <TouchableOpacity
                style={styles.buttonSecondary}
                onPress={sendMoveCommand}
              >
                <Text style={styles.buttonText}>Send Move</Text>
              </TouchableOpacity>
            </>
          )}
        </ScrollView>

        <View style={styles.separator} />

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
      </ThemedView>
    </SafeAreaProvider>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
  },
  heading: {
    fontSize: 28,
    fontWeight: "bold",
    textAlign: "center",
    marginBottom: 20,
  },
  modeSelector: {
    flexDirection: "row",
    justifyContent: "center",
    marginBottom: 20,
  },
  modeButton: {
    paddingVertical: 10,
    paddingHorizontal: 20,
    marginHorizontal: 10,
    borderRadius: 10,
    backgroundColor: "#aaa",
  },
  activeMode: {
    backgroundColor: "#007aff",
  },
  modeText: {
    color: "#fff",
    fontWeight: "600",
  },
  separator: {
    height: 1,
    backgroundColor: "#ccc",
    width: "100%",
    marginVertical: 20,
  },
  formContainer: {
    paddingBottom: 20,
  },
  stepperContainer: {
    marginBottom: 16,
    width: "100%",
  },
  label: {
    fontSize: 16,
    marginBottom: 8,
    fontWeight: "500",
  },
  stepperRow: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    gap: 10,
  },
  arrowBtn: {
    backgroundColor: "#ddd",
    padding: 8,
    borderRadius: 6,
    width: 40,
    alignItems: "center",
  },
  arrowText: {
    fontSize: 18,
    fontWeight: "bold",
  },
  valueText: {
    fontSize: 16,
    width: 50,
    textAlign: "center",
  },
  button: {
    backgroundColor: "#007aff",
    paddingVertical: 12,
    borderRadius: 10,
    alignItems: "center",
    marginTop: 10,
  },
  buttonSecondary: {
    backgroundColor: "#5856d6",
    paddingVertical: 12,
    borderRadius: 10,
    alignItems: "center",
    marginTop: 10,
  },
  buttonText: {
    color: "white",
    fontWeight: "600",
    fontSize: 16,
  },
  commandLog: {
    maxHeight: 180,
  },
  logTitle: {
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 10,
  },
  logItem: {
    fontSize: 14,
    marginBottom: 4,
    color: "#333",
  },
});
