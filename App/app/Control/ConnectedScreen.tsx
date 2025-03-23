import React, { useState } from "react";
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  SafeAreaView,
} from "react-native";
import {
  Entypo,
  FontAwesome5,
  MaterialCommunityIcons,
} from "@expo/vector-icons";
import PlayTabScreen from "./(tabs)/Play";
import EditingTabScreen from "./(tabs)/Editing";
import ScoreTabScreen from "./(tabs)/Score";
import ChordTabScreen from "./(tabs)/Chord";
import { Device } from "react-native-ble-plx";
import DebugTabScreen from "./(tabs)/Debug";

export default function ConnectedScreen({ device }: { device: Device | null }) {
  if (!device) {
    return (
      <SafeAreaView style={styles.middle}>
        <Text style={styles.tabContent}>No device connected</Text>
      </SafeAreaView>
    );
  }

  const [tab, setTab] = useState<
    "chord" | "play" | "editing" | "score" | "debug"
  >("play");
  const renderContent = () => {
    switch (tab) {
      case "editing":
        return <EditingTabScreen device={device} />;
      case "score":
        return <ScoreTabScreen device={device} />;
      case "chord":
        return <ChordTabScreen device={device} />;
      case "debug":
        return <DebugTabScreen device={device} />;
      default:
      case "play":
        return <PlayTabScreen device={device} />;
    }
  };

  return (
    <>
      <View style={styles.middle}>{renderContent()}</View>

      <View style={styles.bottomTabs}>
        <TouchableOpacity
          onPress={() => setTab("play")}
          style={styles.tabButton}
        >
          <Entypo
            name="controller-play"
            size={22}
            color={tab === "play" ? "#007AFF" : "#999"}
          />
          <Text style={tab === "play" ? styles.tabActive : styles.tab}>
            Play
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          onPress={() => setTab("chord")}
          style={styles.tabButton}
        >
          <MaterialCommunityIcons
            name="guitar-electric"
            size={22}
            color={tab === "chord" ? "#007AFF" : "#999"}
          />
          <Text style={tab === "chord" ? styles.tabActive : styles.tab}>
            Chord
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          onPress={() => setTab("editing")}
          style={styles.tabButton}
        >
          <Entypo
            name="text"
            size={22}
            color={tab === "editing" ? "#007AFF" : "#999"}
          />
          <Text style={tab === "editing" ? styles.tabActive : styles.tab}>
            Editing
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          onPress={() => setTab("score")}
          style={styles.tabButton}
        >
          <FontAwesome5
            name="music"
            size={20}
            color={tab === "score" ? "#007AFF" : "#999"}
          />
          <Text style={tab === "score" ? styles.tabActive : styles.tab}>
            Score
          </Text>
        </TouchableOpacity>

        <TouchableOpacity
          onPress={() => setTab("debug")}
          style={styles.tabButton}
        >
          <MaterialCommunityIcons
            name="bug"
            size={22}
            color={tab === "debug" ? "#007AFF" : "#999"}
          />
          <Text style={tab === "debug" ? styles.tabActive : styles.tab}>
            Debug
          </Text>
        </TouchableOpacity>
      </View>
    </>
  );
}
const styles = StyleSheet.create({
  middle: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
  },
  tabContent: {
    fontSize: 20,
    fontWeight: "600",
  },
  bottomTabs: {
    flexDirection: "row",
    justifyContent: "space-around",
    borderTopWidth: 1,
    borderTopColor: "#ddd",
    paddingVertical: 8,
  },
  tabButton: {
    alignItems: "center",
    gap: 5,
    paddingVertical: 10,
    justifyContent: "center",
  },
  tab: {
    fontSize: 12,
    color: "#999",
  },
  tabActive: {
    fontSize: 12,
    fontWeight: "bold",
    color: "#007AFF",
  },
});
