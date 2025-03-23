import React, { useRef, useState } from "react";
import {
  Animated,
  Dimensions,
  PanResponder,
  StyleSheet,
  View,
  ScrollView,
  TouchableWithoutFeedback,
} from "react-native";
import { MaterialIcons } from "@expo/vector-icons";

const SCREEN_HEIGHT = Dimensions.get("window").height;
const MAX_TRANSLATE_Y = SCREEN_HEIGHT * 0.6;
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
  const scrollOffsetY = useRef(0);

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

  const renderChildrenWithDividers = () => {
    const childrenArray = React.Children.toArray(children);
    return childrenArray.map((child, index) => (
      <View key={index} style={{ flex: 1 }} style={styles.childItem}>
        {child}
        {index < childrenArray.length - 1 && <View style={styles.divider} />}
      </View>
    ));
  };

  return (
    <Animated.View style={[styles.drawer, { transform: [{ translateY }] }]}>
      <TouchableWithoutFeedback onPress={() => animateDrawer(!open)}>
        <View style={styles.handleIconWrapper}>
          <MaterialIcons
            name={open ? "keyboard-arrow-down" : "keyboard-arrow-up"}
            size={40}
            color="#666"
          />
        </View>
      </TouchableWithoutFeedback>
      <ScrollView
        contentContainerStyle={{ flexGrow: 1 }}
        bounces={false}
        showsVerticalScrollIndicator={false}
        style={styles.scrollView}
        scrollEventThrottle={16}
        onScroll={(e) => {
          scrollOffsetY.current = e.nativeEvent.contentOffset.y;
        }}
      >
        <View style={styles.content}>{renderChildrenWithDividers()}</View>
      </ScrollView>
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
  scrollView: {
    flex: 1,
    maxHeight: SCREEN_HEIGHT * 0.4 - 40,
  },
  divider: {
    height: 1,
    backgroundColor: "#ddd",
    marginVertical: 6,
  },
  childItem: {
    paddingVertical: 4,
  },
});
