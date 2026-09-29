#!/usr/bin/env python3
"""Extract the Slim/Shady barter voice clips from the retail GameCube audio data.

The barter BotTalk tables point at named SFX in barter.sfb. This resolves those
names through barter.rdg and the MusyX sample directory, then writes PCM WAVs
plus a CSV manifest. It only reads game data and writes to the selected output.
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import pathlib
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import gamedata_dump


def be16(b: bytes, o: int) -> int:
    return struct.unpack_from(">H", b, o)[0]


def be32(b: bytes, o: int) -> int:
    return struct.unpack_from(">I", b, o)[0]


def cstr(b: bytes, o: int, limit: int | None = None) -> str:
    end = b.find(b"\0", o, len(b) if limit is None else min(len(b), o + limit))
    if end < 0:
        raise ValueError(f"unterminated string at 0x{o:x}")
    return b[o:end].decode("ascii", errors="replace")


def parse_bank(data: bytes) -> tuple[dict[str, int], list[dict]]:
    """Return wave-name => MusyX SFX id, and all named bank effects."""
    if len(data) < 32:
        raise ValueError("barter.rdg is too short")
    count = be32(data, 12)
    if not 0 < count <= 4096:
        raise ValueError(f"invalid wave count {count}")
    waves_off = 32
    ids_off = waves_off + count * 40 + 4  # skip the MusyX group ID
    if ids_off + count * 4 > len(data):
        raise ValueError("barter.rdg wave table is truncated")
    names: dict[str, int] = {}
    rows = []
    for i in range(count):
        o = waves_off + i * 40
        name = cstr(data, o, 12).lower()
        sfx_id = be32(data, ids_off + i * 4)
        names[name] = sfx_id
        rows.append({"wave": name, "sfx_id": sfx_id})
    return names, rows


def parse_sfb(data: bytes) -> list[dict]:
    """Read the file-relative retail SFX bank structs (20/16/8/20 bytes)."""
    if len(data) < 20:
        raise ValueError("barter.sfb is too short")
    bank_name_off, _bank_handle, count, seqs_off, _key = struct.unpack_from(">5I", data, 0)
    if not 0 < count <= 4096 or seqs_off + count * 16 > len(data):
        raise ValueError("invalid barter.sfb header")
    _ = cstr(data, bank_name_off)
    out = []
    for i in range(count):
        o = seqs_off + i * 16
        name_off, ncmd, cmds_off, _self = struct.unpack_from(">4I", data, o)
        name = cstr(data, name_off)
        if ncmd > 64 or cmds_off + ncmd * 8 > len(data):
            raise ValueError(f"invalid command list for {name}")
        for j in range(ncmd):
            cmd_type, cmd_off = struct.unpack_from(">2I", data, cmds_off + j * 8)
            if cmd_type != 0:
                continue
            wav_off = be32(data, cmd_off)
            wav_name = cstr(data, wav_off).lower()
            out.append({"fx_name": name, "wave": wav_name})
    return out


def parse_global_audio(init: bytes) -> tuple[dict[int, int], dict[int, dict]]:
    """Map MusyX SFX IDs to sample IDs, and sample IDs to ADPCM metadata."""
    if len(init) < 24:
        raise ValueError("snd_init.rdg is too short")
    proj_len, proj_off, pool_len, pool_off, sdir_len, sdir_off = struct.unpack_from(">6I", init, 0)
    for off, size, label in ((proj_off, proj_len, "project"), (pool_off, pool_len, "pool"), (sdir_off, sdir_len, "sample directory")):
        if off + size > len(init):
            raise ValueError(f"snd_init.rdg {label} is truncated")

    pproj = init[proj_off:proj_off + proj_len]
    ppool = init[pool_off:pool_off + pool_len]
    psdir = init[sdir_off:sdir_off + sdir_len]
    # The project contains variable-size sound-effect groups; each group has a
    # file-relative ID->macro table at +28. Group type 1 is an SFX group.
    sfx_to_macro: dict[int, int] = {}
    pos = 0
    while pos + 4 <= len(pproj):
        end = be32(pproj, pos)
        if end == 0xFFFFFFFF:
            break
        if pos + 40 > len(pproj) or end <= pos or end > len(pproj):
            raise ValueError("invalid sound-project group extent")
        if be16(pproj, pos + 6) == 1:
            table = be32(pproj, pos + 28)
            if table + 4 > len(pproj):
                raise ValueError("invalid sound-project ID table")
            n = be16(pproj, table)
            if table + 4 + n * 10 > len(pproj):
                raise ValueError("truncated sound-project ID table")
            for i in range(n):
                e = table + 4 + i * 10
                sfx_to_macro[be16(pproj, e)] = be16(pproj, e + 2)
        pos = end
    else:
        raise ValueError("sound-project end marker was not found")

    macro_to_sample: dict[int, int] = {}
    pos = be32(ppool, 0)
    while pos + 4 <= len(ppool):
        size = be32(ppool, pos)
        if size == 0xFFFFFFFF:
            break
        if size < 8 or pos + size > len(ppool):
            raise ValueError("invalid SoundMacro extent")
        macro = be16(ppool, pos + 4)
        for cmd in range(pos + 8, pos + size - 7, 8):
            word = be32(ppool, cmd)
            if word & 0xFF == 0x10:  # StartSample
                macro_to_sample[macro] = (word >> 8) & 0xFFFF
                break
        pos += size

    sfx_to_sample = {sfx: macro_to_sample[macro] for sfx, macro in sfx_to_macro.items() if macro in macro_to_sample}
    sdir_end = len(psdir)
    samples: dict[int, dict] = {}
    pos = 0
    while pos + 32 <= sdir_end and be16(psdir, pos) != 0xFFFF:
        sample_id = be16(psdir, pos)
        data_off = be32(psdir, pos + 4)
        raw_len = be32(psdir, pos + 16)
        n_samples, fmt = raw_len & 0xFFFFFF, raw_len >> 24
        rate = be16(psdir, pos + 14)
        info_off = be32(psdir, pos + 28)
        frame_bytes = ((n_samples + 13) // 14) * 8
        if fmt == 0 and n_samples and 4000 <= rate <= 48000 and info_off + 40 <= sdir_end:
            coefs = list(struct.unpack_from(">16h", psdir, info_off + 8))
            samples[sample_id] = {"offset": data_off, "samples": n_samples, "rate": rate, "coefs": coefs, "frame_bytes": frame_bytes}
        pos += 32
    return sfx_to_sample, samples


def decode_dsp(data: bytes, n_samples: int, coefs: list[int]) -> list[int]:
    hist1 = hist2 = 0
    out: list[int] = []
    for pos in range(0, len(data), 8):
        frame = data[pos:pos + 8]
        if len(frame) < 8 or len(out) >= n_samples:
            break
        header = frame[0]
        scale = 1 << (header & 15)
        predictor = (header >> 4) & 7
        c1, c2 = coefs[predictor * 2:predictor * 2 + 2]
        for i in range(14):
            if len(out) >= n_samples:
                break
            nibble = frame[1 + i // 2] >> 4 if i % 2 == 0 else frame[1 + i // 2] & 15
            if nibble >= 8:
                nibble -= 16
            sample = ((nibble * scale << 11) + 1024 + c1 * hist1 + c2 * hist2) >> 11
            sample = max(-32768, min(32767, sample))
            out.append(sample)
            hist2, hist1 = hist1, sample
    return out


def write_wav(path: pathlib.Path, pcm: list[int], rate: int) -> None:
    raw = struct.pack("<" + "h" * len(pcm), *pcm)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("wb") as f:
        f.write(b"RIFF" + struct.pack("<I", 36 + len(raw)) + b"WAVEfmt ")
        f.write(struct.pack("<IHHIIHH", 16, 1, 1, rate, rate * 2, 2, 16))
        f.write(b"data" + struct.pack("<I", len(raw)) + raw)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data", type=pathlib.Path, default=pathlib.Path("gamedata/mst"), help="directory containing extracted retail data")
    parser.add_argument("--output", type=pathlib.Path, default=pathlib.Path("build/export/slim_shady_audio"), help="output directory")
    args = parser.parse_args()
    try:
        data_dir = args.data
        sfx_to_sample, samples = parse_global_audio((data_dir / "snd_init.rdg").read_bytes())
        sample_file = (data_dir / "snd_smpls.rdg").read_bytes()

        refs: dict[str, set[str]] = {}
        for table_path in sorted(data_dir.glob("brbs_*.csv")):
            tables = gamedata_dump.decode(table_path.read_bytes())["tables"]
            action = next((t for t in tables if t["name"].casefold() == "actionlist"), None)
            if not action:
                continue
            fields = [f["value"] for f in action["fields"]]
            count = int(fields[0])
            if len(fields) != 1 + count * 6:
                raise ValueError(f"malformed ActionList in {table_path.name}")
            for i in range(count):
                row = fields[1 + 6 * i: 1 + 6 * (i + 1)]
                if row[2] == 3 and isinstance(row[3], str):
                    refs.setdefault(row[3].casefold(), set()).add(table_path.stem.upper())

        # Speech effects used by these tables can live in more than one sound
        # bank. Resolve each SFB through its declared WVB bank name.
        fx_by_name: dict[str, tuple[str, pathlib.Path]] = {}
        for sfb_path in sorted(data_dir.glob("*.sfb")):
            raw_sfb = sfb_path.read_bytes()
            bank_name = cstr(raw_sfb, be32(raw_sfb, 0))
            rdg_path = data_dir / (bank_name + ".rdg")
            if not rdg_path.is_file():
                continue
            wave_ids, _ = parse_bank(rdg_path.read_bytes())
            for record in parse_sfb(raw_sfb):
                fx_by_name.setdefault(record["fx_name"].casefold(), (record["wave"], rdg_path))
        rows = []
        missing = []
        written = set()
        for fx_name in sorted(refs):
            fx_asset = fx_by_name.get(fx_name)
            if fx_asset is None:
                missing.append((fx_name, "SFB or wave bank name not found"))
                continue
            wave_name, rdg_path = fx_asset
            wave_ids, _ = parse_bank(rdg_path.read_bytes())
            if wave_name not in wave_ids:
                missing.append((fx_name, f"wave {wave_name} not found in {rdg_path.name}"))
                continue
            sample_id = sfx_to_sample.get(wave_ids[wave_name])
            sample = samples.get(sample_id) if sample_id is not None else None
            if sample is None or sample["offset"] + sample["frame_bytes"] > len(sample_file):
                missing.append((fx_name, "MusyX sample mapping/data not found"))
                continue
            pcm = decode_dsp(sample_file[sample["offset"]:sample["offset"] + sample["frame_bytes"]], sample["samples"], sample["coefs"])
            safe = "".join(ch if ch.isalnum() or ch in "-_" else "_" for ch in fx_name)
            speaker = "Slim" if "slim" in fx_name else "Shady"
            out_name = f"{speaker}/{safe}.wav"
            if out_name not in written:
                write_wav(args.output / out_name, pcm, sample["rate"])
                written.add(out_name)
            rows.append({"filename": out_name, "fx_name": fx_name, "wave_name": wave_name,
                         "sample_id": sample_id, "sample_rate_hz": sample["rate"],
                         "duration_seconds": f"{len(pcm) / sample['rate']:.3f}",
                         "speaker": speaker,
                         "bot_talk_tables": ";".join(sorted(refs[fx_name]))})
        args.output.mkdir(parents=True, exist_ok=True)
        with (args.output / "manifest.csv").open("w", newline="", encoding="utf-8-sig") as f:
            writer = csv.DictWriter(f, fieldnames=["filename", "fx_name", "wave_name", "sample_id", "sample_rate_hz", "duration_seconds", "speaker", "bot_talk_tables"])
            writer.writeheader()
            writer.writerows(rows)
        report = {"source": str(data_dir), "output": str(args.output), "unique_clips": len(rows), "missing": missing,
                  "bot_talk_tables_scanned": len(list(data_dir.glob("brbs_*.csv")))}
        (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0 if rows else 1
    except (OSError, ValueError, StopIteration, struct.error) as e:
        parser.exit(1, f"extract_vendor_audio: {e}\n")


if __name__ == "__main__":
    raise SystemExit(main())
