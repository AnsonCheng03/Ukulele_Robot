# File: midi_scheduler.py

import asyncio
import os
import tempfile
import time
from collections import defaultdict
import pretty_midi
from music21 import converter

class MidiScheduler:
    def __init__(self):
        self.current_task = None
        self.grouped_notes = []
        self.start_times = []
        self.paused = False
        self.pause_time = 0
        self.start_time = 0
        self.resume_offset = 0

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

        grouped_notes = defaultdict(list)
        for note in all_notes:
            grouped_notes[note["start"]].append({
                "note": note["note"],
                "octave": note["octave"],
                "duration": note["duration"]
            })

        self.grouped_notes = [grouped_notes[t] for t in sorted(grouped_notes)]
        self.start_times = sorted(grouped_notes)

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
        self.parse_pretty_midi(pmidi)

    async def schedule_notes(self, offset=0):
        prinyt(f"Scheduling notes with offset: {offset}")
        try:
            self.start_time = time.time() - offset
            for i, group_time in enumerate(self.start_times):
                if group_time < offset:
                    continue
                now = time.time()
                wait_time = group_time - (now - self.start_time)
                if wait_time > 0:
                    await asyncio.sleep(wait_time)
                if self.paused:
                    self.resume_offset = group_time
                    return
                for note in self.grouped_notes[i]:
                    print(f"NOTE @ {round(group_time, 2)}s: {note['note']}{note['octave']}")
        except Exception as e:  
            print(f"Error during playback: {e}")

    def play(self, path, offset=0):
        try:
            print(f"Playing {path} from {offset}s")
            self.parse_file(path)
            self.paused = False
            self.current_task = asyncio.create_task(self.schedule_notes(offset))
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

