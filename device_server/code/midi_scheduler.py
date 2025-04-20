import asyncio
import os
import traceback
import tempfile
import time
from loop_manager import global_asyncio_loop
from collections import defaultdict
import pretty_midi
from music21 import converter
from motor_control import send_motor_command, note_mapping, fretPositions, fretScaler 

class MidiScheduler:
    def __init__(self):
        self.current_task = None
        self.grouped_notes = []
        self.start_times = []
        self.paused = False
        self.pause_time = 0
        self.start_time = 0
        self.resume_offset = 0
        self.notes = []
        self.min_same_string_gap = 3_000_000  # default minimum gap in µs (100ms)
        self.precomputed_fingering_timeline = []

    def set_min_gap(self, micros):
        self.min_same_string_gap = micros

    def calculate_distance_from_fret(self, fret):
        if fret + 1 >= len(fretPositions):
            return None
        raw_position = (fretPositions[fret] + fretPositions[fret + 1]) / 2
        return raw_position * fretScaler

    def assign_fingerings_to_notes(self, notes, check_gap=True):
        active = {1: -9999, 2: -9999, 3: -9999, 4: -9999}
        result = []

        i = 0
        while i < len(notes):
            note_obj = notes[i]
            raw_note = note_obj["note"].upper()
            octave = note_obj.get("octave")
            duration = note_obj["duration"]
            current_time = note_obj["time"]
            end_time = current_time + duration

            octaves_to_check = [octave] if octave in note_mapping else note_mapping.keys()

            best_string = None
            best_fret = None
            latest_free_time = current_time
            found = False  # ✅ initialize here

            for o in octaves_to_check:
                if raw_note in note_mapping.get(o, {}):
                    for string, fret in note_mapping[o][raw_note]:
                        if active[string] <= current_time:
                            # ✅ Immediately use this available string
                            distance = self.calculate_distance_from_fret(fret)
                            if distance is None:
                                print(f"⚠️ Fret {fret} out of range for {raw_note}{o}")
                                continue
                            active[string] = end_time
                            result.append((raw_note, string, distance, end_time, current_time))
                            found = True
                            break
                        elif (not check_gap) or ((current_time - active[string]) * 1_000_000 >= self.min_same_string_gap):
                            if active[string] > latest_free_time:
                                latest_free_time = active[string]
                                best_string = string
                                best_fret = fret
                            if active[string] <= current_time:
                                # ✅ Immediately use this available string
                                distance = self.calculate_distance_from_fret(fret)
                                if distance is None:
                                    print(f"⚠️ Fret {fret} out of range for {raw_note}{o}")
                                    continue
                                active[string] = end_time
                                result.append((raw_note, string, distance, end_time, current_time))
                                found = True
                                break
                            else:
                                # Track latest free time if none are currently available
                                if active[string] > latest_free_time:
                                    latest_free_time = active[string]
                                    best_string = string
                                    best_fret = fret
                    if found:
                        break

            if found:
                i += 1
            elif best_string:
                distance = self.calculate_distance_from_fret(best_fret)
                if distance is None:
                    print(f"⚠️ Fret {best_fret} out of range for {raw_note}{octave}")
                    i += 1
                else:
                    delta = latest_free_time - current_time
                    note_obj["time"] += delta
                    for future_note in notes[i + 1:]:
                        future_note["time"] += delta
                    print(f"⏩ Shifted {raw_note}{octave} and future notes by {delta:.6f}s to wait for string availability")
                    active[best_string] = latest_free_time + duration
                    result.append((raw_note, best_string, distance, latest_free_time + duration, latest_free_time))
                i += 1
            else:
                print(f"⚠️ Could not assign string for {raw_note}{octave} at time {current_time}, currently active: {active}")
                result.append((raw_note, None, None, None, current_time))
                i += 1

        return result

    def precompute_fingering_timeline(self):
        self.precomputed_fingering_timeline = []
        all_notes = []
        for group, start_time in zip(self.grouped_notes, self.start_times):
            for note in group:
                note["time"] = start_time
                all_notes.append(note)

        assigned = self.assign_fingerings_to_notes(all_notes)
        for _, string, _, _, time in assigned:
            if string is not None:
                self.precomputed_fingering_timeline.append((string, time))


    def scale_timings(self, notes, min_gap):
        print(f"[Scheduler] Scaling timings with min gap: {min_gap}µs")

        original_fingerings = self.assign_fingerings_to_notes(notes, check_gap=False)
        by_string = defaultdict(list)
        for _, string, _, _, time in original_fingerings:
            if string is not None:
                by_string[string].append(time)

        shortest = float("inf")
        for string, times in by_string.items():
            times.sort()
            for i in range(1, len(times)):
                gap = times[i] - times[i - 1]
                if gap > 0:
                    shortest = min(shortest, gap)

        if shortest == float("inf"):
            print("[Scheduler] No valid same-string note gaps found — skipping scaling")
            return notes

        actual_shortest_micro = shortest * 1_000_000
        if actual_shortest_micro >= min_gap:
            return notes

        scale_factor = min_gap / actual_shortest_micro
        print(f"[Scheduler] Scaling all note timings by factor {scale_factor:.2f}")

        scaled_notes = []
        for note in notes:
            start = note["start"] * scale_factor
            end = note["end"] * scale_factor
            duration = end - start
            scaled_notes.append({
                **note,
                "start": round(start, 6),
                "end": round(end, 6),
                "duration": round(duration, 6),
                "time": round(start, 6),
            })

        print(f"[Scheduler] Shortest valid same-string gap: {shortest:.6f} sec")
        print(f"[Scheduler] Scaling factor: {scale_factor:.2f}")
        print("[Scheduler] First 5 scaled notes (time, duration):")
        for note in scaled_notes[:5]:
            print(f"  → {note['note']}{note['octave']} at {note['time']}s, duration {note['duration']}")

        return scaled_notes

    def get_motor_for_note(self, note, octave):
        for motor_id, note_map in note_mapping.items():
            if note in note_map:
                return motor_id
        return None

    def note_number_to_components(self, note_number):
        name_with_octave = pretty_midi.note_number_to_name(note_number)
        if len(name_with_octave) == 3:
            return name_with_octave[:2], int(name_with_octave[2])
        else:
            return name_with_octave[0], int(name_with_octave[1])

    def parse_pretty_midi(self, pmidi):
        all_notes = []
        for instrument in pmidi.instruments:
            for note in instrument.notes:
                note_name, octave = self.note_number_to_components(note.pitch)
                all_notes.append({
                    "start": round(note.start, 3),
                    "end": round(note.end, 3),
                    "note": note_name,
                    "octave": octave,
                    "duration": round(note.end - note.start, 3)
                })
        return all_notes

    def parse_mxl_to_pretty_midi(self, input_file):
        score = converter.parse(input_file)
        with tempfile.NamedTemporaryFile(delete=False, suffix=".mid") as tmp:
            temp_midi_path = tmp.name
        score.write('midi', fp=temp_midi_path)
        pmidi = pretty_midi.PrettyMIDI(temp_midi_path)
        os.remove(temp_midi_path)
        return pmidi

    def parse_file(self, path):
        ext = os.path.splitext(path)[1].lower()
        if ext in [".mxl", ".musicxml", ".xml"]:
            pmidi = self.parse_mxl_to_pretty_midi(path)
        elif ext == ".mid":
            pmidi = pretty_midi.PrettyMIDI(path)
        else:
            raise ValueError("Unsupported file type.")
        return pmidi

    async def schedule_notes(self, offset=0):
        print(f"Scheduling notes with offset: {offset}")
        try:
            self.start_time = time.time() - offset
            all_notes = []
            for group, start_time in zip(self.grouped_notes, self.start_times):
                for note in group:
                    note["time"] = start_time
                    all_notes.append(note)

            fingerings = self.assign_fingerings_to_notes(all_notes)

            for note, string, dist, _, current_time in fingerings:
                if current_time < offset:
                    continue

                now = time.time()
                wait_time = current_time - (now - self.start_time)
                if wait_time > 0:
                    await asyncio.sleep(wait_time)

                if self.paused:
                    self.resume_offset = current_time
                    return

                # print(f"Scheduling note: {note} on string {string} with distance {dist} at time {current_time}")
                if string is not None:
                    send_motor_command(string, 2, 0, dist)
                # else:
                #     print(f"⚠️ No motor mapped for {note}")

        except Exception as e:
            print(f"Error during playback: {e}")

    def play(self, path, offset=0):
        try:
            print(f"Playing {path} from {offset}s")
            pmidi = self.parse_file(path)
            all_notes = self.parse_pretty_midi(pmidi)
            for n in all_notes:
                n["time"] = n["start"]
            grouped = defaultdict(list)
            for n in all_notes:
                grouped[n["start"]].append(n)
            grouped_times = sorted(grouped)

            # Flatten back into note list with time assigned
            clustered_notes = []
            for t in grouped_times:
                for note in grouped[t]:
                    note["time"] = t
                    clustered_notes.append(note)
            scaled_notes = self.scale_timings(clustered_notes, self.min_same_string_gap)

            print(f"[DEBUG] Scaled first 5 notes:")
            for note in scaled_notes[:5]:
                print(f"  {note['note']}{note['octave']} — time: {note['time']}, dur: {note['duration']}")

            self.notes = scaled_notes
            grouped_notes = defaultdict(list)
            for note in scaled_notes:
                grouped_notes[note["time"]].append({
                    "note": note["note"],
                    "octave": note["octave"],
                    "duration": note["duration"],
                    "time": note["time"]
                })

            self.grouped_notes = [grouped_notes[t] for t in sorted(grouped_notes)]
            self.start_times = sorted(grouped_notes)

            self.paused = False
            if self.current_task:
                self.current_task.cancel()

            print("Scheduling coroutine now...")
            self.current_task = asyncio.run_coroutine_threadsafe(
                self.schedule_notes(offset),
                global_asyncio_loop
            )
        except Exception as e:
            print(f"Error in play request: {e}")

    def pause(self):
        self.paused = True
        self.pause_time = time.time()
        if self.current_task:
            self.current_task.cancel()
            self.current_task = None

    def resume(self):
        if self.resume_offset:
            self.play(self.last_file, self.resume_offset)
            self.resume_offset = 0