import React, { useEffect, useState } from "react";
import { StyleSheet, Button, TextInput } from "react-native";
import { Text, View } from "@/components/Themed";

export default function TabTwoScreen() {
  return (
    <View style={styles.container}>
      <Text style={styles.title}>Bluetooth Client</Text>
      {/* <Text>Received Data: {receivedData}</Text> */}
      {/* <Button title="Send Command 'aaa'" onPress={() => sendCommand("aaa")} /> */}
      <Text>Send Command</Text>
      <TextInput placeholder="Command" />
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
  },
  separator: {
    marginVertical: 30,
    height: 1,
    width: "80%",
  },
});
