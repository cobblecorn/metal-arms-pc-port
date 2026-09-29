"""Run inside Blender to import an OBJ and save a native .blend copy."""

import sys
from math import radians
from pathlib import Path

import bpy
from mathutils import Euler, Vector


def main() -> None:
    if "--" not in sys.argv:
        raise SystemExit("usage: blender --background --python tools/import_obj_blender.py -- scene.obj scene.blend")
    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 2:
        raise SystemExit("expected an input OBJ path and an output .blend path")

    obj_path = Path(arguments[0]).resolve()
    blend_path = Path(arguments[1]).resolve()
    if not obj_path.is_file():
        raise SystemExit(f"OBJ file not found: {obj_path}")
    if blend_path.suffix.lower() != ".blend":
        raise SystemExit("output path must end in .blend")
    if blend_path.exists():
        raise SystemExit(f"refusing to overwrite existing Blender file: {blend_path}")
    blend_path.parent.mkdir(parents=True, exist_ok=True)

    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    if hasattr(bpy.ops.wm, "obj_import"):
        bpy.ops.wm.obj_import(filepath=str(obj_path))
    else:
        bpy.ops.import_scene.obj(filepath=str(obj_path))

    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    face_count = sum(len(obj.data.polygons) for obj in mesh_objects)
    if not mesh_objects or face_count == 0:
        raise SystemExit("Blender imported no mesh faces from the OBJ")

    corners = [obj.matrix_world @ Vector(corner) for obj in mesh_objects for corner in obj.bound_box]
    minimum = Vector(tuple(min(point[axis] for point in corners) for axis in range(3)))
    maximum = Vector(tuple(max(point[axis] for point in corners) for axis in range(3)))
    center = (minimum + maximum) * 0.5
    bounds_diagonal = (maximum - minimum).length
    view_distance = max(bounds_diagonal * 1.25, 1.0)
    view_rotation = Euler((radians(62), 0.0, radians(42)), "XYZ").to_quaternion()
    for screen in bpy.data.screens:
        for area in screen.areas:
            if area.type == "VIEW_3D":
                region_3d = area.spaces.active.region_3d
                region_3d.view_location = center
                region_3d.view_distance = view_distance
                region_3d.view_rotation = view_rotation
                area.spaces.active.clip_start = max(bounds_diagonal * 0.0001, 0.01)
                area.spaces.active.clip_end = max(view_distance + bounds_diagonal * 2.0, bounds_diagonal * 4.0, 1000.0)
                area.spaces.active.shading.type = "MATERIAL"
                area.spaces.active.shading.use_scene_world = False
                area.spaces.active.shading.use_scene_lights = False

    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    print(f"Imported {len(mesh_objects)} mesh objects with {face_count} faces.")
    print(f"Saved {blend_path}")


if __name__ == "__main__":
    main()
