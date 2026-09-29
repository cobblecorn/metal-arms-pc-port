#!/usr/bin/env python3
"""Export the game's rigged character meshes, textures, and known .mtx clips to Blender."""

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys

from character_anim import AnimationFormatError, read_animation


ROOT = Path(__file__).resolve().parents[1]
CATALOG_PATH = Path(__file__).resolve().with_name("character_models.json")


def find_blender(explicit: Path | None) -> Path | None:
    if explicit:
        return explicit.resolve()
    on_path = shutil.which("blender")
    if on_path:
        return Path(on_path).resolve()
    for version in ("5.2", "5.1", "5.0", "4.5", "4.4", "4.3", "4.2", "4.1", "4.0", "3.6"):
        candidate = Path(rf"C:\Program Files\Blender Foundation\Blender {version}\blender.exe")
        if candidate.is_file():
            return candidate
    return None


def collect_animation_files(models: list[dict], source_dir: Path, output_dir: Path) -> tuple[dict, list[dict]]:
    output_dir.mkdir(parents=True, exist_ok=True)
    known_families = {family.casefold() for model in models
                      for family in model.get("animation_families", [])}
    candidates = sorted((path for path in source_dir.glob("*.mtx")
                         if any(path.stem.casefold().startswith(family) for family in known_families)),
                        key=lambda path: path.name.casefold())
    assignments: dict[str, list[str]] = {model["resource"]: [] for model in models}
    reports: list[dict] = []

    for path in candidates:
        family = next((item for item in sorted(known_families, key=len, reverse=True)
                       if path.stem.casefold().startswith(item)), None)
        if family is None:
            continue
        try:
            animation = read_animation(path)
        except (OSError, AnimationFormatError) as error:
            reports.append({"animation": path.name, "family": family,
                            "status": "decode_failed", "error": str(error)})
            continue

        animation_bones = {track["bone"].casefold() for track in animation.get("tracks", [])}
        matched_models = []
        for model in models:
            if family not in {item.casefold() for item in model.get("animation_families", [])}:
                continue
            model_json = output_dir.parent / f"{model['resource']}.json"
            # The runtime mesh export writes model JSON into the same raw directory.
            if not model_json.is_file():
                continue
            with model_json.open("r", encoding="utf-8") as stream:
                model_data = json.load(stream)
            model_bones = {bone.get("name", "").casefold() for bone in model_data.get("bones", [])}
            overlap = animation_bones & model_bones
            ratio = len(overlap) / max(1, len(animation_bones))
            explicit_clip = animation["name"].casefold() in {
                name.casefold() for name in model.get("animation_allowlist", [])
            }
            if overlap and (ratio >= 0.80 or explicit_clip):
                assignments[model["resource"]].append(animation["name"])
                matched_models.append({"resource": model["resource"],
                                       "overlap_bones": len(overlap),
                                       "animation_bones": len(animation_bones),
                                       "coverage": round(ratio, 4)})

        animation_path = output_dir / f"{animation['name']}.json"
        with animation_path.open("w", encoding="utf-8", newline="\n") as stream:
            json.dump(animation, stream, ensure_ascii=False, indent=2, allow_nan=False)
            stream.write("\n")
        reports.append({"animation": animation["name"], "family": family,
                        "bone_count": animation["bone_count"],
                        "duration_seconds": animation["duration_seconds"],
                        "matches": matched_models,
                        "status": "assigned" if matched_models else "no_mesh_bone_match"})

    for resource in assignments:
        assignments[resource] = sorted(set(assignments[resource]), key=str.casefold)
    return assignments, reports


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", choices=("Debug", "Release"), default="Release")
    parser.add_argument("--exe", type=Path, help="optional path to ma_port.exe")
    parser.add_argument("--blender", type=Path, help="optional Blender executable path")
    parser.add_argument("--output", type=Path,
                        help="new export directory (default: build/export/characters_<timestamp>)")
    parser.add_argument("--resources", nargs="+", metavar="RESOURCE",
                        help="export only these catalog resource names (useful for a focused porting set)")
    parser.add_argument("--timeout", type=int, default=600,
                        help="ma_port export timeout in seconds (default: 600)")
    parser.add_argument("--skip-game-export", action="store_true",
                        help="use existing raw model JSON and textures in the output directory")
    parser.add_argument("--skip-blender", action="store_true",
                        help="export raw model, texture, and animation resources without creating .blend")
    args = parser.parse_args()
    if args.timeout < 1:
        parser.error("--timeout must be positive")

    output = args.output
    if output is None:
        stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        output = ROOT / "build" / "export" / f"characters_{stamp}"
    elif not output.is_absolute():
        output = ROOT / output
    output = output.resolve()
    if output.exists() and any(output.iterdir()) and not args.skip_game_export:
        print(f"ERROR: output directory is not empty: {output}", file=sys.stderr)
        return 2
    output.mkdir(parents=True, exist_ok=True)

    with CATALOG_PATH.open("r", encoding="utf-8") as stream:
        catalog = json.load(stream)
    models = catalog.get("models", [])
    if args.resources:
        model_by_name = {model["resource"].casefold(): model for model in models}
        unknown = [resource for resource in args.resources if resource.casefold() not in model_by_name]
        if unknown:
            parser.error("unknown character resources: " + ", ".join(unknown))
        selected = {resource.casefold() for resource in args.resources}
        models = [model for model in models if model["resource"].casefold() in selected]
    if not models:
        print("ERROR: character model catalog is empty.", file=sys.stderr)
        return 2

    data_dir = ROOT / "gamedata" / "files"
    source_dir = ROOT / "gamedata" / "mst"
    missing = [model["resource"] for model in models
               if not (source_dir / f"{model['resource']}.ape").is_file()]
    if missing:
        print("ERROR: missing source meshes: " + ", ".join(missing), file=sys.stderr)
        return 2
    if not (data_dir / "mettlearms_gc.mst").is_file():
        print(f"ERROR: retail master file not found under {data_dir}", file=sys.stderr)
        return 2

    raw_dir = output / "raw"
    texture_dir = raw_dir / "textures"
    animation_dir = raw_dir / "animations"
    raw_dir.mkdir(parents=True, exist_ok=True)
    texture_dir.mkdir(parents=True, exist_ok=True)
    animation_dir.mkdir(parents=True, exist_ok=True)
    list_path = output / "model_list.txt"
    list_path.write_text("".join(f"{model['resource']}\n" for model in models), encoding="ascii")

    if not args.skip_game_export:
        exe = (args.exe or ROOT / "build" / args.config / "ma_port.exe").resolve()
        if not exe.is_file():
            print(f"ERROR: executable not found: {exe}", file=sys.stderr)
            return 2
        environment = os.environ.copy()
        environment["MA_CHARACTER_EXPORT_DIR"] = str(raw_dir)
        environment["MA_CHARACTER_TEXTURE_DIR"] = str(source_dir)
        command = [
            str(exe),
            "-data", str(data_dir.resolve()),
            "-export-character-meshes", str(list_path.resolve()),
            "-no-audio",
            "-discord-app-id", "off",
            "-instance-label", "Character Export",
            "-log", str((output / "character_export.log").resolve()),
            "-asset-log", str((output / "character_assets.log").resolve()),
        ]
        print(f"Loading {len(models)} rigged character resources through ma_port's mesh converter.")
        print("This extraction-only process has audio disabled and exits after the named resources load.")
        try:
            completed = subprocess.run(command, cwd=ROOT, env=environment,
                                       timeout=args.timeout, check=False)
        except subprocess.TimeoutExpired:
            print(f"ERROR: character mesh loading exceeded {args.timeout} seconds; "
                  "the export process was stopped.", file=sys.stderr)
            return 3
        if completed.returncode != 0:
            print(f"ERROR: ma_port exited with code {completed.returncode}; "
                  f"see {output / 'character_export.log'}", file=sys.stderr)
            return completed.returncode or 1

    missing_exports = [model["resource"] for model in models
                       if not (raw_dir / f"{model['resource']}.json").is_file()]
    if missing_exports:
        print("ERROR: no rigged mesh export for: " + ", ".join(missing_exports), file=sys.stderr)
        print(f"Review {output / 'character_export.log'} and {output / 'character_assets.log'}.",
              file=sys.stderr)
        return 4

    assignments, animation_reports = collect_animation_files(models, source_dir, animation_dir)
    import_manifest = {
        "schema_version": 1,
        "catalog": str(CATALOG_PATH.relative_to(ROOT)).replace("\\", "/"),
        "models": models,
        "animations_by_model": assignments,
        "animation_reports": animation_reports,
    }
    manifest_path = output / "import_manifest.json"
    manifest_path.write_text(json.dumps(import_manifest, indent=2) + "\n", encoding="utf-8")
    (output / "character_export_manifest.json").write_text(
        json.dumps({"catalog": catalog, "animations": animation_reports}, indent=2) + "\n",
        encoding="utf-8")

    if args.skip_blender:
        print(f"Exported {len(models)} rigged models, {len(animation_reports)} animation resources, "
              f"and {len(list(texture_dir.glob('*.tga')))} textures to {output}")
        return 0

    blender = find_blender(args.blender)
    if not blender or not blender.is_file():
        print("ERROR: Blender executable not found; pass --blender <path>.", file=sys.stderr)
        return 2
    blend_path = output / "metal_arms_characters.blend"
    importer = Path(__file__).resolve().with_name("import_character_models.py")
    command = [str(blender), "--background", "--python", str(importer), "--",
               str(raw_dir), str(manifest_path), str(blend_path)]
    try:
        completed = subprocess.run(command, cwd=ROOT, timeout=1200, check=False)
    except subprocess.TimeoutExpired:
        print("ERROR: Blender scene assembly exceeded 20 minutes.", file=sys.stderr)
        return 3
    if completed.returncode != 0 or not blend_path.is_file():
        print(f"ERROR: Blender scene assembly failed; see {blend_path.parent}.", file=sys.stderr)
        return completed.returncode or 4

    summary_path = output / "metal_arms_characters_summary.json"
    summary = json.loads(summary_path.read_text(encoding="utf-8")) if summary_path.is_file() else []
    total_triangles = sum(item.get("triangles", 0) for item in summary)
    assigned_actions = sum(len(item.get("animations", [])) for item in summary)
    print(f"Created {blend_path}")
    print(f"Included {len(summary)} models, {total_triangles} triangles, "
          f"{len(list(texture_dir.glob('*.tga')))} textures, and {assigned_actions} model/clip actions.")
    print(f"Raw source exports and animation JSON are in {raw_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
