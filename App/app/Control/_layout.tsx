import React, { useState } from "react";
import {
  View,
  Text,
  StyleSheet,
  TouchableOpacity,
  SafeAreaView,
} from "react-native";
import {
  Ionicons,
  Entypo,
  FontAwesome5,
  MaterialCommunityIcons,
} from "@expo/vector-icons";
import { useNavigation } from "@react-navigation/native";
import PlayTabScreen from "./(tabs)/Play";
import EditingTabScreen from "./(tabs)/Editing";
import ScoreTabScreen from "./(tabs)/Score";
import ChordTabScreen from "./(tabs)/Chord";

export default function ControlLayout() {
  const navigation = useNavigation();
  const [tab, setTab] = useState<"chord" | "play" | "editing" | "score">(
    "chord"
  );

  const renderContent = () => {
    switch (tab) {
      case "editing":
        return <EditingTabScreen />;
      case "score":
        return <ScoreTabScreen />;
      case "chord":
        return <ChordTabScreen />;
      default:
      case "play":
        return <PlayTabScreen />;
    }
  };

  return (
    <SafeAreaView style={styles.container}>
      {/* Top Bar */}
      <View style={styles.header}>
        <TouchableOpacity onPress={() => navigation.goBack()}>
          <Ionicons name="arrow-back" size={24} color="black" />
        </TouchableOpacity>
        <Text style={styles.status}>● Connected</Text>
      </View>

      {/* Center (Hexagon placeholder) */}
      <View style={styles.middle}>{renderContent()}</View>

      {/* Bottom Tab Navigation */}
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
      </View>
    </SafeAreaView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    backgroundColor: "#fff",
  },
  header: {
    paddingHorizontal: 20,
    paddingTop: 10,
    paddingBottom: 10,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  status: {
    fontSize: 14,
    color: "green",
    fontWeight: "600",
  },
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
