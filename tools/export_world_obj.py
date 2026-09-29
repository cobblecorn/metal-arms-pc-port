#!/usr/bin/env python3
"""Export embedded static world meshes from a retail level to OBJ for Blender/Roblox."""

from __future__ import annotations

import argparse
from datetime import datetime
import json
import os
import re
import subprocess
import sys
from pathlib import Path


WORLD_RE = re.compile(r"^[A-Za-z0-9_-]+$")
RESOURCE_RE_TEMPLATE = "{world}[0-9][0-9][0-9].obj"


def parse_obj(path: Path) -> tuple[list[str], dict[str, int]]:
    lines = path.read_text(encoding="utf-8", errors="strict").splitlines()
    counts = {"vertices": 0, "texcoords": 0, "normals": 0, "faces": 0}
    for line_number, line in enumerate(lines, 1):
        parts = line.split()
        if not parts or parts[0].startswith("#"):
            continue
        kind = parts[0]
        if kind == "v":
            counts["vertices"] += 1
        elif kind == "vt":
            counts["texcoords"] += 1
        elif kind == "vn":
            counts["normals"] += 1
        elif kind == "f":
            if len(parts) != 4:
                raise ValueError(f"{path}:{line_number}: expected triangle face")
            for item in parts[1:]:
                indices = item.split("/")
                vi = int(indices[0])
                if vi <= 0 or vi > counts["vertices"]:
                    raise ValueError(f"{path}:{line_number}: vertex index out of bounds")
                if len(indices) > 1 and indices[1]:
                    ti = int(indices[1])
                    if ti <= 0 or ti > counts["texcoords"]:
                        raise ValueError(f"{path}:{line_number}: UV index out of bounds")
                if len(indices) > 2 and indices[2]:
                    ni = int(indices[2])
                    if ni <= 0 or ni > counts["normals"]:
                        raise ValueError(f"{path}:{line_number}: normal index out of bounds")
            counts["faces"] += 1
    return lines, counts


def merge_obj_files(mesh_paths: list[Path], output_path: Path) -> dict[str, int]:
    totals = {"vertices": 0, "texcoords": 0, "normals": 0, "faces": 0}
    with output_path.open("w", encoding="utf-8", newline="\n") as merged:
        merged.write("# Metal Arms static world geometry; merged from per-mesh OBJ chunks.\n")
        merged.write(f"mtllib {output_path.with_suffix('.mtl').name}\n")
        for path in mesh_paths:
            lines, counts = parse_obj(path)
            merged.write(f"\n# Source mesh: {path.stem}\n")
            merged.write(f"o {path.stem}\n")
            vertex_base = totals["vertices"]
            texcoord_base = totals["texcoords"]
            normal_base = totals["normals"]
            for line in lines:
                parts = line.split()
                if not parts or parts[0].startswith("#"):
                    continue
                kind = parts[0]
                if kind == "mtllib":
                    continue
                if kind == "f":
                    adjusted = []
                    for item in parts[1:]:
                        indices = item.split("/")
                        vi = int(indices[0]) + vertex_base
                        if len(indices) == 1:
                            adjusted.append(str(vi))
                        elif len(indices) == 2:
                            ti = int(indices[1]) + texcoord_base if indices[1] else ""
                            adjusted.append(f"{vi}/{ti}")
                        else:
                            ti = int(indices[1]) + texcoord_base if indices[1] else ""
                            ni = int(indices[2]) + normal_base if indices[2] else ""
                            adjusted.append(f"{vi}/{ti}/{ni}")
                    merged.write("f " + " ".join(adjusted) + "\n")
                else:
                    merged.write(line + "\n")
            for key, value in counts.items():
                totals[key] += value
    return totals


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Load a world with ma_port's world-only mode and export its static geometry to OBJ."
    )
    parser.add_argument("world", help="world resource name, for example wedmmines01")
    parser.add_argument("--config", choices=("Debug", "Release"), default="Release")
    parser.add_argument("--exe", type=Path, help="optional path to ma_port.exe")
    parser.add_argument("--output", type=Path, help="new or empty output directory (default: build/export/<world>_<timestamp>)")
    parser.add_argument("--timeout", type=int, default=240, help="world load timeout in seconds (default: 240)")
    args = parser.parse_args()

    if not WORLD_RE.fullmatch(args.world):
        parser.error("world names may contain only letters, digits, underscores, and hyphens")
    if args.timeout < 1:
        parser.error("--timeout must be positive")

    repo = Path(__file__).resolve().parents[1]
    world = args.world.lower()
    exe = (args.exe or repo / "build" / args.config / "ma_port.exe").resolve()
    if not exe.is_file():
        print(f"ERROR: executable not found: {exe}", file=sys.stderr)
        return 2

    if args.output:
        output = args.output if args.output.is_absolute() else repo / args.output
        output = output.resolve()
    else:
        stamp = datetime.now().strftime("%Y%m%d_%H%M%S_%f")
        output = (repo / "build" / "export" / f"{world}_{stamp}").resolve()
    if output.exists() and (not output.is_dir() or any(output.iterdir())):
        print(f"ERROR: output directory already contains files: {output}", file=sys.stderr)
        return 2

    mesh_dir = output / "meshes"
    mesh_dir.mkdir(parents=True, exist_ok=True)
    (output / "textures").mkdir(parents=True, exist_ok=True)
    data_dir = (repo / "gamedata" / "files").resolve()
    if not (data_dir / "mettlearms_gc.mst").is_file():
        print(f"ERROR: retail master file not found under {data_dir}", file=sys.stderr)
        return 2

    log_path = output / "world_export.log"
    asset_log_path = output / "world_export_assets.log"
    command = [
        str(exe),
        "-data", str(data_dir),
        "-world-only", world,
        "-discord-app-id", "off",
        "-instance-label", "World Export",
        "-log", str(log_path),
        "-asset-log", str(asset_log_path),
    ]
    environment = os.environ.copy()
    environment["MA_WORLD_EXPORT"] = world
    environment["MA_WORLD_EXPORT_DIR"] = str(output)
    environment["MA_WORLD_TEXTURE_DIR"] = str((repo / "gamedata" / "mst").resolve())

    print(f"Loading {world} in world-only mode. Audio/gameplay setup is not entered.")
    try:
        completed = subprocess.run(command, cwd=repo, env=environment, timeout=args.timeout, check=False)
    except subprocess.TimeoutExpired:
        print(f"ERROR: world loading exceeded {args.timeout} seconds; process was stopped.", file=sys.stderr)
        return 3

    mesh_paths = sorted(mesh_dir.glob(RESOURCE_RE_TEMPLATE.format(world=world)))
    if completed.returncode != 0:
        print(f"ERROR: ma_port exited with code {completed.returncode}; see {log_path}", file=sys.stderr)
        return completed.returncode or 1
    if not mesh_paths:
        print(f"ERROR: no static world mesh chunks were exported; see {log_path}", file=sys.stderr)
        return 4

    chunk_counts = []
    for path in mesh_paths:
        _, counts = parse_obj(path)
        chunk_counts.append({"file": f"meshes/{path.name}", **counts})
    scene_path = output / f"{world}.obj"
    totals = merge_obj_files(mesh_paths, scene_path)
    _, merged_counts = parse_obj(scene_path)
    if merged_counts != totals:
        print("ERROR: merged OBJ counts do not match the mesh chunks.", file=sys.stderr)
        return 4
    if totals["faces"] == 0:
        print("ERROR: export contained no triangle faces.", file=sys.stderr)
        return 4

    material_library = output / f"{world}.mtl"
    exported_textures = sorted((output / "textures").glob("*.tga")) if (output / "textures").is_dir() else []
    material_count = 0
    if material_library.is_file():
        with material_library.open("r", encoding="utf-8") as material_file:
            material_count = sum(line.startswith("newmtl ") for line in material_file)
    manifest = {
        "format": "metal-arms-static-world-obj",
        "schema_version": 2,
        "world": world,
        "coordinate_system": "native world mesh coordinates; no axis or scale conversion",
        "scene_obj": f"{world}.obj",
        "material_library": material_library.name if material_library.is_file() else None,
        "textures": [f"textures/{path.name}" for path in exported_textures],
        "materials": material_count,
        "mesh_chunks": chunk_counts,
        "totals": totals,
        "contents": {
            "embedded_static_world_geometry": True,
            "placed_world_shape_objects": False,
            "runtime_liquid_surfaces": False,
            "game_texture_images_and_materials": material_library.is_file() and bool(exported_textures),
            "material_layers_exported": "First available surface layer (Layer0 preferred); game shader blending, lightmaps, and effects are not reproduced.",
            "vertex_normals": True,
            "texture_coordinates": True,
        },
        "roblox": {
            "mesh_format": "OBJ",
            "import_scene_obj_for_a_single_combined_mesh": f"{world}.obj",
            "import_mesh_chunks_for_separate_parts": "meshes/",
            "chunk_coordinates": "Each chunk keeps its original level-space vertex coordinates.",
        },
    }
    manifest_path = output / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"Exported {len(mesh_paths)} meshes, {totals['vertices']} vertices, and {totals['faces']} triangles.")
    print(f"Blender OBJ: {output / (world + '.obj')}")
    print(f"Per-mesh chunks and manifest: {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
