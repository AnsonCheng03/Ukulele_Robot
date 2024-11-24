import React, { useState, useEffect } from "react";
import {
  View,
  Text,
  TextInput,
  Button,
  StyleSheet,
  ScrollView,
} from "react-native";
import { RouteProp } from "@react-navigation/native";
import { RootStackParamList } from "../(tabs)/index";
import { BleManager, Device } from "react-native-ble-plx";

interface ControlDeviceScreenProps {
  route: RouteProp<RootStackParamList, "Control Device">;
}

const manager = new BleManager();

export default function ControlDeviceScreen({
  route,
}: ControlDeviceScreenProps) {
  const [command, setCommand] = useState("");
  const [receivedCommands, setReceivedCommands] = useState<string[]>([]);
  const [isConnected, setIsConnected] = useState(false);
  const [loading, setLoading] = useState(true);

  if (!route || !route.params) return <Text>route.params is undefined</Text>;

  const { device } = route.params;

  useEffect(() => {
    device
      .connect()
      .then((device) => {
        console.log("Discovering services and characteristics");
        return device.discoverAllServicesAndCharacteristics();
      })
      .then(() => {
        setIsConnected(true);
        setLoading(false);
      })
      .catch((error) => {
        console.error(error.message);
        setLoading(false);
      });
  }, [device]);

  const sendCommand = () => {
    if (!isConnected) return;

    device
      .discoverAllServicesAndCharacteristics()
      .then(async (device) => {
        const services = await device.services();
        for (const service of services) {
          const characteristics = await service.characteristics();
          for (const characteristic of characteristics) {
            if (characteristic.isWritableWithResponse) {
              console.log(
                `Service UUID: ${service.uuid}, Characteristic UUID: ${characteristic.uuid}`
              );
              return device.writeCharacteristicWithResponseForService(
                service.uuid,
                characteristic.uuid,
                btoa(command)
              );
            }
          }
        }
        throw new Error("No writable characteristic found");
      })
      .then((characteristic) => {
        console.log(characteristic.value);
        setReceivedCommands((prevCommands) => [...prevCommands, command]);
      })
      .catch((error) => {
        console.error(error.message);
      });
  };

  return (
    <View style={styles.container}>
      <Text style={styles.title}>{`Controlling Device: ${device.name}`}</Text>
      <Text style={styles.text}>{`Device ID: ${device.id}`}</Text>
      {loading ? (
        <Text style={styles.loadingText}>Connecting...</Text>
      ) : isConnected ? (
        <>
          <TextInput
            style={styles.textInput}
            placeholder="Enter command"
            placeholderTextColor="white"
            value={command}
            onChangeText={(text) => setCommand(text)}
          />
          <Button title="Send Command" onPress={sendCommand} />
          <ScrollView style={styles.commandBox}>
            <Text style={styles.commandTitle}>Commands Received:</Text>
            {receivedCommands.map((cmd, index) => (
              <Text key={index} style={styles.commandText}>
                {cmd}
              </Text>
            ))}
          </ScrollView>
        </>
      ) : (
        <Text style={styles.errorText}>Failed to connect to device</Text>
      )}
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
    fontSize: 24,
    fontWeight: "bold",
    marginBottom: 20,
    color: "white",
  },
  text: {
    color: "white",
  },
  textInput: {
    height: 40,
    borderColor: "gray",
    borderWidth: 1,
    marginBottom: 20,
    width: "80%",
    paddingHorizontal: 10,
    color: "white",
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
    marginBottom: 10,
    color: "white",
  },
  commandText: {
    fontSize: 16,
    marginBottom: 5,
    color: "white",
  },
  loadingText: {
    fontSize: 18,
    color: "white",
  },
  errorText: {
    fontSize: 18,
    color: "red",
  },
});
