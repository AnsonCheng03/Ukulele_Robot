// FoundDeviceModal.tsx
import React from "react";
import {
  Modal,
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  Dimensions,
} from "react-native";
import { Ionicons } from "@expo/vector-icons";

interface FoundDeviceModalProps {
  visible: boolean;
  onClose: () => void;
  onPair: () => void;
  device: { name: string; id: string } | null;
}

const FoundDeviceModal = ({
  visible,
  onClose,
  onPair,
  device,
}: FoundDeviceModalProps) => {
  if (!device) return null;

  return (
    <Modal transparent animationType="fade" visible={visible}>
      <View style={styles.overlay}>
        <View style={styles.modalContainer}>
          {/* Close Button */}
          <View style={styles.header}>
            <Text style={styles.title}>Device Found</Text>
            <TouchableOpacity style={styles.closeButton} onPress={onClose}>
              <Ionicons name="close" size={28} color="#333" />
            </TouchableOpacity>
          </View>

          <View style={styles.content}>
            <Ionicons
              name="musical-notes-outline"
              size={60}
              color="#007AFF"
              style={styles.icon}
            />

            {/* Device Info */}
            <Text style={styles.deviceName}>
              {device.name || "Unnamed Device"}
            </Text>
            <Text style={styles.deviceId}>{device.id}</Text>
          </View>

          {/* Pair Button */}
          <TouchableOpacity style={styles.pairButton} onPress={onPair}>
            <Text style={styles.pairText}>Pair Now</Text>
          </TouchableOpacity>
        </View>
      </View>
    </Modal>
  );
};

export default FoundDeviceModal;

const { width, height } = Dimensions.get("window");

const styles = StyleSheet.create({
  overlay: {
    flex: 1,
    justifyContent: "flex-end",
    backgroundColor: "rgba(0,0,0,0.3)",
  },
  modalContainer: {
    width: width * 0.9,
    height: height * 0.35,
    backgroundColor: "#fff",
    alignSelf: "center",
    marginBottom: 20,
    borderRadius: 20,
    padding: 30,
    position: "relative",
  },
  header: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    marginBottom: 20,
  },
  closeButton: {
    padding: 10,
    borderRadius: 50,
    backgroundColor: "#f0f0f0",
  },
  title: {
    fontSize: 28,
    fontWeight: "bold",
    marginBottom: 12,
  },

  content: {
    alignItems: "center",
    justifyContent: "center",
    flex: 1,
    paddingVertical: 20,
    gap: 20,
    borderTopWidth: 1,
    borderTopColor: "#eee",
    borderBottomWidth: 1,
    borderBottomColor: "#eee",
  },
  icon: {
    marginBottom: 12,
  },
  deviceName: {
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 4,
  },
  deviceId: {
    fontSize: 12,
    color: "#666",
    marginBottom: 16,
  },
  pairButton: {
    backgroundColor: "#007AFF",
    paddingVertical: 8,
    paddingHorizontal: 15,
    alignItems: "center",
    borderRadius: 25,
  },
  pairText: {
    color: "#fff",
    fontWeight: "bold",
  },
});
