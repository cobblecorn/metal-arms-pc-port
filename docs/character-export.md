# Character model and animation export

`tools/export_characters.py` extracts the catalogued character rigs from the retail GameCube data,
decodes their textures, reads the matching `.mtx` animation clips, and assembles a native Blender
scene. It exports the models through the port's mesh converter so rigid and skinned parts use the
same path as the game. The importer converts the GameCube Y-up rigs to Blender's Z-up orientation,
centers them at ground level, and spaces each row from measured model bounds. Texture alpha is not
connected to surface opacity because it acts as a packed mask in these opaque character materials.

Build the Release executable, then run this from the repository root:

```powershell
cmake --build build --config Release --target ma_port
python tools/export_characters.py
```

Each run creates a new timestamped directory under `build/export/characters_.../`. The clean
verified package is `build/export/character_porting_kit_20260928/`: `metal_arms_characters.blend`
(all rigs), `metal_arms_vendors.blend` (Shady and Slim), and `metal_arms_enemies.blend` (an
eight-enemy sample). The raw extraction and logs are under
`build/export/characters_20260928_063553/`. Open a `.blend` with Blender's **File → Open**.
Character collections are arranged in a grid; each has a mesh, armature,
UV map, skin weights, materials, and its applicable animation actions. The scene opens in Material
Preview with the collection framed in view.

The full scene has 29 character meshes and rigs, 96,695 triangles, 107 packed texture images, and
586 model/clip actions. The raw folder contains per-model mesh JSON, decoded TGA images, and 537
parsed animation JSON files. The Zombie rig has 19 actions. The vendor file has 43 actions: 21 for
Shady and 22 for Slim. The enemy sample has eight models and 199 actions. Animation clips are
assigned when their bone names overlap at least 80% with a model's skeleton, with a catalog allowlist
for Slim's `arditable02` fold-table clip, which the source runtime explicitly loads. Other clips that
do not meet the match rule remain in `raw/animations/` and are listed in `import_manifest.json`.

To export only a focused set, pass resource IDs after `--resources`:

```powershell
python tools/export_characters.py --resources grdhshady00 grdislim_00
python tools/export_characters.py --resources grmggrunt00 grmeelite00 grdkkrunk00 grmzcorro00 grmppreda00 grmnsnipr00 grzazomba00 grzzboss_00
```

See [vendor-robot-porting.md](vendor-robot-porting.md) for Shady and Slim's C++ behavior and the
source files to use when recreating their store interactions. The focused projects in the clean
package can be regenerated from the existing full raw export with the Blender-only importer by adding
`--resources grdhshady00 grdislim_00` (or the enemy resource list) to its command line.

The catalog at `tools/character_models.json` records the included gameplay actors and rigs: Glitch
(single-player and multiplayer), Generic Droid, Grunt and variant, Miner, Elite Guard, Krunk, Mozer,
Jumper, Corrosive and variant, Predator, Probe/Leech, Scientist, Scout, Snarq, Sniper, Slosh, Titan,
Zombie, Zombie Boss, Swarmer, Vermin, Swarmer Boss, Mortar Droid, Shady, Slim, and Alloy. This is a
character/actor export, not a dump of weapons, vehicles, turret mechanisms, or level props.

The extraction process starts its own short-lived `ma_port` instance with `-no-audio`; that flag is
local to extraction and does not change normal game launches or another running game session. To
choose a different executable or output folder, use `--exe PATH`, `--config Debug`, or `--output PATH`.
To reuse existing raw model exports, `--skip-game-export` can be used when that output directory has
the model JSON and texture files and the destination Blender file does not already exist. The
Blender-only importer can also rebuild the scene directly:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" --background `
  --python tools/import_character_models.py -- `
  "build/export/<export-folder>/raw" `
  "build/export/<export-folder>/import_manifest.json" `
  "build/export/<export-folder>/characters_rebuilt.blend"
```

The exporter packs the selected diffuse texture into the `.blend` and preserves UVs, vertex normals,
bone hierarchy, and weights. It does not reproduce the game's layered shader effects, lightmaps,
emissive treatment, or runtime material behavior. Animation conversion uses source key times plus
intermediate samples for Blender playback; timing is represented at 30 frames per second. The
separate `arzadespar1` animation resource is retained in raw JSON but did not match the Zombie rig's
bone names well enough to attach as an action. The Blender file is not a Roblox import package; use
Blender's FBX export and adapt/validate the rig for Roblox separately if needed.
