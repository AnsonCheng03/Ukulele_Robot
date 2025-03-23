import React, { useRef, useState } from "react";
import {
  Animated,
  Dimensions,
  PanResponder,
  StyleSheet,
  View,
  TouchableWithoutFeedback,
} from "react-native";
import { MaterialIcons } from "@expo/vector-icons";

const SCREEN_HEIGHT = Dimensions.get("window").height;
const MAX_TRANSLATE_Y = SCREEN_HEIGHT * 0.4;
const MIN_TRANSLATE_Y = SCREEN_HEIGHT - 40;
const DRAG_THRESHOLD = 50;

export default function BottomDrawer({
  children,
}: {
  children: React.ReactNode;
}) {
  const translateY = useRef(new Animated.Value(MIN_TRANSLATE_Y)).current;
  const lastOffset = useRef(MIN_TRANSLATE_Y);
  const [open, setOpen] = useState(false);

  const animateDrawer = (toOpen: boolean) => {
    const toValue = toOpen ? MAX_TRANSLATE_Y : MIN_TRANSLATE_Y;
    Animated.timing(translateY, {
      toValue,
      duration: 300,
      useNativeDriver: true,
    }).start(() => {
      lastOffset.current = toValue;
      setOpen(toOpen);
    });
  };

  const panResponder = useRef(
    PanResponder.create({
      onMoveShouldSetPanResponder: (_, gestureState) => {
        return Math.abs(gestureState.dy) > 5;
      },
      onPanResponderMove: (_, gestureState) => {
        let newTranslateY = lastOffset.current + gestureState.dy;
        newTranslateY = Math.max(
          MAX_TRANSLATE_Y,
          Math.min(newTranslateY, MIN_TRANSLATE_Y)
        );
        translateY.setValue(newTranslateY);
      },
      onPanResponderRelease: (_, gestureState) => {
        const shouldOpen = gestureState.dy < -DRAG_THRESHOLD;
        animateDrawer(shouldOpen);
      },
    })
  ).current;

  const renderChildrenWithDividers = () => {
    const childrenArray = React.Children.toArray(children);
    return childrenArray.map((child, index) => (
      <View key={index} style={styles.childItem}>
        {child}
        {index < childrenArray.length - 1 && <View style={styles.divider} />}
      </View>
    ));
  };

  return (
    <Animated.View
      style={[styles.drawer, { transform: [{ translateY }] }]}
      {...panResponder.panHandlers}
    >
      <TouchableWithoutFeedback onPress={() => animateDrawer(!open)}>
        <View style={styles.handleIconWrapper}>
          <MaterialIcons
            name={open ? "keyboard-arrow-down" : "keyboard-arrow-up"}
            size={40}
            color="#666"
          />
        </View>
      </TouchableWithoutFeedback>
      <View style={styles.content}>{renderChildrenWithDividers()}</View>
    </Animated.View>
  );
}

const styles = StyleSheet.create({
  drawer: {
    position: "absolute",
    bottom: 25,
    height: SCREEN_HEIGHT,
    width: "100%",
    backgroundColor: "#fff",
    borderTopLeftRadius: 20,
    borderTopRightRadius: 20,
    shadowColor: "#000",
    shadowOffset: { width: 0, height: -3 },
    shadowOpacity: 0.2,
    shadowRadius: 5,
    elevation: 10,
    paddingTop: 6,
    zIndex: 1000,
  },
  handleIconWrapper: {
    alignItems: "center",
    justifyContent: "center",
    height: 60,
    marginBottom: 8,
  },
  content: {
    flex: 1,
    paddingHorizontal: 20,
    paddingTop: 10,
    paddingBottom: 20,
  },
  divider: {
    height: 1,
    backgroundColor: "#ddd",
    marginVertical: 12,
  },
  childItem: {
    paddingVertical: 4,
  },
});
