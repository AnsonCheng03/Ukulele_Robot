import { ThemedText } from "@/components/ThemedText";
import { ThemedView } from "@/components/ThemedView";
import RNFS from "react-native-fs";
import DocumentPicker from "react-native-document-picker";

import { Button, StyleSheet, TextInput } from "react-native";

export default function TabTwoScreen() {
  return (
    <ThemedView style={styles.container}>
      <ThemedText style={styles.title}>Bluetooth Client</ThemedText>
      {/* <Text>Received Data: {receivedData}</Text> */}
      {/* <Button title="Send Command 'aaa'" onPress={() => sendCommand("aaa")} /> */}

      <Button
        title="Upload File"
        onPress={async () => {
          // retrieve file from device
          console.log("Uploading file");
          DocumentPicker.pick({
            type: [DocumentPicker.types.allFiles],
            allowMultiSelection: false,
          })
            .then(async (res) => {
              // get file content
              for (const file of res) {
                console.log("File picked", res);
                if (!file.uri) {
                  console.log("No file uri found");
                  continue;
                }
                const fileContent = await RNFS.readFile(file.uri, "base64");
                console.log("Sending file to device", fileContent);
              }
            })
            .catch((err: any) => {
              console.log("err", err);
            });
        }}
      />
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
