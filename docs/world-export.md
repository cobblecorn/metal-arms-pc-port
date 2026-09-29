# Static level export

`tools/export_world_obj.py` asks the port to load a world through `-world-only` and exports its
embedded static world meshes as Wavefront OBJ. It does not start gameplay or initialize the audio
system. The tool leaves normal game launches and their audio settings alone.

Build the port, then run from the repository root:

```powershell
python tools/export_world_obj.py wedmmines01
```

The default Release executable is `build/Release/ma_port.exe`. Use `--config Debug` for the Debug
build, `--exe PATH` to select another executable, or `--output PATH` to choose an empty output
directory. By default, artifacts go to a new timestamped folder under ignored `build/export/`.
Retail data stays in `gamedata/`; it is not copied into the repository's tracked files.

Each successful export contains:

- `<world>.obj`: all exported static meshes combined, convenient for importing into Blender.
- `<world>.mtl` and `textures/*.tga`: material assignments and decoded game texture images.
- `meshes/<world>NNN.obj`: one OBJ per embedded world mesh, kept separate for chunked imports.
- `manifest.json`: mesh counts, included data, and format notes.
- `world_export.log` and `world_export_assets.log`: loader diagnostics.

To save a native Blender project as well, pass the combined OBJ to the Blender helper:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background --factory-startup `
  --python tools/import_obj_blender.py -- `
  "build/export/<export-folder>/<world>.obj" "build/export/<export-folder>/<world>.blend"
```

The OBJ includes triangle faces, vertex normals, texture coordinates in native level-space XYZ, and
material assignments. The exporter decodes the first available surface texture (Layer0 preferred)
from the game's tiled texture format into standard 32-bit TGA images for Blender. Its MTL files use
those images as diffuse maps. Game shader blending, lightmaps, animation, and other surface effects
are not reproduced, so materials can look different from the running game. The Blender helper opens
the viewport in Material Preview mode and scales its view clipping range to the level bounds. This is
needed for large maps, which can otherwise sit beyond Blender's default far clip and appear empty.

It exports embedded static world geometry only;
runtime liquid surfaces, placed world-shape objects, collision data, bots, and gameplay behavior are
not represented. In particular, this export will not answer whether the in-game liquid rendering fix
is visually correct.

In Blender, use **File → Import → Wavefront (.obj)** and select the combined `<world>.obj`. The
per-mesh files remain available if the combined scene is too large for an editor or game import. Roblox
Studio's 3D Importer currently accepts OBJ; the chunk files retain their original level-space
coordinates, though materials, collision, lighting, scripts, and gameplay must be authored separately.
See [Roblox Creator Hub: Work with third-party software](https://create.roblox.com/docs/art/overview-dcc).

The first export target is `wedmmines01` (“Seal the Mines 1”). To export another embedded world, pass
its resource name as the positional argument.
