#!/usr/bin/env python3
"""Export Shady and Slim as separate static, textured OBJ packages from Blender.

Run with Blender's bundled Python, for example:

    blender --background path/to/metal_arms_vendors.blend --python tools/export_vendor_obj.py -- \
      --output build/export/character_porting_kit_20260928/obj

Each output directory contains one OBJ, its MTL, and textures copied by Blender's
OBJ exporter. OBJ has no armature or animation support; retain the source .blend
for the rigs and animation actions.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import shutil
import sys
from zipfile import ZIP_DEFLATED, ZipFile

import bpy


ROOT = Path(__file__).resolve().parents[1]
DEFAULT_OUTPUT = ROOT / "build" / "export" / "character_porting_kit_20260928" / "obj"
MODELS = {
    "Shady": "grdhshady00_Model",
    "Slim": "grdislim_00_Model",
}


def centered_grounded_obj(path: Path) -> tuple[int, tuple[float, float, float]]:
    """Center exported vertices in X/Y and place the lowest vertex at Z=0."""
    lines = path.read_text(encoding="utf-8").splitlines()
    vertices: list[tuple[int, list[str]]] = []
    for index, line in enumerate(lines):
        parts = line.split()
        if parts and parts[0] == "v":
            if len(parts) < 4:
                raise RuntimeError(f"Invalid vertex line in {path}:{index + 1}")
            vertices.append((index, parts))
    if not vertices:
        raise RuntimeError(f"No vertices were exported to {path}")

    positions = [(float(parts[1]), float(parts[2]), float(parts[3]))
                 for _, parts in vertices]
    minimum = tuple(min(point[axis] for point in positions) for axis in range(3))
    maximum = tuple(max(point[axis] for point in positions) for axis in range(3))
    offset = ((minimum[0] + maximum[0]) / 2.0,
              (minimum[1] + maximum[1]) / 2.0,
              minimum[2])

    for index, parts in vertices:
        for axis in range(3):
            value = float(parts[axis + 1]) - offset[axis]
            parts[axis + 1] = f"{value:.9g}"
        lines[index] = " ".join(parts)
    path.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")

    dimensions = tuple(maximum[axis] - minimum[axis] for axis in range(3))
    return len(vertices), dimensions


def material_texture_paths(mtl_path: Path) -> list[Path]:
    paths: list[Path] = []
    if not mtl_path.is_file():
        return paths
    for line in mtl_path.read_text(encoding="utf-8", errors="replace").splitlines():
        stripped = line.strip()
        if not stripped.startswith(("map_Kd ", "map_kd ")):
            continue
        # The exporter writes simple relative paths for copied images. Preserve
        # spaces by treating everything after the map directive as the path.
        value = stripped.split(None, 1)[1].strip().strip('"')
        candidate = (mtl_path.parent / Path(value.replace("\\", "/"))).resolve()
        paths.append(candidate)
    return paths


def material_images(obj: bpy.types.Object) -> list[bpy.types.Image]:
    images: dict[str, bpy.types.Image] = {}
    for material in obj.data.materials:
        if material is None or material.node_tree is None:
            continue
        for node in material.node_tree.nodes:
            image = getattr(node, "image", None)
            if node.type == "TEX_IMAGE" and image is not None:
                images[image.name] = image
    return list(images.values())


def copy_material_images(images: list[bpy.types.Image], destination: Path) -> None:
    for image in images:
        texture_path = destination / Path(image.name).name
        if image.packed_file is not None:
            texture_path.write_bytes(bytes(image.packed_file.data))
            continue
        source_path = Path(bpy.path.abspath(image.filepath)).resolve()
        if not source_path.is_file():
            raise RuntimeError(f"Texture {image.name!r} is neither packed nor available")
        shutil.copy2(source_path, texture_path)


def write_package_files(label: str, destination: Path) -> tuple[Path, int]:
    readme = destination / "README.txt"
    readme.write_text(
        f"{label} vendor robot OBJ package\n"
        "\n"
        "Import the OBJ with its matching MTL and keep the TGA textures beside them.\n"
        "The mesh is static, in its armature rest pose, Z-up, centered in X/Y, and\n"
        "grounded at Z=0. OBJ does not contain the skeleton or animation actions.\n"
        "For the rig and source-matched animations, use metal_arms_vendors.blend.\n",
        encoding="utf-8",
        newline="\n",
    )
    archive_path = destination.parent / f"{label}_OBJ.zip"
    texture_paths = material_texture_paths(destination / f"{label}.mtl")
    package_files = [destination / f"{label}.obj", destination / f"{label}.mtl", readme,
                     *texture_paths]
    with ZipFile(archive_path, "w", compression=ZIP_DEFLATED) as archive:
        for file_path in sorted(set(package_files), key=lambda path: path.name.casefold()):
            if not file_path.is_file():
                raise RuntimeError(f"Missing package file for {label}: {file_path}")
            archive.write(file_path, arcname=f"{label}/{file_path.name}")
    texture_count = len({path.name.casefold() for path in texture_paths})
    return archive_path, texture_count


def export_model(label: str, object_name: str, output_root: Path) -> dict[str, object]:
    obj = bpy.data.objects.get(object_name)
    if obj is None or obj.type != "MESH":
        raise RuntimeError(f"Expected mesh object {object_name!r} was not found in the blend file")

    destination = output_root / label
    destination.mkdir(parents=True, exist_ok=True)
    obj_path = destination / f"{label}.obj"

    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj

    # Export the unanimated mesh in the armature's rest pose. Textures are
    # staged from the packed blend data because this blend may have moved since
    # it was assembled; the character project itself remains untouched.
    rest_positions: list[tuple[object, str]] = []
    for armature in bpy.data.objects:
        if armature.type == "ARMATURE":
            rest_positions.append((armature.data, armature.data.pose_position))
            armature.data.pose_position = "REST"
    images = material_images(obj)
    copy_material_images(images, destination)
    bpy.context.view_layer.update()
    try:
        result = bpy.ops.wm.obj_export(
            filepath=str(obj_path),
            export_selected_objects=True,
            export_animation=False,
            forward_axis="NEGATIVE_Y",
            up_axis="Z",
            global_scale=1.0,
            apply_modifiers=True,
            apply_transform=True,
            export_eval_mode="DAG_EVAL_VIEWPORT",
            export_uv=True,
            export_normals=True,
            export_colors=False,
            export_materials=True,
            export_pbr_extensions=False,
            path_mode="STRIP",
            export_triangulated_mesh=False,
            export_object_groups=False,
            export_material_groups=False,
        )
    finally:
        for armature_data, pose_position in rest_positions:
            armature_data.pose_position = pose_position
        bpy.context.view_layer.update()

    if "FINISHED" not in result or not obj_path.is_file():
        raise RuntimeError(f"Blender did not finish exporting {label} to {obj_path}")

    vertex_count, dimensions = centered_grounded_obj(obj_path)
    mtl_path = destination / f"{label}.mtl"
    if not mtl_path.is_file():
        raise RuntimeError(f"Missing material file for {label}: {mtl_path}")
    missing_textures = [path for path in material_texture_paths(mtl_path) if not path.is_file()]
    if missing_textures:
        paths = "\n".join(str(path) for path in missing_textures)
        raise RuntimeError(f"{label} has MTL texture references that do not resolve:\n{paths}")

    face_count = sum(1 for line in obj_path.read_text(encoding="utf-8").splitlines()
                     if line.startswith("f "))
    if not face_count:
        raise RuntimeError(f"No faces were exported to {obj_path}")

    archive_path, texture_count = write_package_files(label, destination)
    return {
        "name": label,
        "obj": str(obj_path),
        "mtl": str(mtl_path),
        "zip": str(archive_path),
        "vertices": vertex_count,
        "faces": face_count,
        "dimensions": [round(value, 6) for value in dimensions],
        "textures": texture_count,
    }


def main() -> int:
    blender_args = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, default=DEFAULT_OUTPUT,
                        help="destination root (default: %(default)s)")
    args = parser.parse_args(blender_args)
    output_root = args.output if args.output.is_absolute() else ROOT / args.output
    output_root = output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)

    results = [export_model(label, object_name, output_root)
               for label, object_name in MODELS.items()]
    print("Vendor OBJ export summary:")
    for item in results:
        print(f"{item['name']}: {item['vertices']} vertices, {item['faces']} faces, "
              f"dimensions={item['dimensions']} (X/Y/Z)")
        print(f"  OBJ: {item['obj']}")
        print(f"  MTL: {item['mtl']}")
        print(f"  textures: {item['textures']}")
        print(f"  package: {item['zip']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
