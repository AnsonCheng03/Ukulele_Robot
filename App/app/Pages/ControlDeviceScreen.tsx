import React from "react";
import { View, Text, StyleSheet } from "react-native";
import { RouteProp } from "@react-navigation/native";
import { RootStackParamList } from "../(tabs)/index";

interface ControlDeviceScreenProps {
  route: RouteProp<RootStackParamList, "Control Device">;
}

export default function ControlDeviceScreen({
  route,
}: ControlDeviceScreenProps) {
  if (!route || !route.params) return <Text>route.params is undefined</Text>;

  const { device } = route.params;

  return (
    <View style={styles.container}>
      <Text style={styles.title}>{`Controlling Device: ${device.name}`}</Text>
      <Text>{`Device ID: ${device.id}`}</Text>
      {/* Add more controls and actions to interact with the device here */}
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
  },
});
