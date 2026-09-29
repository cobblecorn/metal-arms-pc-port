"""Build a textured, rigged Blender scene from ma_port character export JSON."""

from __future__ import annotations

import json
import math
import argparse
import sys
from pathlib import Path

import bpy
from mathutils import Euler, Matrix, Quaternion, Vector
from math import radians


def source_matrix(columns: list[list[float]]) -> Matrix:
    """Convert the source row-vector affine basis into Blender's column-vector matrix."""
    right, up, front, position = columns
    return Matrix((
        (right[0], up[0], front[0], position[0]),
        (right[1], up[1], front[1], position[1]),
        (right[2], up[2], front[2], position[2]),
        (0.0, 0.0, 0.0, 1.0),
    ))


def load_json(path: Path) -> dict:
    with path.open("r", encoding="utf-8") as stream:
        return json.load(stream)


def make_materials(model: dict, texture_dir: Path, resource: str) -> list:
    materials = []
    for entry in model.get("materials", []):
        index = int(entry["index"])
        texture_name = entry.get("texture")
        material = bpy.data.materials.new(f"{resource}_mat_{index}")
        material.diffuse_color = (0.55, 0.55, 0.55, 1.0)
        material.use_nodes = True
        if texture_name:
            image_path = texture_dir / f"{texture_name}.tga"
            if image_path.is_file():
                image = bpy.data.images.load(str(image_path), check_existing=True)
                try:
                    image.pack()
                except RuntimeError:
                    pass
                nodes = material.node_tree.nodes
                shader = next((node for node in nodes if node.type == "BSDF_PRINCIPLED"), None)
                if shader:
                    texture = nodes.new("ShaderNodeTexImage")
                    texture.image = image
                    texture.label = texture_name
                    material.node_tree.links.new(texture.outputs["Color"], shader.inputs["Base Color"])
                    # The exported GameCube alpha channel is also used as a packed mask on
                    # opaque bot textures. Treating it as surface opacity made the rigs look
                    # ghosted in Blender and in downstream exports.
                    if "Alpha" in shader.inputs:
                        shader.inputs["Alpha"].default_value = 1.0
                material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
        materials.append(material)
    return materials


def make_armature(model: dict, collection: bpy.types.Collection, resource: str):
    armature_data = bpy.data.armatures.new(f"{resource}_Rig")
    armature_object = bpy.data.objects.new(f"{resource}_Armature", armature_data)
    collection.objects.link(armature_object)
    armature_object.show_in_front = True
    armature_data.display_type = "STICK"

    for obj in bpy.context.selected_objects:
        obj.select_set(False)
    bpy.context.view_layer.objects.active = armature_object
    armature_object.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = []
    source_bones = model.get("bones", [])
    for index, bone in enumerate(source_bones):
        name = bone.get("name") or f"bone_{index:03d}"
        edit_bone = armature_data.edit_bones.new(name)
        edit_bone.matrix = source_matrix(bone["bone_to_model"])
        edit_bone.length = 0.12
        edit_bones.append(edit_bone)
    for index, bone in enumerate(source_bones):
        parent_index = int(bone.get("parent", 255))
        if 0 <= parent_index < len(edit_bones) and parent_index != index:
            edit_bones[index].parent = edit_bones[parent_index]
            edit_bones[index].use_connect = False
    bpy.ops.object.mode_set(mode="OBJECT")
    armature_object.select_set(False)
    return armature_object, source_bones


def make_mesh(model: dict, texture_dir: Path, collection: bpy.types.Collection,
              armature_object: bpy.types.Object, source_bones: list, resource: str):
    vertices: list[tuple[float, float, float]] = []
    vertex_uvs: list[tuple[float, float]] = []
    vertex_normals: list[tuple[float, float, float]] = []
    influences: list[list[tuple[int, float]]] = []
    faces: list[tuple[int, int, int]] = []
    polygon_materials: list[int] = []
    material_count = len(model.get("materials", []))

    for part in model.get("parts", []):
        vertex_base = len(vertices)
        material_index = int(part.get("material", 0))
        if material_index < 0 or material_index >= material_count:
            material_index = 0
        rigid_bone = int(part.get("rigid_bone", 0))
        for row in part.get("vertices", []):
            if len(row) != 16:
                raise ValueError(f"{resource}: expected 16 values in exported vertex, got {len(row)}")
            vertices.append((float(row[0]), float(row[1]), float(row[2])))
            vertex_normals.append((float(row[3]), float(row[4]), float(row[5])))
            vertex_uvs.append((float(row[6]), 1.0 - float(row[7])))
            if part.get("weighted"):
                weighted = []
                for slot in range(4):
                    bone_index = int(row[8 + slot])
                    weight = max(0.0, float(row[12 + slot]))
                    if bone_index < len(source_bones) and weight > 1.0e-6:
                        weighted.append((bone_index, weight))
                if not weighted and source_bones:
                    weighted = [(max(0, min(rigid_bone, len(source_bones) - 1)), 1.0)]
                influences.append(weighted)
            else:
                bone_index = max(0, min(rigid_bone, max(0, len(source_bones) - 1)))
                influences.append([(bone_index, 1.0)] if source_bones else [])

        indices = [int(value) for value in part.get("indices", [])]
        if len(indices) % 3:
            raise ValueError(f"{resource}: exported index count is not divisible by three")
        for tri in range(0, len(indices), 3):
            face = tuple(vertex_base + indices[tri + k] for k in range(3))
            if any(index >= len(vertices) for index in face):
                raise ValueError(f"{resource}: exported triangle index is out of range")
            faces.append(face)
            polygon_materials.append(material_index)

    if not vertices or not faces:
        raise ValueError(f"{resource}: export has no visible triangle geometry")

    mesh_data = bpy.data.meshes.new(f"{resource}_Mesh")
    mesh_data.from_pydata(vertices, [], faces)
    mesh_data.update()
    mesh_object = bpy.data.objects.new(f"{resource}_Model", mesh_data)
    collection.objects.link(mesh_object)
    mesh_object.parent = armature_object

    materials = make_materials(model, texture_dir, resource)
    for material in materials:
        mesh_data.materials.append(material)
    for polygon, material_index in zip(mesh_data.polygons, polygon_materials):
        polygon.material_index = min(material_index, max(0, len(materials) - 1))
        polygon.use_smooth = True

    uv_layer = mesh_data.uv_layers.new(name="UVMap")
    for loop in mesh_data.loops:
        uv_layer.data[loop.index].uv = vertex_uvs[loop.vertex_index]
    if hasattr(mesh_data, "normals_split_custom_set_from_vertices"):
        mesh_data.normals_split_custom_set_from_vertices(vertex_normals)

    bone_names = [bone.get("name") or f"bone_{i:03d}" for i, bone in enumerate(source_bones)]
    groups = [mesh_object.vertex_groups.new(name=name) for name in bone_names]
    for vertex_index, weighted in enumerate(influences):
        for bone_index, weight in weighted:
            if weight > 1.0e-6:
                groups[bone_index].add([vertex_index], weight, "REPLACE")
    modifier = mesh_object.modifiers.new("Skin", "ARMATURE")
    modifier.object = armature_object
    modifier.use_vertex_groups = True
    modifier.use_bone_envelopes = False
    mesh_object.parent = armature_object
    return mesh_object


def sample_scalar(keys: list[dict], time: float) -> float:
    if time <= keys[0]["time"]:
        return float(keys[0]["value"])
    if time >= keys[-1]["time"]:
        return float(keys[-1]["value"])
    for low, high in zip(keys, keys[1:]):
        if time <= high["time"]:
            alpha = (time - low["time"]) / max(high["time"] - low["time"], 1.0e-12)
            return (1.0 - alpha) * float(low["value"]) + alpha * float(high["value"])
    return float(keys[-1]["value"])


def sample_vector(keys: list[dict], time: float) -> Vector:
    if time <= keys[0]["time"]:
        return Vector(keys[0]["value"])
    if time >= keys[-1]["time"]:
        return Vector(keys[-1]["value"])
    for low, high in zip(keys, keys[1:]):
        if time <= high["time"]:
            span = max(high["time"] - low["time"], 1.0e-12)
            return Vector(low["value"]).lerp(Vector(high["value"]), (time - low["time"]) / span)
    return Vector(keys[-1]["value"])


def sample_quaternion(keys: list[dict], time: float) -> Quaternion:
    def read(entry: dict) -> Quaternion:
        x, y, z, w = entry["value_xyzw"]
        value = Quaternion((w, x, y, z))
        if value.magnitude < 1.0e-8:
            value = Quaternion()
        else:
            value.normalize()
        return value

    if time <= keys[0]["time"]:
        return read(keys[0])
    if time >= keys[-1]["time"]:
        return read(keys[-1])
    for low, high in zip(keys, keys[1:]):
        if time <= high["time"]:
            span = max(high["time"] - low["time"], 1.0e-12)
            return read(low).slerp(read(high), (time - low["time"]) / span)
    return read(keys[-1])


def add_curve(channelbag, data_path: str, index: int, samples: list[tuple[float, float]]) -> None:
    curve = channelbag.fcurves.new(data_path=data_path, index=index)
    points = curve.keyframe_points
    points.add(len(samples))
    for point, (frame, value) in zip(points, samples):
        point.co = (frame, value)
        point.interpolation = "LINEAR"
    curve.update()


def track_sample_times(track: dict, duration: float) -> list[float]:
    """Keep source key times and add quarter rotations between keys for smooth SLERP playback."""
    times = {0.0, duration}
    for channel in ("scale", "translation", "rotation"):
        times.update(float(key["time"]) for key in track.get(channel, []))
    rotations = track.get("rotation", [])
    for low, high in zip(rotations, rotations[1:]):
        start = float(low["time"])
        end = float(high["time"])
        if end > start:
            times.add(start + (end - start) * 0.25)
            times.add(start + (end - start) * 0.50)
            times.add(start + (end - start) * 0.75)
    return sorted(max(0.0, min(duration, time)) for time in times)


def add_actions(model: dict, metadata: dict, animation_dir: Path,
                armature_object: bpy.types.Object, source_bones: list,
                fps: int = 30) -> list[str]:
    names = metadata.get("animations_by_model", {}).get(model["resource"], [])
    if not names:
        return []
    bone_lookup = {str(bone.get("name", "")).casefold(): index
                   for index, bone in enumerate(source_bones)}
    created: list[str] = []
    for animation_name in names:
        path = animation_dir / f"{animation_name}.json"
        if not path.is_file():
            continue
        animation = load_json(path)
        duration = float(animation.get("duration_seconds", 0.0))
        if not math.isfinite(duration) or duration <= 0.0:
            continue

        action = bpy.data.actions.new(f"{model['resource']}_{animation_name}")
        action.use_fake_user = True
        armature_object.animation_data_create()
        armature_object.animation_data.action = action
        slot = action.slots.new(armature_object.id_type, f"{model['resource']}_{animation_name}")
        armature_object.animation_data.action_slot = slot
        layer = action.layers.new("Animation")
        strip = layer.strips.new(type="KEYFRAME")
        channelbag = strip.channelbag(slot, ensure=True)
        for track in animation.get("tracks", []):
            bone_index = bone_lookup.get(str(track.get("bone", "")).casefold())
            if bone_index is None:
                continue
            scale_keys = track.get("scale", [])
            translation_keys = track.get("translation", [])
            rotation_keys = track.get("rotation", [])
            if not scale_keys or not translation_keys or not rotation_keys:
                continue
            rest_local = source_matrix(source_bones[bone_index]["bone_to_parent"])
            inverse_rest = rest_local.inverted_safe()
            local_samples = []
            for time in track_sample_times(track, duration):
                scale = sample_scalar(scale_keys, time)
                translation = sample_vector(translation_keys, time)
                rotation = sample_quaternion(rotation_keys, time)
                animated_local = Matrix.LocRotScale(translation, rotation, Vector((scale, scale, scale)))
                local_samples.append((time, (inverse_rest @ animated_local).decompose()))

            # Static channels equal to the bone's at-rest transform need no curves.
            if all(
                location.length < 1.0e-5 and
                abs(rotation.angle) < 1.0e-4 and
                (scale - Vector((1.0, 1.0, 1.0))).length < 1.0e-5
                for _, (location, rotation, scale) in local_samples
            ):
                continue

            bone_name = source_bones[bone_index].get("name") or f"bone_{bone_index:03d}"
            prefix = f"pose.bones[{json.dumps(bone_name)}]"
            curve_values = {"location": [[], [], []],
                            "rotation_quaternion": [[], [], [], []],
                            "scale": [[], [], []]}
            for time, (location, rotation, scale) in local_samples:
                frame = 1.0 + time * fps
                values = {
                    "location": tuple(location),
                    "rotation_quaternion": tuple(rotation),
                    "scale": tuple(scale),
                }
                for property_name, components in values.items():
                    for component, value in enumerate(components):
                        curve_values[property_name][component].append((frame, float(value)))
            for property_name, components in curve_values.items():
                for component, samples in enumerate(components):
                    add_curve(channelbag, f"{prefix}.{property_name}", component, samples)

        created.append(animation_name)
    armature_object.animation_data.action = None
    return created


def main() -> None:
    if "--" not in sys.argv:
        raise SystemExit(
            "usage: blender --background --python tools/import_character_models.py -- "
            "raw_dir import_manifest.json output.blend [--resources RESOURCE ...]"
        )
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("raw_dir", type=Path)
    parser.add_argument("import_manifest", type=Path)
    parser.add_argument("output_blend", type=Path)
    parser.add_argument("--resources", nargs="+", metavar="RESOURCE",
                        help="include only these character resources from the manifest")
    arguments = parser.parse_args(sys.argv[sys.argv.index("--") + 1:])
    raw_dir = arguments.raw_dir.resolve()
    import_manifest_path = arguments.import_manifest.resolve()
    blend_path = arguments.output_blend.resolve()
    if blend_path.suffix.lower() != ".blend":
        raise SystemExit("output path must end in .blend")
    if blend_path.exists():
        raise SystemExit(f"refusing to overwrite existing Blender file: {blend_path}")
    metadata = load_json(import_manifest_path)
    models = metadata.get("models", [])
    if arguments.resources:
        by_resource = {entry["resource"].casefold(): entry for entry in models}
        unknown = [name for name in arguments.resources if name.casefold() not in by_resource]
        if unknown:
            raise SystemExit("unknown character resources in manifest: " + ", ".join(unknown))
        selected = {name.casefold() for name in arguments.resources}
        models = [entry for entry in models if entry["resource"].casefold() in selected]
        metadata["models"] = models
        metadata["animations_by_model"] = {
            resource: clips for resource, clips in metadata.get("animations_by_model", {}).items()
            if resource.casefold() in selected
        }
    if not models:
        raise SystemExit("the import manifest contains no selected character models")
    model_dir = raw_dir
    texture_dir = raw_dir / "textures"
    animation_dir = raw_dir / "animations"
    missing_models = [entry["resource"] for entry in models
                      if not (model_dir / f"{entry['resource']}.json").is_file()]
    if missing_models:
        raise SystemExit("missing model exports: " + ", ".join(missing_models))

    blend_path.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    root = bpy.context.scene.collection
    characters = bpy.data.collections.new("Metal Arms Characters")
    root.children.link(characters)
    bpy.context.scene.render.fps = 30

    summaries = []
    assembled = []
    for entry in models:
        resource = entry["resource"]
        model = load_json(model_dir / f"{resource}.json")
        collection = bpy.data.collections.new(f"{entry.get('character', resource)} [{resource}]")
        characters.children.link(collection)
        armature_object, source_bones = make_armature(model, collection, resource)
        # Fang/GameCube character rigs are Y-up. Convert at the armature root so the
        # hierarchy, mesh, and every animation share Blender's conventional Z-up space.
        armature_object.rotation_mode = "XYZ"
        armature_object.rotation_euler.x = radians(90.0)
        mesh_object = make_mesh(model, texture_dir, collection, armature_object, source_bones, resource)
        actions = add_actions(entry, metadata, animation_dir, armature_object, source_bones)
        mesh_object.location = (0.0, 0.0, 0.0)
        summaries.append({"resource": resource, "character": entry.get("character", resource),
                          "vertices": len(mesh_object.data.vertices),
                          "triangles": len(mesh_object.data.polygons),
                          "bones": len(source_bones), "animations": actions})
        assembled.append({"armature": armature_object, "mesh": mesh_object})

    scene = bpy.context.scene
    scene.render.filepath = "//"
    bpy.context.view_layer.update()

    # Center each character on its own ground-plane origin and pack rows using its
    # actual bounds. A fixed 8-unit grid caused large rigs to overlap smaller ones.
    for item in assembled:
        armature_object = item["armature"]
        mesh_object = item["mesh"]
        bounds = [mesh_object.matrix_world @ Vector(corner) for corner in mesh_object.bound_box]
        minimum = Vector(tuple(min(point[axis] for point in bounds) for axis in range(3)))
        maximum = Vector(tuple(max(point[axis] for point in bounds) for axis in range(3)))
        item["width"] = max(maximum.x - minimum.x, 0.1)
        item["depth"] = max(maximum.y - minimum.y, 0.1)
        armature_object.location.x -= (minimum.x + maximum.x) * 0.5
        armature_object.location.y -= (minimum.y + maximum.y) * 0.5
        armature_object.location.z -= minimum.z

    if assembled:
        bpy.context.view_layer.update()
        columns = min(4, max(1, math.ceil(math.sqrt(len(assembled)))))
        gap = max(1.5, max(max(item["width"], item["depth"]) for item in assembled) * 0.15)
        row_y = 0.0
        for start in range(0, len(assembled), columns):
            row = assembled[start:start + columns]
            row_depth = max(item["depth"] for item in row)
            x = 0.0
            for item in row:
                item["armature"].location.x += x + item["width"] * 0.5
                item["armature"].location.y += row_y + row_depth * 0.5
                x += item["width"] + gap
            row_y += row_depth + gap

    bpy.context.view_layer.update()
    bounds = [
        obj.matrix_world @ Vector(corner)
        for obj in scene.objects if obj.type == "MESH"
        for corner in obj.bound_box
    ]
    if bounds:
        minimum = Vector(tuple(min(point[axis] for point in bounds) for axis in range(3)))
        maximum = Vector(tuple(max(point[axis] for point in bounds) for axis in range(3)))
        center = (minimum + maximum) * 0.5
        diagonal = (maximum - minimum).length
        for screen in bpy.data.screens:
            for area in screen.areas:
                if area.type == "VIEW_3D":
                    area.spaces.active.shading.type = "MATERIAL"
                    area.spaces.active.clip_start = max(diagonal * 0.0001, 0.01)
                    area.spaces.active.clip_end = max(diagonal * 3.0, 1000.0)
                    region = area.spaces.active.region_3d
                    region.view_location = center
                    region.view_distance = max(diagonal * 1.25, 10.0)
                    region.view_rotation = Euler((radians(68), 0.0, radians(42)), "XYZ").to_quaternion()
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == "VIEW_3D":
                area.spaces.active.shading.type = "MATERIAL"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path), compress=True)
    summary_path = blend_path.with_name(blend_path.stem + "_summary.json")
    summary_path.write_text(json.dumps(summaries, indent=2) + "\n", encoding="utf-8")
    total_triangles = sum(item["triangles"] for item in summaries)
    total_animations = sum(len(item["animations"]) for item in summaries)
    print(f"Saved {blend_path} with {len(summaries)} characters, {total_triangles} triangles, "
          f"and {total_animations} assigned animation actions.")


if __name__ == "__main__":
    main()
