import React, { useRef } from "react";
import { useEffect } from "react";
import "react-native-reanimated";
import { View, Easing, StyleSheet, Animated } from "react-native";
import { MaterialCommunityIcons } from "@expo/vector-icons";

export default function rippleBluetooth() {
  const rippleAnim = useRef(new Animated.Value(0)).current;

  const rippleScale = rippleAnim.interpolate({
    inputRange: [0, 1],
    outputRange: [1, 2.5],
  });

  const rippleOpacity = rippleAnim.interpolate({
    inputRange: [0, 1],
    outputRange: [0.6, 0],
  });

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

  return (
    <View style={styles.rippleWrapper}>
      <Animated.View
        style={[
          styles.rippleCircle,
          {
            transform: [{ scale: rippleScale }],
            opacity: rippleOpacity,
            position: "absolute",
          },
        ]}
      />
      <View style={styles.iconCircle}>
        <MaterialCommunityIcons name="bluetooth" size={40} color="#007AFF" />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  rippleCircle: {
    width: 120,
    height: 120,
    borderRadius: 60,
    borderWidth: 2,
    borderColor: "#007AFF",
    alignItems: "center",
    justifyContent: "center",
  },

  rippleWrapper: {
    width: 120,
    height: 120,
    alignItems: "center",
    justifyContent: "center",
  },
  iconCircle: {
    width: 80,
    height: 80,
    borderRadius: 40,
    backgroundColor: "#fff",
    alignItems: "center",
    justifyContent: "center",
    zIndex: 1,
  },
});
