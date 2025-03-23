import {
  DarkTheme,
  DefaultTheme,
  ThemeProvider,
} from "@react-navigation/native";
import { useFonts } from "expo-font";
import React, { useRef } from "react";
import * as SplashScreen from "expo-splash-screen";
import { StatusBar } from "expo-status-bar";
import { useEffect } from "react";
import "react-native-reanimated";
import { Easing, StyleSheet, TouchableOpacity, Animated } from "react-native";
import RippleBluetooth from "./Component/rippleBluetooth";

import { useColorScheme } from "@/hooks/useColorScheme";
import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";

// Prevent the splash screen from auto-hiding before asset loading is complete.
SplashScreen.preventAutoHideAsync();

export default function RootLayout() {
  const colorScheme = useColorScheme();
  const rippleAnim = useRef(new Animated.Value(0)).current;
  const [loaded] = useFonts({
    SpaceMono: require("../assets/fonts/SpaceMono-Regular.ttf"),
  });

  useEffect(() => {
    if (loaded) {
      SplashScreen.hideAsync();
    }
  }, [loaded]);

  useEffect(() => {
    Animated.loop(
      Animated.timing(rippleAnim, {
        toValue: 1,
        duration: 1500,
        easing: Easing.out(Easing.circle),
        useNativeDriver: true,
      })
    ).start();
  }, []);

  if (!loaded) {
    return null;
  }

  return (
    <ThemeProvider value={colorScheme === "dark" ? DarkTheme : DefaultTheme}>
      <ThemedView style={styles.container}>
        <ThemedText style={styles.header}>Bluetooth Pairing</ThemedText>

        <RippleBluetooth />

        <TouchableOpacity style={styles.bottomDrawer}>
          <ThemedView style={styles.drawerHandle} />
        </TouchableOpacity>
      </ThemedView>
      <StatusBar style="auto" />
    </ThemeProvider>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    paddingTop: 60,
    alignItems: "center",
    justifyContent: "space-between",
  },
  header: {
    fontSize: 20,
    fontWeight: "bold",
  },
  centerContainer: {
    alignItems: "center",
    justifyContent: "center",
    flex: 1,
  },
  statusText: {
    margin: 30,
    fontSize: 18,
    color: "#444",
  },
  bottomDrawer: {
    height: 60,
    width: "100%",
    backgroundColor: "#eee",
    borderTopLeftRadius: 20,
    borderTopRightRadius: 20,
    alignItems: "center",
    justifyContent: "center",
  },
  drawerHandle: {
    width: 40,
    height: 4,
    backgroundColor: "#aaa",
    borderRadius: 2,
  },
});