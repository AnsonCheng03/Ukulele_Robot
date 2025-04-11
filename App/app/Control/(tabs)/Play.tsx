import * as React from "react";
import { useState, useRef, useEffect, useCallback } from "react";
import {
  StyleSheet,
  ScrollView,
  Platform,
  TouchableOpacity,
  Text,
  View,
  FlatList,
  NativeScrollEvent,
  NativeSyntheticEvent,
  Dimensions,
  useWindowDimensions,
} from "react-native";
import { BleService } from "../../Services/BleService";
import { Device } from "react-native-ble-plx";
import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import Toast from "react-native-toast-message";

const ITEM_WIDTH = 70;

const noteAliases: Record<string, string> = {
  C: "C",
  "C#": "C#/Db",
  D: "D",
  "D#": "D#/Eb",
  E: "E",
  F: "F",
  "F#": "F#/Gb",
  G: "G",
  "G#": "G#/Ab",
  A: "A",
  "A#": "A#/Bb",
  B: "B",
};

export default function PlayTabScreen({ device }: { device: Device }) {
  const bleService = BleService.getInstance();
  const { width: screenWidth } = useWindowDimensions();
  const flatListRef = useRef<FlatList>(null);

  const baseNotes = Object.keys(noteAliases);

  const ukuleleRange = { min: 3 * 12, max: 6 * 12 + 11 }; // C3 to B6
  const data = Array.from(
    { length: ukuleleRange.max - ukuleleRange.min + 1 },
    (_, i) => ukuleleRange.min + i
  );

  const centerDataIndex = Math.floor(data.length / 2);

  const [selectedNoteIndex, setSelectedNoteIndex] = useState(centerDataIndex);
  const selectedAbsoluteIndex = data[selectedNoteIndex];
  const selectedNote = baseNotes[selectedAbsoluteIndex % 12];
  const selectedOctave = Math.floor(selectedAbsoluteIndex / 12);

  useEffect(() => {
    const visibleCenter = Math.floor(screenWidth / ITEM_WIDTH / 2);
    flatListRef.current?.scrollToOffset({
      offset: (selectedNoteIndex - visibleCenter) * ITEM_WIDTH,
      animated: false,
    });
  }, [screenWidth]);

  const chordTypes = [
    "Maj",
    "Min",
    "Maj7",
    "Min7",
    "7",
    "Sus2",
    "Sus4",
    "Dim",
    "Aug",
    "Add9",
  ];

  const getNoteWithOctave = (index: number) => {
    const note = baseNotes[index % 12];
    const octave = Math.floor(index / 12);
    return { note, octave };
  };

  const handleScrollEnd = (event: NativeSyntheticEvent<NativeScrollEvent>) => {
    const offsetX = event.nativeEvent.contentOffset.x;
    const centerIndex = Math.round(offsetX / ITEM_WIDTH);

    if (centerIndex >= 0 && centerIndex < data.length) {
      setSelectedNoteIndex(centerIndex);
      flatListRef.current?.scrollToOffset({
        offset: centerIndex * ITEM_WIDTH,
        animated: true,
      });
    } else if (centerIndex >= data.length) {
      setSelectedNoteIndex(data.length - 1);
      flatListRef.current?.scrollToOffset({
        offset: (data.length - 1) * ITEM_WIDTH,
        animated: true,
      });
    } else {
      setSelectedNoteIndex(0);
      flatListRef.current?.scrollToOffset({
        offset: 0,
        animated: true,
      });
    }
  };

  const NoteItem = React.memo(
    ({
      item,
      index,
      isSelected,
      onPress,
    }: {
      item: number;
      index: number;
      isSelected: boolean;
      onPress: () => void;
    }) => {
      const { note } = getNoteWithOctave(item);
      return (
        <TouchableOpacity style={styles.noteColumn} onPress={onPress}>
          <ThemedText
            style={[
              styles.noteLabel,
              isSelected ? styles.noteLabelSelected : styles.noteLabelDim,
            ]}
          >
            {note}
          </ThemedText>
          <View
            style={[styles.noteLine, isSelected && styles.noteLineSelected]}
          />
        </TouchableOpacity>
      );
    }
  );

  const renderNoteItem = useCallback(
    ({ item, index }: { item: number; index: number }) => (
      <NoteItem
        item={item}
        index={index}
        isSelected={index === selectedNoteIndex}
        onPress={() => {
          setSelectedNoteIndex(index);
          flatListRef.current?.scrollToOffset({
            offset: index * ITEM_WIDTH,
            animated: true,
          });
        }}
      />
    ),
    [selectedNoteIndex]
  );

  const sendCommand = (command: string) => {
    if (!command.trim()) return;
    bleService
      .sendCommandToDevice(command)
      .then(() => {
        Toast.show({
          type: "success",
          text1: "Command sent successfully",
          position: "top",
          visibilityTime: 2000,
          autoHide: true,
          topOffset: 50,
        });
      })
      .catch((error: Error) =>
        Toast.show({
          type: "error",
          text1: "Command Error",
          text2: error.message,
          position: "top",
          visibilityTime: 2000,
          autoHide: true,
          topOffset: 50,
        })
      );
  };

  return (
    <ThemedView style={styles.container}>
      <View style={styles.topSection}>
        <FlatList
          ref={flatListRef}
          data={data}
          horizontal
          onMomentumScrollEnd={handleScrollEnd}
          snapToInterval={ITEM_WIDTH}
          decelerationRate="fast"
          showsHorizontalScrollIndicator={false}
          getItemLayout={(_, index) => ({
            length: ITEM_WIDTH,
            offset: ITEM_WIDTH * index,
            index,
          })}
          initialNumToRender={12}
          maxToRenderPerBatch={12}
          removeClippedSubviews={true}
          scrollEventThrottle={16}
          windowSize={5}
          contentContainerStyle={{
            paddingHorizontal: screenWidth / 2 - ITEM_WIDTH / 2,
            alignItems: "flex-start",
          }}
          keyExtractor={(item) => item.toString()}
          renderItem={renderNoteItem}
        />
      </View>
      <View style={styles.middleSection}>
        <ThemedText style={styles.largeNote}>
          {noteAliases[selectedNote]}
        </ThemedText>
        <Text style={styles.largeOctave}>Octave {selectedOctave}</Text>
      </View>
      <View style={styles.bottomSection}>
        <ScrollView contentContainerStyle={styles.grid}>
          {chordTypes.map((type, idx) => (
            <TouchableOpacity
              key={idx}
              style={styles.chordButton}
              onPress={() =>
                sendCommand(`chord ${selectedNote} ${selectedOctave} ${type}`)
              }
            >
              <Text style={styles.chordButtonText}>{type}</Text>
            </TouchableOpacity>
          ))}
        </ScrollView>
      </View>
    </ThemedView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 20,
  },
  topSection: {
    paddingVertical: 10,
    flex: 7,
    justifyContent: "flex-end",
  },
  middleSection: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
  },
  bottomSection: {
    flex: 2,
    paddingHorizontal: 10,
  },
  noteColumn: {
    alignItems: "center",
    width: 70,
    paddingTop: 100,
  },
  noteLabel: {
    marginBottom: 5,
  },
  noteLabelSelected: {
    fontSize: 20,
    fontWeight: "bold",
  },
  noteLabelDim: {
    fontSize: 14,
    color: "#aaa",
  },
  noteLine: {
    height: "100%",
    width: 2,
    backgroundColor: "#ccc",
  },
  noteLineSelected: {
    width: 5,
    backgroundColor: "#007aff",
  },
  largeNote: {
    fontSize: 36,
    lineHeight: 40,
    fontWeight: "bold",
  },
  largeOctave: {
    fontSize: 24,
    color: "#666",
  },
  grid: {
    flexDirection: "row",
    flexWrap: "wrap",
    justifyContent: "center",
    gap: 10,
    paddingBottom: 20,
  },
  chordButton: {
    backgroundColor: "#ddd",
    paddingVertical: 10,
    paddingHorizontal: 16,
    borderRadius: 12,
    margin: 5,
    minWidth: 80,
    alignItems: "center",
  },
  chordButtonText: {
    fontSize: 14,
    fontWeight: "500",
  },
});
