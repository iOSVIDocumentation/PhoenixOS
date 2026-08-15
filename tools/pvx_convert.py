#!/usr/bin/env python3
"""PVX converter: обычное видео -> 160x120 RGB565BE + чиптюн-ноты для PhoenixOS.
Использование: python3 pvx_convert.py input.mp4 out.pvx [--fps 30] [--no-audio]"""
import argparse, os, struct, subprocess, wave, array

def run(cmd):
    print(" ", " ".join(cmd))
    subprocess.run(cmd, check=True)

def extract_notes(wav_path):
    with wave.open(wav_path, "rb") as w:
        rate = w.getframerate()
        raw = w.readframes(w.getnframes())
    s = array.array("h"); s.frombytes(raw)
    win = int(rate * 0.05)
    notes = []
    for i in range(0, len(s) - win, win):
        chunk = s[i:i + win]
        acc = 0
        for v in chunk: acc += v * v
        rms = (acc / win) ** 0.5
        if rms < 300:
            notes.append([0, 50]); continue
        mean = sum(chunk) / win
        cross = 0; prev = chunk[0] - mean
        for v in chunk[1:]:
            cur = v - mean
            if (prev < 0) != (cur < 0): cross += 1
            prev = cur
        freq = cross / 2.0 / (win / rate)
        if freq < 40 or freq > 4000:
            notes.append([0, 50])
        else:
            notes.append([int(freq), 50])
    merged = []
    for f, d in notes:
        if merged and merged[-1][0] == f and f != 0:
            merged[-1][1] += d
        else:
            merged.append([f, d])
    return merged

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("input"); ap.add_argument("output")
    ap.add_argument("--fps", type=int, default=30)
    ap.add_argument("--no-audio", action="store_true")
    a = ap.parse_args()

    vraw = a.output + ".vraw"; awav = a.output + ".wav"
    run(["ffmpeg", "-y", "-i", a.input, "-vf", f"scale=160:120,fps={a.fps}",
         "-pix_fmt", "rgb565be", "-f", "rawvideo", vraw])
    notes = []
    if not a.no_audio:
        run(["ffmpeg", "-y", "-i", a.input, "-ac", "1", "-ar", "8000", awav])
        notes = extract_notes(awav)

    data = open(vraw, "rb").read()
    frames = len(data) // (160 * 120 * 2)
    audio_off = 24 + frames * 160 * 120 * 2
    hdr = struct.pack("<4sHHHHIII", b"PVX1", 160, 120, a.fps,
                      1 if notes else 0, frames, audio_off, len(notes))
    with open(a.output, "wb") as out:
        out.write(hdr)
        out.write(data[:frames * 160 * 120 * 2])
        for f, d in notes:
            out.write(struct.pack("<HH", f & 0xFFFF, d & 0xFFFF))
    print(f"OK: frames={frames} fps={a.fps} notes={len(notes)} size={os.path.getsize(a.output)}")
    for f in (vraw, awav):
        if os.path.exists(f): os.remove(f)

if __name__ == "__main__":
    main()
