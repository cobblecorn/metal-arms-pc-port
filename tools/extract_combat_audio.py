#!/usr/bin/env python3
"""Export weapon and enemy damage SFX from the retail GameCube audio banks.

The export is organized as WAV files plus a CSV manifest, preserving source
bank/effect names for identification. It only reads the extracted game data.
"""
from __future__ import annotations

import argparse
import csv
import json
import pathlib
import re
import struct
import sys

sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))
import extract_vendor_audio as audio
import gamedata_dump


DAMAGE_WORDS = re.compile(r"damage|hitdam|ouch|owie|scream|pain|hurt|death|dead|attimp|crum|blow|roar|burn", re.I)
DAMAGE_TABLES = {"snd_bot.csv", "snd_corr.csv", "snd_zboss.csv"}
WEAPON_TABLES = {"snd_wpn.csv"}


def safe_name(value: str) -> str:
    return re.sub(r"[^A-Za-z0-9_-]+", "_", value).strip("_") or "unnamed"


def build_catalog(data_dir: pathlib.Path) -> tuple[dict[str, list[dict]], dict[str, dict]]:
    """Build case-insensitive effect and wave lookup tables for all SFB banks."""
    by_name: dict[str, list[dict]] = {}
    banks: dict[str, dict] = {}
    for sfb_path in sorted(data_dir.glob("*.sfb")):
        raw_sfb = sfb_path.read_bytes()
        bank_name = audio.cstr(raw_sfb, audio.be32(raw_sfb, 0))
        rdg_path = next((p for p in data_dir.glob("*.rdg") if p.stem.casefold() == bank_name.casefold()), None)
        if rdg_path is None:
            continue
        wave_ids, _ = audio.parse_bank(rdg_path.read_bytes())
        records = audio.parse_sfb(raw_sfb)
        banks[sfb_path.name.casefold()] = {"bank": bank_name, "rdg": rdg_path, "wave_ids": wave_ids, "records": records}
        for record in records:
            enriched = {**record, "sfb": sfb_path.name, "rdg": rdg_path.name, "bank": bank_name}
            by_name.setdefault(record["fx_name"].casefold(), []).append(enriched)
            by_name.setdefault(record["wave"].casefold(), []).append(enriched)
    return by_name, banks


def collect(data_dir: pathlib.Path, by_name: dict[str, list[dict]], banks: dict[str, dict]) -> dict[tuple[str, str, str], set[str]]:
    """Return (category, bank, effect) -> source table descriptions."""
    wanted: dict[tuple[str, str, str], set[str]] = {}

    def add(category: str, token: str, source: str) -> None:
        for record in by_name.get(token.casefold(), []):
            key = (category, record["sfb"], record["fx_name"])
            wanted.setdefault(key, set()).add(source)

    for table_path in sorted(data_dir.glob("snd_*.csv")):
        name = table_path.name.casefold()
        if name not in WEAPON_TABLES | DAMAGE_TABLES:
            continue
        tables = gamedata_dump.decode(table_path.read_bytes())["tables"]
        for table in tables:
            event = str(table["name"])
            if name in WEAPON_TABLES:
                category = "weapon_sfx"
            else:
                if not DAMAGE_WORDS.search(event):
                    continue
                category = "enemy_damage_vocals"
            for field in table["fields"]:
                value = field["value"]
                if isinstance(value, str):
                    add(category, value, f"{table_path.name}:{event}")

    # Some voiced flinches and enemy hit/death calls are banked but absent from
    # the shared sound tables (for example, generic Grunt and Zombie Ouch lines).
    relevant_banks = ("damage", "grunt", "zombie", "corrosive", "elite", "bots_")
    for bank_key, info in banks.items():
        if not any(tag in bank_key for tag in relevant_banks):
            continue
        for record in info["records"]:
            if DAMAGE_WORDS.search(record["fx_name"]):
                key = ("enemy_damage_vocals", bank_key, record["fx_name"])
                wanted.setdefault(key, set()).add("bank name match")
    return wanted


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--data", type=pathlib.Path, default=pathlib.Path("gamedata/mst"), help="directory containing extracted retail data")
    parser.add_argument("--output", type=pathlib.Path, default=pathlib.Path("build/export/combat_audio"), help="output directory")
    args = parser.parse_args()
    try:
        sfx_to_sample, samples = audio.parse_global_audio((args.data / "snd_init.rdg").read_bytes())
        sample_file = (args.data / "snd_smpls.rdg").read_bytes()
        by_name, banks = build_catalog(args.data)
        wanted = collect(args.data, by_name, banks)

        rows: list[dict] = []
        missing: list[dict] = []
        for (category, sfb_name, fx_name), sources in sorted(wanted.items()):
            info = banks[sfb_name.casefold()]
            records = [r for r in info["records"] if r["fx_name"].casefold() == fx_name.casefold()]
            for record in records:
                wave_name = record["wave"]
                wave_id = info["wave_ids"].get(wave_name.casefold())
                sample_id = sfx_to_sample.get(wave_id) if wave_id is not None else None
                sample = samples.get(sample_id) if sample_id is not None else None
                if sample is None or sample["offset"] + sample["frame_bytes"] > len(sample_file):
                    missing.append({"category": category, "bank": info["bank"], "effect": fx_name, "wave": wave_name, "reason": "sample mapping/data not found"})
                    continue
                # Keep each bank/effect alias separately named for the manifest.
                clip_base = safe_name(fx_name)
                bank_tag = safe_name(info["bank"])
                path = f"{category}/{clip_base}__{bank_tag}.wav"
                pcm = audio.decode_dsp(sample_file[sample["offset"]:sample["offset"] + sample["frame_bytes"]], sample["samples"], sample["coefs"])
                if not (args.output / path).is_file():
                    audio.write_wav(args.output / path, pcm, sample["rate"])
                rows.append({
                    "category": category, "filename": path, "effect": fx_name,
                    "wave": wave_name, "bank": info["bank"], "sfb": sfb_name,
                    "sample_id": sample_id, "sample_rate_hz": sample["rate"],
                    "duration_seconds": f"{len(pcm) / sample['rate']:.3f}",
                    "referenced_by": ";".join(sorted(sources)),
                })

        args.output.mkdir(parents=True, exist_ok=True)
        fields = ["category", "filename", "effect", "wave", "bank", "sfb", "sample_id", "sample_rate_hz", "duration_seconds", "referenced_by"]
        with (args.output / "manifest.csv").open("w", newline="", encoding="utf-8-sig") as f:
            writer = csv.DictWriter(f, fieldnames=fields)
            writer.writeheader()
            writer.writerows(rows)
        report = {"output": str(args.output), "clips": len(rows), "weapon_effects": sum(r["category"] == "weapon_sfx" for r in rows),
                  "enemy_damage_vocals": sum(r["category"] == "enemy_damage_vocals" for r in rows), "missing": missing}
        (args.output / "report.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        readme = """# Combat sound effects\n\nExtracted from the locally supplied Metal Arms GameCube data and decoded to mono 16-bit PCM WAV.\n\n- `weapon_sfx/`: firing, reload, impact, ricochet, and weapon-related effects.\n- `enemy_damage_vocals/`: enemy hit, pain, ouch, death, and related damage reactions.\n- `manifest.csv`: source bank/effect names, sample rate, clip duration, and source table.\n- `report.json`: extraction counts and any unresolved sample mappings.\n\nThese are original game assets for use with the user's local project.\n"""
        (args.output / "README.md").write_text(readme, encoding="utf-8")
        print(json.dumps(report, indent=2))
        return 0 if rows else 1
    except (OSError, ValueError, struct.error) as e:
        parser.exit(1, f"extract_combat_audio: {e}\n")


if __name__ == "__main__":
    raise SystemExit(main())
