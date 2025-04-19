import React, { useRef } from "react";
import {
  View,
  ScrollView,
  Pressable,
  StyleSheet,
  Text,
  Dimensions,
  Animated,
  Easing,
} from "react-native";
import { Device } from "react-native-ble-plx";
import { BleService } from "../../Services/BleService";
import Toast from "react-native-toast-message";

const screenWidth = Dimensions.get("window").width;
const screenHeight = Dimensions.get("window").height;

const NOTE_SEQUENCE = [
  "C",
  "C#",
  "D",
  "D#",
  "E",
  "F",
  "F#",
  "G",
  "G#",
  "A",
  "A#",
  "B",
];

function getNoteName(baseNote: string, baseOctave: number, fret: number) {
  const baseIndex = NOTE_SEQUENCE.indexOf(baseNote);
  const totalHalfSteps = baseIndex + fret;
  const note = NOTE_SEQUENCE[totalHalfSteps % 12];
  const octave = baseOctave + Math.floor(totalHalfSteps / 12);
  return `${note}${octave}`;
}

export default function FingeringPage({ device }: { device: Device }) {
  const bleService = BleService.getInstance();
  const numStrings = 4;
  const spacing = screenWidth / (numStrings + 1);

  const fretPositions = [
    0.0, 19.6, 38.1, 55.57, 72.05, 87.61, 102.29, 116.14, 129.24, 141.59,
    153.24, 164.24, 174.83, 184.43, 193.2, 202.41, 210.26, 218.43, 225.77, 233.0,
  ];
  const scaleMM = fretPositions[fretPositions.length - 1];
  const totalHeight = screenHeight * 2.4;

  const stringTuning = ["G", "C", "E", "A"];
  const stringOctaves = [4, 4, 4, 4];

  const vibration = useRef(
    Array(numStrings)
      .fill(null)
      .map(() => new Animated.Value(0))
  ).current;

  const handlePress = (x: number, y: number) => {
    const stringCenters = Array.from(
      { length: numStrings },
      (_, i) => spacing * (i + 1)
    );
    let closestString = 0;
    let minDist = Infinity;
    for (let i = 0; i < numStrings; i++) {
      const dist = Math.abs(x - stringCenters[i]);
      if (dist < minDist) {
        minDist = dist;
        closestString = i;
      }
    }

    const stringID = closestString + 1;
    const mmPressed = (y / totalHeight) * scaleMM;

    let closestFret = 0;
    let minDiff = Infinity;
    for (let i = 0; i < fretPositions.length - 1; i++) {
      const center = (fretPositions[i] + fretPositions[i + 1]) / 2;
      const diff = Math.abs(center - mmPressed);
      if (diff < minDiff) {
        minDiff = diff;
        closestFret = i;
      }
    }

    const distanceMM = Math.round(fretPositions[closestFret] * (310 / 141.59));
    const note = getNoteName(
      stringTuning[closestString],
      stringOctaves[closestString],
      closestFret
    );
    const cmd = `move ${stringID} ${distanceMM}`;
    console.log(`Sending command: ${cmd} | ${distanceMM}mm`);

    Toast.show({
      type: "success",
      text1: `Pressed Fret ${closestFret}, String ${stringID} → ${note}`,
      position: "top",
      visibilityTime: 1500,
      topOffset: 60,
    });

    Animated.timing(vibration[closestString], {
      toValue: 1,
      duration: 400,
      easing: Easing.bounce,
      useNativeDriver: true,
    }).start(() => vibration[closestString].setValue(0));

    bleService.sendCommandToDevice(cmd).catch((err) =>
      Toast.show({
        type: "error",
        text1: "Command Error",
        text2: err.message,
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      })
    );
  };

  const renderStrings = () => {
    const lines = [];
    for (let i = 0; i < numStrings; i++) {
      const left = spacing * (i + 1);
      const translateX = vibration[i].interpolate({
        inputRange: [0, 0.25, 0.5, 0.75, 1],
        outputRange: [0, -3, 3, -3, 0],
      });

      for (let fret = 0; fret < fretPositions.length; fret++) {
        const mm = fretPositions[fret];
        const top = (mm / scaleMM) * totalHeight;
        const noteName = getNoteName(stringTuning[i], stringOctaves[i], fret);
        lines.push(
          <Text
            key={`note-${i}-${fret}`}
            style={[styles.noteLabel, { top: top + 5, left: left - 45, textAlign:"right", width: 40 }]}
          >
            {noteName}
          </Text>
        );
      }

      lines.push(
        <Animated.View
          key={`string-${i}`}
          style={[styles.verticalLine, { left, transform: [{ translateX }] }]}
          pointerEvents="none"
        />
      );

    }
    return lines;
  };

  const renderFrets = () => {
    const lines = [];
    for (let i = 0; i < fretPositions.length; i++) {
      const mm = fretPositions[i];
      const top = (mm / scaleMM) * totalHeight;

      lines.push(
        <View key={`fret-${i}`} style={[styles.horizontalLine, { top }]}>
          <Text style={[styles.hintText, { left: 4, top: 5 }]}>Fret {i}</Text>
          <Text style={[styles.hintText, { right: 4, top: 5 }]}>Fret {i}</Text>
        </View>
      );
    }
    return lines;
  };

  return (
    <ScrollView
      contentContainerStyle={{
        height: totalHeight, // Add padding for safety
      }}
      style={styles.scrollView}
      bounces={false}
      overScrollMode="never"
    >
      <View style={styles.container}>
        {renderStrings()}
        {renderFrets()}

        <Pressable
          style={{
            position: "absolute",
            top: 0,
            left: 0,
            right: 0,
            bottom: 0,
            height: totalHeight + 300,
          }}
          onPress={(e) =>
            handlePress(e.nativeEvent.locationX, e.nativeEvent.locationY)
          }
        />
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    position: "relative",
    borderLeftWidth: 24,
    borderRightWidth: 24,
    borderColor: "transparent",
  },
  scrollView: {
    width: "100%",
    marginVertical: 20,
  },
  verticalLine: {
    position: "absolute",
    top: 0,
    bottom: 0,
    width: 1,
    backgroundColor: "#a0a0a0",
    opacity: 0.8,
    borderRadius: 2,
    shadowColor: "#000",
    shadowOffset: { width: 1, height: 2 },
    shadowOpacity: 0.2,
    shadowRadius: 2,
  },
  horizontalLine: {
    position: "absolute",
    left: 0,
    right: 0,
    height: 1.5,
    backgroundColor: "#9c8067",
    flexDirection: "row",
    justifyContent: "space-between",
    paddingHorizontal: 8,
  },
  hintText: {
    fontSize: 10,
    color: "#bbb",
    position: "absolute",
    top: -8,
  },
  noteLabel: {
    position: "absolute",
    fontSize: 9,
    color: "#4a3b2e",
    fontWeight: "500",
  },
});
