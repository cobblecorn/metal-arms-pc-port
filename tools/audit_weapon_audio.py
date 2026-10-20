"""Measure clipping risk from retail AA/Predator samples and authored firing rates.

Writes numerical evidence to ignored build/. This is a deterministic overlap
simulation, not a recording of DirectSound, the console mixer or heard gameplay.
No audio is played and no game assets/configuration are modified.
"""
from pathlib import Path
import json
import math
import random
import re
import struct

from extract_vendor_audio import decode_dsp, parse_bank, parse_global_audio, parse_sfb

ROOT = Path(__file__).resolve().parent.parent


def main():
    data = ROOT / "gamedata/mst"
    mapping, samples = parse_global_audio((data / "snd_init.rdg").read_bytes())
    raw = (data / "snd_smpls.rdg").read_bytes()
    source = (ROOT / "ma/Lib/Fang2/dx/fdx8audio.cpp").read_text()
    trim_db = float(re.search(r"#define\s+_PORT_SFX_DEFAULT_DB\s+\(\s*([-\d.]+)f", source)[1])
    targets = {}
    for path in data.glob("*.sfb"):
        bank_data = path.read_bytes()
        name_offset = struct.unpack_from(">I", bank_data)[0]
        name = bank_data[name_offset:bank_data.find(b"\0", name_offset)].decode()
        bank_path = data / (name + ".rdg")
        if not bank_path.exists():
            continue
        bank, _ = parse_bank(bank_path.read_bytes())
        for effect in parse_sfb(bank_data):
            if effect["fx_name"].lower() in ("swma_fire", "fire laser euk1", "spew fire"):
                targets[effect["fx_name"]] = (bank[effect["wave"]], effect["wave"])
    rows = []
    for name, (effect_id, wave) in targets.items():
        sample = samples[mapping[effect_id]]
        pcm = decode_dsp(raw[sample["offset"]:sample["offset"]+sample["frame_bytes"]],
                         sample["samples"], sample["coefs"])
        rate = sample["rate"]
        mix = [0.] * (rate*6 + len(pcm)*2)
        rng = random.Random(123)
        aa = name.lower() == "swma_fire"
        laser = name.lower() == "fire laser euk1"
        cadence = 16 if aa else 15
        for shot in range(cadence*5):
            pitch = rng.uniform(.92, 1.08) if aa else rng.uniform(1.2, 1.25) if laser else 1
            start = round(shot*rate/cadence)
            for j in range(int(len(pcm)/pitch)):
                index = int(j*pitch)
                fraction = j*pitch-index
                value = pcm[index] if index+1 >= len(pcm) else pcm[index]*(1-fraction)+pcm[index+1]*fraction
                mix[start+j] += value/32768
        volume = .8 if aa else .9
        gain = (.38*(volume**.25 + volume**.5))**2
        row = {"effect": name, "wave": wave, "rate": rate, "samples": len(pcm),
               "duration": len(pcm)/rate, "voices_per_second": cadence,
               "sample_clip_fraction": sum(abs(x) >= 32767 for x in pcm)/len(pcm)}
        for db in (0, trim_db):
            amplitude = gain*10**(db/20)
            window = mix[rate:rate*5]
            row[f"{db:g} dB"] = {
                "peak": max(abs(x*amplitude) for x in window),
                "clip_fraction": sum(abs(x*amplitude) > 1 for x in window)/len(window),
                "rms": math.sqrt(sum((x*amplitude)**2 for x in window)/len(window)),
            }
        rows.append(row)
    output = ROOT / "build/reddit-audit-20261018/weapon-audio-mix.json"
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps({"method": __doc__, "seed": 123, "results": rows}, indent=2)+"\n")
    for row in rows:
        original, current = row["0 dB"], row[f"{trim_db:g} dB"]
        print(f"{row['effect']}: {row['rate']} Hz; modeled clipped samples "
              f"{original['clip_fraction']:.3%} -> {current['clip_fraction']:.3%} at {trim_db:g} dB")
    print("Numerical simulation only; no audio played.")


if __name__ == "__main__":
    main()
