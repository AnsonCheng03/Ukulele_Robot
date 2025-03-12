import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";

import { StyleSheet, TextInput } from "react-native";

export default function TabTwoScreen() {
  return (
    <ThemedView style={styles.container}>
      <ThemedText style={styles.title}>Bluetooth Client</ThemedText>
      {/* <Text>Received Data: {receivedData}</Text> */}
      {/* <Button title="Send Command 'aaa'" onPress={() => sendCommand("aaa")} /> */}
      <ThemedText>Send Command</ThemedText>
      <TextInput placeholder="Command" />
    </ThemedView>
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
