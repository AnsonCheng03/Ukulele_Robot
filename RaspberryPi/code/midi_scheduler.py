import asyncio
import os
import tempfile
import time
from collections import defaultdict

import pretty_midi
from music21 import converter

from loop_manager import global_asyncio_loop
from motor_control import calculate_distance_from_fret, send_motor_command, note_mapping

class MidiScheduler:
    def __init__(self):
        self._reset_state()

    def _reset_state(self):
        self.current_task = None
        self.grouped_notes = []
        self.start_times = []
        self.paused = False
        self.pause_time = 0
        self.start_time = 0
        self.resume_offset = 0
        self.notes = []
        self.min_same_string_gap = 3_000_000  # µs
        self.precomputed_fingering_timeline = []

    def set_min_gap(self, micros):
        self.min_same_string_gap = micros

    def estimate_movement_time_us(self, string_number, distance_mm):
        ratio = 0.1  # tenths of sec per mm
        scale = 100_000  # to µs
        rack_up = rack_down = 1_000_000
        pluck_pulse = 10_000
        slider_us = int(distance_mm * ratio * scale)
        return rack_up + slider_us + rack_down + pluck_pulse

    def assign_fingerings_to_notes(self, notes, check_gap=True):
        active = {s: -9999 for s in range(1, 5)}
        result = []
        EPSILON = 1e-4

        for i in range(len(notes)):
            note_obj = notes[i]
            raw_note = note_obj["note"].upper()
            octave = note_obj.get("octave")
            start_time = note_obj["time"]
            duration = note_obj["duration"]
            end_time = start_time + duration
            found, best = False, (None, None, start_time)

            octaves = [octave] if octave in note_mapping else note_mapping.keys()

            for o in octaves:
                if raw_note not in note_mapping.get(o, {}):
                    continue
                for string, fret in note_mapping[o][raw_note]:
                    gap_ok = not check_gap or ((start_time - active[string]) * 1_000_000 >= self.min_same_string_gap - EPSILON)
                    if active[string] <= start_time + EPSILON:
                        dist = calculate_distance_from_fret(fret)
                        if dist is not None:
                            active[string] = end_time
                            result.append((raw_note, string, dist, end_time, start_time))
                            found = True
                            break
                    elif gap_ok and active[string] > best[2]:
                        best = (string, fret, active[string])
                if found:
                    break

            if found:
                continue

            if best[0] is not None:
                string, fret, latest_time = best
                dist = calculate_distance_from_fret(fret)
                if dist is None:
                    print(f"⚠️ Fret {fret} out of range for {raw_note}{octave}")
                    continue
                delta = latest_time - start_time
                note_obj["time"] += delta
                for j in range(i + 1, len(notes)):
                    notes[j]["time"] += delta
                active[string] = latest_time + duration
                result.append((raw_note, string, dist, latest_time + duration, latest_time))
                print(f"⏩ Shifted {raw_note}{octave} by {delta:.6f}s to wait for string {string}")
            else:
                print(f"⚠️ Could not assign string for {raw_note}{octave} at time {start_time}")
                result.append((raw_note, None, None, None, start_time))

        return result

    def precompute_fingering_timeline(self):
        all_notes = []
        for group, start in zip(self.grouped_notes, self.start_times):
            for note in group:
                note["time"] = start
                all_notes.append(note)
        fingerings = self.assign_fingerings_to_notes(all_notes)
        self.precomputed_fingering_timeline = [(s, t) for _, s, _, _, t in fingerings if s is not None]

    def scale_timings(self, notes, min_gap):
        print(f"[Scheduler] Scaling with min gap {min_gap}µs")
        raw = self.assign_fingerings_to_notes(notes, check_gap=False)
        by_string = defaultdict(list)
        for _, s, _, _, t in raw:
            if s is not None:
                by_string[s].append(t)

        min_gap_s = float("inf")
        for times in by_string.values():
            times.sort()
            for a, b in zip(times, times[1:]):
                gap = b - a
                if gap > 0:
                    min_gap_s = min(min_gap_s, gap)

        if min_gap_s == float("inf"):
            print("[Scheduler] No valid gap — skipping scaling")
            return notes

        actual_gap_us = min_gap_s * 1_000_000
        if actual_gap_us >= min_gap:
            return notes

        scale = min_gap / actual_gap_us
        print(f"[Scheduler] Scale factor: {scale:.2f}")

        scaled = []
        for n in notes:
            s, e = n["start"] * scale, n["end"] * scale
            scaled.append({
                **n,
                "start": round(s, 6),
                "end": round(e, 6),
                "duration": round(e - s, 6),
                "time": round(s, 6),
            })

        for note in scaled[:5]:
            print(f"  → {note['note']}{note['octave']} @ {note['time']}, dur {note['duration']}")

        return scaled

    def parse_file_to_notes(self, path):
        ext = os.path.splitext(path)[1].lower()
        if ext in [".mxl", ".musicxml", ".xml"]:
            score = converter.parse(path)
            with tempfile.NamedTemporaryFile(delete=False, suffix=".mid") as tmp:
                score.write('midi', fp=tmp.name)
                midi = pretty_midi.PrettyMIDI(tmp.name)
            os.remove(tmp.name)
        elif ext == ".mid":
            midi = pretty_midi.PrettyMIDI(path)
        else:
            raise ValueError("Unsupported file type.")

        notes = []
        for inst in midi.instruments:
            for note in inst.notes:
                name = pretty_midi.note_number_to_name(note.pitch)
                pitch = name[:2] if len(name) == 3 else name[0]
                octave = int(name[-1])
                notes.append({
                    "start": round(note.start, 3),
                    "end": round(note.end, 3),
                    "note": pitch,
                    "octave": octave,
                    "duration": round(note.end - note.start, 3),
                    "time": round(note.start, 3)
                })
        return notes

    def apply_physical_shift(self, notes):
        grouped_by_time = defaultdict(list)
        for note in notes:
            grouped_by_time[note["time"]].append(note)

        for group_time, notes_in_group in grouped_by_time.items():
            max_shift_us = 0
            for note in notes_in_group:
                raw_note = note["note"].upper()
                octave = note.get("octave")
                for o in [octave] if octave in note_mapping else note_mapping:
                    if raw_note in note_mapping[o]:
                        string, fret = note_mapping[o][raw_note][0]
                        distance = calculate_distance_from_fret(fret)
                        if distance is not None:
                            move_us = self.estimate_movement_time_us(string, distance)
                            max_shift_us = max(max_shift_us, move_us)
                        break
            shift_s = max_shift_us / 1_000_000
            for note in notes_in_group:
                note["time"] = max(0, note["time"] - shift_s)
        return notes

    def group_notes_by_time(self, notes):
        grouped = defaultdict(list)
        for note in notes:
            grouped[note["time"]].append({
                "note": note["note"],
                "octave": note["octave"],
                "duration": note["duration"],
                "time": note["time"]
            })
        self.grouped_notes = [grouped[t] for t in sorted(grouped)]
        self.start_times = sorted(grouped)
        
    def prepare_motor_distances(self, group):
        distances = [None] * 4  # Strings 1–4
        for note in group:
            raw_note = note["note"].upper()
            octave = note.get("octave")
            for o in [octave] if octave in note_mapping else note_mapping:
                if raw_note in note_mapping[o]:
                    string, fret = note_mapping[o][raw_note][0]
                    distance = calculate_distance_from_fret(fret)
                    if distance is not None:
                        distances[string - 1] = distance
                    break
        return distances

    async def schedule_notes(self, offset=0):
        print(f"Scheduling notes with offset: {offset}")
        try:
            self.start_time = time.time() - offset

            for group, current_time in zip(self.grouped_notes, self.start_times):
                distances = self.prepare_motor_distances(group)

                now = time.time()
                wait_time = current_time - (now - self.start_time)
                if wait_time > 0:
                    await asyncio.sleep(wait_time)

                if self.paused:
                    self.resume_offset = current_time
                    return

                if any(d is not None for d in distances):
                    dist_out = [d if d is not None else -2 for d in distances]
                    print(f"[Scheduler] Sending MF @ t={current_time:.3f}s → {dist_out}")
                    send_motor_command(0, 6, *dist_out)

        except Exception as e:
            print(f"Error during playback: {e}")

    def play(self, path, offset=0):
        try:
            print(f"Playing {path} from {offset}s")
            notes = self.parse_file_to_notes(path)
            scaled_notes = self.scale_timings(notes, self.min_same_string_gap)
            shifted_notes = self.apply_physical_shift(scaled_notes)
            self.group_notes_by_time(shifted_notes)
            self.notes = shifted_notes
            self.paused = False
            self.last_file = path

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