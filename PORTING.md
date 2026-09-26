# Metal Arms: Glitch in the System - PC source port

Work in progress: a native 32-bit Windows build of Swingin' Ape's engine (Fang2) and
game, targeting the retail **GameCube** data set.

The original source lives under `ma/` and is kept as close to untouched as possible.
`main` is the verbatim source drop; all porting work is on the `x86-port` branch, so
`git diff main..x86-port` is exactly what was changed.

## Building

Requires Visual Studio 2022 (C++ workload, x86 tools), CMake 3.20+, Python 3.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Debug --target ma_port

Output: `build/Debug/ma_port.exe` (+ `binkw32.dll`). It must be 32-bit (see below).

## Running

Retail data is **not** in this repo. Put the extracted disc files in `gamedata/files`
(the `.mst` master file and the `Movies` folder), or point at them:

    ma_port -data <dir> [-mst <file>] [-res WxH] [-fullscreen] [-level <world>] [-world-only <world>] [-log <file>] [-shots <dir>] [-shot-every <frames>] [-mouse-sensitivity <n>]

`-level <world>` starts the normal generic level path. `-world-only <world>` loads and converts
the WLD resource, then exits before localized setup and gameplay entity creation.

`tools/mst_list.py` lists/extracts a `.mst` master file (GameCube byte order).
`tools/gamedata_dump.py` inspects extracted binary `.csv` game-data tables as indexed JSON;
write reports under ignored `build/` because they contain retail asset values.

## Desktop controls

| Control | Action |
|---|---|
| WASD | Move (diagonal speed is normalized) |
| F1 | Toggle raw mouse look |
| Mouse / arrow keys | Look / turn |
| Space | Jump |
| E | Weapons menu (observed in current run; adapter intends action) |
| F | Melee |
| Left / right mouse button | Primary / secondary fire (requires a supported weapon) |
| Q | Throwables menu (observed in current run) |
| R | Adapter maps secondary selection; active menu mapping needs reconciliation |
| 1 / 2 / 3 / 4 | Quick-select up / right / down / left |
| Escape / Enter | Pause; Escape also releases the mouse |
| Alt-F4 | Close the game |

The user confirmed responsive mouse look and reported that some weapons appear to work.
The observed Q/E menus differ from the adapter/default action table; keep this discrepancy
open until the active input path is traced. With a throwable equipped and ammo available,
right mouse maps to secondary fire and starts the throw. The HUD selection code accepts W/S
to scroll while a selection menu is held open; releasing the menu button equips the selection.
Throwable behavior has not yet been confirmed interactively.

Mouse look uses raw relative motion, applied as angular displacement without the controller's
acceleration curve or turn-speed cap. `-mouse-sensitivity 0.1` is the default, in degrees per
mouse count, before the game's look-sensitivity multiplier. Use `0.05` for half that speed.
F1 must be pressed again after switching away from the game or entering menu controls. All
inputs return to neutral when the game loses focus. Desktop defaults to non-inverted look;
loaded profiles retain their own setting.

XInput controllers occupy ports 1-4 and can connect after launch. The keyboard shares port 1.
The adapter intends A for jump, Y for action, B/X for weapon selection, triggers for fire,
and RB/right-stick click for melee. Menu-button behavior needs the reconciliation noted above. Controller mapping has automated coverage; physical controller
behavior and rumble still need verification. Legacy DirectInput-only pads are not supported by
the new desktop adapter.

Capture through the engine with `-shots build/shots -shot-every 1800`. This saves numbered BMPs
from the D3D back buffer. The console, `-log` output, CRT diagnostics, and shader/texture probe
environment variables remain available.

## What changed and why

| Area | Change |
|---|---|
| Build | New CMake project. Source lists are generated from the original `.vcproj` files (`tools/gen_sources.py`). 32-bit only. |
| Renderer | The engine targets Direct3D 8, which modern Windows SDKs no longer ship. `port/compat/d3d8.h` + `d3d8_compat.cpp` wrap D3D9 behind the D3D8 interface the engine expects (vertex-declaration token translation, sampler-state split, shader handle tables, BaseVertexIndex, ZBIAS->DEPTHBIAS). `d3dx8.h` reimplements the few D3DX math/texture helpers used. |
| Shaders | `tools/build_shaders.py` assembles the `.nvv`/`.nvp` sources with the Windows D3D assembler into the `CompiledVShader*.h`/`PShader*.h` headers the original build produced with the DX8 SDK. |
| Threading | `fdx8loop.cpp`: the game-loop thread was an MFC `CWinThread`; replaced with a small Win32 thread class with the same lifecycle. |
| Entry point | `port/main_win.cpp` replaces the MFC "mawin" dialog. Same boot sequence as the Xbox `main.cpp`. |
| Struct layout | Built with `_FANGDEF_WINGC` (GameCube alignment, no SSE) so structures read from GC data line up in memory. This mode was previously tools-only, so `fdx8gcmath_*.inl` gained 12 math functions the runtime needs (scalar code from the GC reference). |
| GameCube assets | `port/gcdata.cpp` converts big-endian CSV, particle, animation, world header, visibility, and world-init records, and decodes GX tiled TGA images. `port/gcmesh.cpp` expands supported GX display lists into D3D vertex and index buffers. |
| AI graphs | `AIGraph.cpp` converts retail GameCube `.gt` graph headers, vertices, edge slots, and POIs before pointerization; the GC build uses the five edge slots present in the retail format. |
| Language | C++ rule changes since 2003: anonymous-union members made implicit copies deleted (`CFSphere`, `CFRect2D`), implicit-int declarations, `Lock(void**)`, modern MASM operand sizes. |

## Status

- [x] Engine (Fang2), scripting VM, game logic and AI compile and link.
- [x] Executable starts; engine boots; D3D device/mode enumeration works on a real GPU.
- [x] Load the retail GameCube master file. The runtime swaps its big-endian header and
      directory, accepts the GC platform version, and warns about newer asset compiler versions.
- [x] Convert GameCube CSV tables to host byte order, including pointer offsets and UTF-16 strings.
- [x] Decode GameCube TGA textures from GX tiled formats, including CMPR and split S3TCx2,
      into linear ARGB pixels for the D3D texture path.
- [x] Convert static and skinned GameCube meshes, including streamed NBT3 display lists, to
      D3D vertex/index buffers. Convert embedded kDOP collision trees and their triangle data.
- [x] Adapt the retail version 8 particle layout to the source version 7 runtime layout;
      particle textures now load during startup.
- [x] Convert WLD headers, visibility trees, shape-init records, and world mesh tables.
      `-world-only we01multi01` loads 22 meshes, 22 portals, 22 volumes, 22 cells, and 101
      world-init shapes through the resource loader. Streamed GX display-list data is retained
      for mesh conversion.
- [x] Convert retail `.mtx` animation headers, bone records, key-time arrays, and compressed or
      floating-point tracks. Startup loads character animations during world entity creation.
- [x] Convert retail GameCube `.gt` AI graphs. The converter validates graph counts, array ranges,
      edge counts and edge targets before swapping the header, vertices, edges, and POIs. The
      full `we01multi01` level path converted a graph with 153 vertices and reached boot completion.
- [x] Convert GameCube `.fnt` fonts. `ftext.cpp`'s font loader (`ftext_Load()`) is a bespoke
      reader that never went through the generic `fresload`/`gcdata_Convert` hook and never
      byte-swapped anything; `gcdata_ConvertFont()` validates and swaps the header and its
      three variable-length arrays (letter buckets, bucket-letters, letters) in place before
      the loader's own offset-to-pointer fixup runs. This was crashing every launch (a wild
      pointer read while drawing the first piece of HUD text); see the CRT diagnostics entry
      below for how this was actually diagnosed instead of just hanging.
- [x] Fixed a real, pre-existing engine bug (not GC-specific) in `fresload.cpp`'s mesh-portion
      loader: its failure path called `fang_Free()` on memory that was actually allocated via
      `fmem_Alloc()`/`fres_AlignedAlloc()` (both `CFHeap`-backed, a completely different
      allocator with no `fang_Malloc`-style tracking header). `fang_Free()` read whatever bytes
      happened to precede the block as if they were that header and corrupted them trying to
      unlink it, crashing in `flinklist_Remove()`. This was presumably always latent - retail
      PASM output apparently never failed a mesh conversion, so this path was never exercised -
      until this port's own (still incomplete) GameCube mesh converter genuinely rejected an
      unsupported display list and hit it. Removed the redundant, wrong free (the
      `fres_ReleaseFrame()` immediately above it already reclaims the same memory, since its
      frame marker was captured before the allocation). This turned a hard crash into a clean
      "this mesh/level can't fully load yet" outcome on real single-player levels.
- [x] Render the single-player `wecdsneak01` scene with textures, Glitch, and the HUD after
      correcting D3D shader input declarations and affine bone/instance transforms.
- [ ] Extend mesh coverage beyond the currently supported data. Collision conversion failures
      still drop collision for that mesh; display lists with more than four bones use an
      approximation that needs review.
- [ ] Convert scripts (`.sma`) and other runtime resources. Confirmed via
      `CFScriptSystem::LoadScriptsFromFile : No script names found for this level.` that no
      scripts even attempt to run for the levels tested so far - so whatever else is wrong,
      it isn't yet a script-conversion problem for these specific levels.
- [ ] Audio (GC MusyX / DSP-ADPCM streams) and Bink video hookup.
- [x] Keyboard controls and direct raw mouse look, confirmed interactively in `wecdsneak01`.
      XInput mapping includes deadzones, separate triggers, focus handling, and hotplug support.
- [ ] Verify physical XInput controllers, vehicle-specific mouse aiming, and rumble.
- [ ] Save games (memory-card layer -> files).
- [x] Engine back-buffer BMP capture through `-shots` / `-shot-every`. The original
      `port/screenshot_port.cpp` keyboard shortcut implementation is still a stub.

## Known issues / things worth a reviewer's eye

- `SetRenderState(D3DRS_ZBIAS)` is mapped to `D3DRS_DEPTHBIAS` with a guessed scale
  (`d3d8_compat.cpp`); decals/coplanar geometry may z-fight until tuned.
- `D3DXLoadSurfaceFromMemory` supports only same-format (and 32-bit interchange) copies.
- D3D8-only render states with no D3D9 equivalent are silently ignored.
- The launcher writes Fang's resource load log to `ma_port_asset_log.txt`; the `-log` option
  remains the engine/debug log.
- Retail data was built with newer tool versions than this source snapshot (e.g. mesh
  compiler 0x39 vs 0x37 in `fdata.h`); the runtime doesn't enforce these, but layouts
  may differ slightly.
- Retail `Difficulty.csv` has 20 fields in its `Diff` table; this source expects 8 fields
  for four difficulty levels and therefore falls back to its defaults. The retail flamer
  table also has newer tail columns, which are ignored by the older source vocabulary.
- Retail `w_laser.csv` uses a substantially newer schema and remains unavailable. The blaster
  loader now reads its 45-field layout and handles the three available player variants without
  initializing absent military variants. Its two extra numeric fields are retained but their
  behavior is not implemented. Blaster resource creation, firing and upgrades need runtime
  confirmation. Other item tables still report unsupported retail collectable names.
- Weapon selection rejects unavailable runtime weapon objects and retains the previous equipped
  item. Starting with Empty Secondary now preserves the inventory count so throwables remain
  selectable. These changes build successfully; runtime confirmation is pending.
- The wrapper system accepts trailing retail phrases and reads its six-field screen-name stride,
  selecting the Xbox UI entries and ignoring the two screens this source does not define.
- A regular `-level we01multi01` launch now passes wrapper and world setup and reaches
  `END OF BOOTUP`. Audio remains disabled: `fsndfx.cpp` skips parsing GC SFX banks, so sound groups
  have no loaded sound definitions.
- Older multiplayer `we01multi01` logs showed a repeated startup cycle. The later skinned-mesh,
  collision, shader, and matrix fixes supersede the earlier blocker descriptions in the handoff.
  Use `wecdsneak01` as the current interactive baseline; multiplayer behavior needs a fresh check.
- Current captures still show some scenery surfaces as solid black. Rendering completeness
  remains a separate task from the now-working movement and mouse look.
- The animation adapter bounds-checks offsets, counts, and track ranges. It rejects animations
  with overlapping track ranges and files with more bones than the source runtime's 127-bone
  limit; those assets still need a format-specific review.
- The `.gt` converter matches the analyzed retail corpus: its GameCube graph header is 40 bytes,
  each vertex is 128 bytes with five edge slots, and each POI is 8 bytes. Recheck those layout
  assumptions if support is extended to other platforms or asset versions.
- `-world-only` deliberately skips world-shape entity creation. Its diagnostic exits successfully
  but prints debug allocator warnings for game-system objects during shutdown; it does not
  represent normal gameplay teardown.
- The particle adapter is based on the retail corpus layout: each version 8 keyframe has an
  additional 8-byte tail field, removed before the version 7 structures are byte-swapped.
  It validates that both removed fields are zero, plus the shifted texture name, sampling
  interval, and zeroed list state before converting a file.
- `main_win.cpp` now redirects every Debug-CRT diagnostic (asserts, `/RTC` stack/uninitialized-
  variable failures, pure-virtual calls, invalid-parameter aborts) to the log instead of a
  blocking "Microsoft Visual C++ Runtime Library" dialog. Without this, a hit anywhere in the
  Debug build looks exactly like a hang: the process is still alive, sitting in a modal message
  box nobody is there to click. This is how the font crash below was actually found, instead of
  just timing out. Non-fatal asserts now log and continue (matching "Ignore"); this can let a
  real bug run further than it would on the original platforms before something else notices -
  treat a firing assert as a real bug to fix, not background noise.
- Found via the above: `CBotGlitch::_InitInventory()` was passing a retail inventory's current-
  weapon slot straight to `_ChangeWeaponIndex()` with no bounds check against this source's
  reduced weapon set (see the laser/blaster schema note above) - a real out-of-bounds array read
  that the original `FASSERT` alone did not prevent (`FASSERT` compiles out entirely in
  Release/Production builds, so this was always a live bug, not just a debug-build nuisance).
  Fixed at both the call site (fall back to slot 0 when the retail slot is out of range) and in
  `_ChangeWeaponIndex()` itself (bounds-checked no-op instead of undefined behavior).
- Compiler flags a few `1 << n` results widened to 64 bits (C4334: `fcoll_kDOP.cpp`,
  `GeneralCorrosiveGame.cpp`, `SpaceDock.cpp`, `fEventListener.h`, `ColiseumMiniGame.cpp`).
  Behavior is the same as the original 32-bit shift, but it may be a latent bug.
- 32-bit only: 150+ inline-asm blocks, MASM collision code, x86 Bink import lib.

## Legal

The source and the game data are proprietary. This repository is kept private and does
not contain any retail game data (`gamedata/` and disc images are git-ignored).
