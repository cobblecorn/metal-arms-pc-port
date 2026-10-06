# Shady and Slim vendor robots

The barter/store pair is Shady and Slim. The focused Blender project is
`build/export/character_porting_kit_20260928/metal_arms_vendors.blend`; it contains separate upright,
opaque rigs, packed textures, and the source-matched animation actions. Resource IDs are `grdhshady00`
(Shady) and `grdislim_00` (Slim). The current build has 21 Shady actions and 22 Slim actions. The raw
folder also preserves every parsed family clip as JSON.

## Source behavior and animation map

- [`Shady.cpp`](../ma/App/ma/Shady.cpp) and [`Shady.h`](../ma/App/ma/Shady.h) implement the vendor
  conversation and purchase reactions. Its mesh is `GRDHshady00`; the base animation slots name
  `ARDHidle003` (idle) and `ARDHturnR01` (look toward Slim). The wider `ARDH` family supplies the
  dialogue and gesture clips used by the barter animation combiner. Its state flow includes world
  idle, seeing the player, pitch/offer, barter idle/talk, purchase success/failure, upsell, and exit.
- [`Slim.cpp`](../ma/App/ma/Slim.cpp) and [`Slim.h`](../ma/App/ma/Slim.h) implement the storekeeper's
  table/sign routine and shopper reactions. The `ARDI` clips include idle variations, unfolding and
  folding the table, taking out/putting away/flipping the sign, waving, nodding, shrugging, laughing,
  thumbs-up/down, and looking at Shady. `arditable02` is explicitly listed by the game as the fold
  table clip; it has helper tracks that put it just under the general bone-match threshold, so the
  catalog explicitly includes it. `ardiwalkf01` is preserved in raw JSON but is not in Slim's runtime
  animation table.
- [`ShadyAnim.cpp`](../ma/App/ma/ShadyAnim.cpp) and [`Slim.cpp`](../ma/App/ma/Slim.cpp) use layered
  animation combiners. Slim's game behavior blends main, head, body, and idle channels; Shady blends
  idle and dialogue channels. The Blender actions are individual source clips, so a Roblox or other
  engine port still needs to implement those state transitions, blends, timing, dialogue, and purchase
  UI.
- [`BarterSystem.cpp`](../ma/App/ma/BarterSystem.cpp), [`BarterTypes.cpp`](../ma/App/ma/BarterTypes.cpp),
  [`BarterSound.cpp`](../ma/App/ma/BarterSound.cpp), and [`CamBarter.cpp`](../ma/App/ma/CamBarter.cpp)
  cover the interaction manager, item/purchase data, sound responses, and camera. These are source
  references for recreating behavior; the C++ implementation is not embedded in the `.blend` file.

## Slim action names

The Blender actions keep the retail `.mtx` resource names. This maps the runtime table in `Slim.cpp`
to the action names so they are easier to select in Blender or another tool:

| Purpose | Source action names |
|---|---|
| Idle variations | `ARDIidle001`, `ARDIidle002`, `ARDIidle02a`, `ARDIidle02b`, `ARDIidle02c` |
| Wave flag / hand | `ARDIflag_01`, `ARDIwave_01` |
| Nod yes / no | `ARDIheadys1`, `ARDIheadno1` |
| Shrug / laugh | `ARDIshrug`, `ARDIlaugh01` |
| Unfold / fold table | `ARDItable01`, `ARDItable02` |
| Pull out / flip sign / put away | `ARDIsign_01`, `ARDIsign_02`, `ARDIsign_03`, `ARDIsign_04` |
| Thumbs up / down | `ARDIthumbu1`, `ARDIthumbd1` |
| Look toward Shady | `ARDIlookl01`, `ARDIlookl02` |

The imported `ARDIsigndmo` is an extra family clip beyond the 21 resources named by Slim's runtime
table. Shady's two explicit base slots are `ARDHidle003` and `ARDHturnR01`; the vendor project also
includes the other bone-matched `ARDH` dialogue/gesture resources.

Rebuild just the vendor asset set from the repository root with:

```powershell
python tools/export_characters.py --resources grdhshady00 grdislim_00
```

The export's raw animation JSON keeps the source clip names, durations, and bone tracks. The Blender
importer saves the rigs upright for Z-up tools, centers them at ground level, and spaces them by their
measured bounds. It leaves source texture alpha disconnected from surface opacity because these bot
textures use that channel as a mask; this avoids ghosted-looking models during external export.

## Separate static OBJ packages

The vendor blend also exports one standalone, textured OBJ package per robot:

- Shady: [`Shady.obj`](../build/export/character_porting_kit_20260928/obj/Shady/Shady.obj) and
  [`Shady_OBJ.zip`](../build/export/character_porting_kit_20260928/obj/Shady_OBJ.zip)
- Slim: [`Slim.obj`](../build/export/character_porting_kit_20260928/obj/Slim/Slim.obj) and
  [`Slim_OBJ.zip`](../build/export/character_porting_kit_20260928/obj/Slim_OBJ.zip)

Each ZIP contains that robot's OBJ, MTL, required TGA textures, and an import note. Keep the OBJ, MTL,
and textures together when importing. These meshes are static rest poses with Z up, centered in X/Y, and
grounded at Z=0; OBJ does not carry the skeleton or animation actions. Use the vendor `.blend` above for
animation work.

Rebuild the packages from the repository root with Blender 5.x:

```powershell
blender --background build/export/character_porting_kit_20260928/metal_arms_vendors.blend `
  --python tools/export_vendor_obj.py -- `
  --output build/export/character_porting_kit_20260928/obj
```
