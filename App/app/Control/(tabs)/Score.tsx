import * as React from "react";
import { useEffect, useState } from "react";
import {
  StyleSheet,
  ScrollView,
  TouchableOpacity,
  Text,
  View,
  FlatList,
  Pressable,
} from "react-native";
import { BleService } from "../../Services/BleService";
import { Device } from "react-native-ble-plx";
import Toast from "react-native-toast-message";
import DocumentPicker from "react-native-document-picker";
import { ThemedView } from "@/components/ThemedView";
import { ThemedText } from "@/components/ThemedText";
import Slider from "@react-native-community/slider";

export default function PlayTabScreen({ device }: { device: Device }) {
  const bleService = BleService.getInstance();
  const [audioFiles, setAudioFiles] = useState<
    { name: string; modified: string }[]
  >([]);
  const [deleteMode, setDeleteMode] = useState(false);
  const [selected, setSelected] = useState<string[]>([]);

  const [currentFile, setCurrentFile] = useState<string | null>(null);
  const [currentTime, setCurrentTime] = useState<number>(0);
  const [duration, setDuration] = useState<number>(120); // Replace with real later
  const [isPlaying, setIsPlaying] = useState<boolean>(false);

  const [uploadProgress, setUploadProgress] = useState<number | null>(null);
  const [uploadTotal, setUploadTotal] = useState<number | null>(null);

  useEffect(() => {
    let interval: NodeJS.Timeout;
    if (isPlaying) {
      interval = setInterval(() => {
        setCurrentTime((t) => {
          if (t >= duration) {
            setIsPlaying(false);
            return t;
          }
          return t + 1;
        });
      }, 1000);
    }
    return () => clearInterval(interval);
  }, [isPlaying, duration]);

  const formatTime = (secs: number): string => {
    const min = Math.floor(secs / 60);
    const sec = Math.floor(secs % 60);
    return `${min}:${sec.toString().padStart(2, "0")}`;
  };

  const playPrev = () => {
    const index = audioFiles.findIndex((f) => f.name === currentFile);
    if (index > 0) {
      handlePlay(audioFiles[index - 1].name);
    }
  };

  const playNext = () => {
    const index = audioFiles.findIndex((f) => f.name === currentFile);
    if (index >= 0 && index < audioFiles.length - 1) {
      handlePlay(audioFiles[index + 1].name);
    }
  };

  const handlePlay = async (name: string) => {
    try {
      if (deleteMode) {
        toggleSelect(name);
      } else {
        await bleService.playAudioFile(name, 0);
        setCurrentFile(name);
        setCurrentTime(0);
        setIsPlaying(true);
      }
    } catch (error) {
      Toast.show({
        type: "error",
        text1: "Playback Error",
        text2: "Please try again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
    }
  };

  useEffect(() => {
    fetchAudioFiles();
  }, []);

  const fetchAudioFiles = async () => {
    const files = await bleService.getAudioFileList();
    setAudioFiles(files);
  };

  const handleFileUpload = async () => {
    try {
      const res = await DocumentPicker.pick({
        type: [DocumentPicker.types.allFiles],
        allowMultiSelection: false,
      });

      for (const file of res) {
        if (!file.uri) continue;
        setUploadProgress(0);
        setUploadTotal(null);
        await bleService.sendFileToDevice(file.uri, (current, total) => {
          setUploadProgress(current);
          setUploadTotal(total);
        });
        setUploadProgress(null);
        setUploadTotal(null);
      }

      await fetchAudioFiles(); // ✅ Ensure this runs after all uploads
    } catch (err) {
      Toast.show({
        type: "error",
        text1: "File Selection Error",
        text2: "Please try again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
      setUploadProgress(null);
      setUploadTotal(null);
    }
  };

  const handleDelete = async () => {
    try {
      await bleService.deleteAudioFiles(selected);
      setSelected([]);
      setDeleteMode(false);
      await fetchAudioFiles(); // ✅ Wait for it to complete
    } catch (error) {
      Toast.show({
        type: "error",
        text1: "Delete Error",
        text2: "Please try again.",
        position: "top",
        visibilityTime: 2000,
        autoHide: true,
        topOffset: 50,
      });
    }
  };
  const toggleSelect = (name: string) => {
    setSelected((prev) =>
      prev.includes(name) ? prev.filter((f) => f !== name) : [...prev, name]
    );
  };

  return (
    <ThemedView style={styles.container}>
      {uploadProgress !== null && uploadTotal !== null && (
        <View style={{ marginBottom: 10 }}>
          <Text>
            Uploading: {uploadProgress} / {uploadTotal}
          </Text>
          <Slider
            disabled
            value={uploadProgress}
            minimumValue={0}
            maximumValue={uploadTotal}
          />
        </View>
      )}

      <View style={styles.rowButtons}>
        <TouchableOpacity style={styles.uploadBtn} onPress={handleFileUpload}>
          <ThemedText style={styles.buttonText}>Upload File</ThemedText>
        </TouchableOpacity>
        <TouchableOpacity
          style={[styles.deleteBtn, deleteMode && styles.deleteActive]}
          onPress={() => (deleteMode ? handleDelete() : setDeleteMode(true))}
        >
          <ThemedText style={styles.buttonText}>
            {deleteMode ? "Confirm Delete" : "Delete Files"}
          </ThemedText>
        </TouchableOpacity>
      </View>

      <FlatList
        data={audioFiles}
        keyExtractor={(item) => item.name}
        renderItem={({ item }) => (
          <Pressable
            style={[
              styles.fileItem,
              selected.includes(item.name) && styles.selectedItem,
            ]}
            onPress={() => handlePlay(item.name)}
          >
            <ThemedText style={styles.fileName}>{item.name}</ThemedText>
            <ThemedText style={styles.modified}>{item.modified}</ThemedText>
          </Pressable>
        )}
      />

      {currentFile && (
        <View style={styles.controlPanel}>
          <ThemedText style={styles.nowPlaying}>{currentFile}</ThemedText>
          <View style={styles.timeRow}>
            <Text style={styles.timeText}>{formatTime(currentTime)}</Text>
            <Slider
              style={{ flex: 1 }}
              value={currentTime}
              minimumValue={0}
              maximumValue={duration}
              onSlidingComplete={(val) => {
                setCurrentTime(val);
                bleService.playAudioFile(currentFile, Math.floor(val));
              }}
            />
            <Text style={styles.timeText}>{formatTime(duration)}</Text>
          </View>
          <View style={styles.controlsRow}>
            <TouchableOpacity onPress={playPrev}>
              <Text style={styles.controlBtn}>⏮️</Text>
            </TouchableOpacity>
            <TouchableOpacity
              onPress={async () => {
                try {
                  if (isPlaying) {
                    await bleService.pauseAudioFile();
                    setIsPlaying(false);
                  } else if (currentFile) {
                    await bleService.playAudioFile(
                      currentFile,
                      Math.floor(currentTime)
                    );
                    setIsPlaying(true);
                  }
                } catch (error) {
                  Toast.show({
                    type: "error",
                    text1: "Playback Error",
                    text2: "Please try again.",
                    position: "top",
                    visibilityTime: 2000,
                    autoHide: true,
                    topOffset: 50,
                  });
                }
              }}
            >
              <Text style={styles.controlBtn}>{isPlaying ? "⏸️" : "▶️"}</Text>
            </TouchableOpacity>
            <TouchableOpacity onPress={playNext}>
              <Text style={styles.controlBtn}>⏭️</Text>
            </TouchableOpacity>
          </View>
        </View>
      )}
    </ThemedView>
  );
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    padding: 16,
  },
  rowButtons: {
    flexDirection: "row",
    justifyContent: "space-between",
    marginBottom: 12,
    gap: "5%",
  },
  uploadBtn: {
    backgroundColor: "#007aff",
    padding: 10,
    borderRadius: 8,
    width: "47.5%",
    alignItems: "center",
  },
  deleteBtn: {
    backgroundColor: "#ff3b30",
    padding: 10,
    borderRadius: 8,

    width: "47.5%",
    alignItems: "center",
  },
  deleteActive: {
    backgroundColor: "#ff9500",
  },
  fileItem: {
    padding: 12,
    borderRadius: 10,
    borderColor: "#ddd",
    borderWidth: 1,
    marginBottom: 8,
  },
  selectedItem: {
    backgroundColor: "#ffe4e1",
  },
  fileName: {
    fontSize: 16,
    fontWeight: "600",
  },
  modified: {
    fontSize: 12,
    color: "#666",
  },
  buttonText: {
    color: "#fff",
    fontWeight: "600",
  },
  controlPanel: {
    padding: 16,
    backgroundColor: "#f2f2f2",
    borderTopWidth: 1,
    borderColor: "#ccc",
    marginTop: 8,
  },
  nowPlaying: {
    fontSize: 16,
    fontWeight: "600",
    marginBottom: 8,
  },
  timeRow: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 10,
    gap: 8,
  },
  timeText: {
    width: 40,
    textAlign: "center",
    color: "#333",
  },
  controlsRow: {
    flexDirection: "row",
    justifyContent: "space-around",
    alignItems: "center",
  },
  controlBtn: {
    fontSize: 24,
    paddingHorizontal: 12,
  },
});
