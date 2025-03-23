import React, { useRef } from "react";
import { useEffect } from "react";
import "react-native-reanimated";
import { View, Easing, StyleSheet, Animated } from "react-native";
import { MaterialCommunityIcons } from "@expo/vector-icons";

export default function rippleBluetooth() {
  const rippleCount = 3;
  const duration = 2000;
  const ripples = Array.from({ length: rippleCount }).map(
    () => useRef(new Animated.Value(0)).current
  );

  useEffect(() => {
    ripples.forEach((rippleAnim, index) => {
      Animated.loop(
        Animated.timing(rippleAnim, {
          toValue: 1,
          duration,
          delay: index * (duration / rippleCount),
          easing: Easing.out(Easing.circle),
          useNativeDriver: true,
        })
      ).start();
    });
  }, []);

  return (
    <View style={styles.rippleWrapper}>
      {ripples.map((rippleAnim, index) => {
        const scale = rippleAnim.interpolate({
          inputRange: [0, 1],
          outputRange: [0, 2.5],
        });

        const opacity = rippleAnim.interpolate({
          inputRange: [0, 1],
          outputRange: [0.4, 0],
        });

        return (
          <Animated.View
            key={index}
            style={[
              styles.rippleCircle,
              {
                transform: [{ scale }],
                opacity,
                position: "absolute",
              },
            ]}
          />
        );
      })}

      <View style={styles.iconCircle}>
        <MaterialCommunityIcons name="bluetooth" size={50} color="#007AFF" />
      </View>
    </View>
  );
}

const styles = StyleSheet.create({
  rippleCircle: {
    width: 160,
    height: 160,
    borderRadius: "100%",
    borderWidth: 2,
    borderColor: "#007AFF",
    alignItems: "center",
    justifyContent: "center",
  },

  rippleWrapper: {
    width: 160,
    height: 160,
    alignItems: "center",
    justifyContent: "center",
  },
  iconCircle: {
    width: 160,
    height: 160,
    borderRadius: "100%",
    backgroundColor: "#fff",
    alignItems: "center",
    justifyContent: "center",
    zIndex: 1,
  },
});
