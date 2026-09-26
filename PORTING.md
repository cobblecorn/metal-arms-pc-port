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

    ma_port -data <dir> [-mst <file>] [-res WxH] [-fullscreen] [-level <world> | -mission <world> | -world-only <world>] [-log <file>] [-shots <dir>] [-shot-every <frames>] [-mouse-sensitivity <n>] [-save-dir <dir>]

`-level <world>` starts the generic debug level path using `Level01` configuration.
`-mission <world>` resolves a registered single-player mission and uses its own configuration,
material table and normal level-loading path. Unknown/unregistered worlds fail explicitly.
`-world-only <world>` loads and converts the WLD resource, then exits before localized setup
and gameplay entity creation. These three launch modes are mutually exclusive.

For the mission path with engine captures:

    ma_port -data gamedata/files -mission wecdsneak01 -log build/logs/mission.log -shots build/shots-mission -shot-every 1800

`-mission wecdsneak01` loads Night Sneak's configuration, attaches and initializes its eight
retail scripts (no unresolved natives, no script errors), and reaches gameplay. Objectives,
level transitions and saves have not been verified; this is not yet a complete campaign launch.

`tools/mst_list.py` lists/extracts a `.mst` master file (GameCube byte order).
`tools/gamedata_dump.py` inspects extracted binary `.csv` game-data tables as indexed JSON;
write reports under ignored `build/` because they contain retail asset values.
`tools/dol_vocab.py` recovers the retail game-data vocabularies (field type, conversion,
size and clamp range) from `gamedata/sys/main.dol` by following its `FGameDataMap_t` tables,
e.g. `python tools/dol_vocab.py gamedata/sys/main.dol LaserL1`. `tools/dol_xref.py` finds and
lists retail PowerPC code that references a data address (a minimal decoder, enough to see
which structure offsets feed which runtime fields). Use them to resolve schema drift from
retail evidence instead of guessing; keep their output under `build/`.

`tools/syntax_check.py` syntax-checks the C/C++ sources without MSVC, for sessions that cannot
build (Linux/cloud): clang against MinGW-w64 headers with flags that mimic the MSVC build. It
catches type errors and wrong API use, not everything MSVC would; it never links or runs.
`python3 tools/syntax_check.py --changed HEAD` checks the C/C++ files changed since HEAD.

`tools/mathdiff/mathdiff.py` runs the GC-layout math (`dx/fdx8gcmath_*.inl`, the scalar code this
build uses) and the shipped SSE math (`dx/fdx8math_*.inl`) on the same inputs as 32-bit Linux programs
and reports methods whose results differ. Known differences are documented in the tool; each was
checked against the retail GameCube code (`gc/fGCmath_*.inl`). Run it after changing the GC-layout math.

## Saves

Player profiles are saved in `%APPDATA%\Metal Arms PC Port\Saves` (override with `-save-dir <dir>`
or `MA_PORT_SAVE_DIR`); the log names the directory at startup. Each profile is one file,
`profile-<hex>.sav`, whose name is the profile name's UTF-16 code units in hex (profile names may
contain characters Windows file names can't, or differ only by case). Saves are written to a
`.tmp` copy that replaces the profile once complete. Profiles that earlier builds saved as
`profile-<name>` in the working directory are copied into the save directory at startup; the
originals are left in place. In-level checkpoints are memory-only, as on the consoles.

## Desktop controls

| Control | Action |
|---|---|
| WASD | Move (diagonal speed is normalized) |
| Mouse movement | Captures the mouse for raw mouse look (during gameplay, while the game has focus) |
| F1 | Turn automatic mouse look off (free cursor) / back on |
| Mouse / arrow keys | Look / turn |
| Space | Jump |
| E | Action (traced in source; one run was reported to open the weapons menu, see below) |
| F | Melee |
| Left / right mouse button | Primary / secondary fire (requires a supported weapon) |
| Q | Hold for the primary weapons list (traced; one run was reported to open throwables) |
| R | Hold for the secondary (throwables) list (traced) |
| 1 / 2 / 3 / 4 | Quick-select up / right / down / left |
| Escape / Enter | Pause; Escape also releases the mouse |
| Alt-Tab | Releases the mouse; moving it over the game again recaptures it |
| Alt-F4 | Close the game |

The user confirmed responsive mouse look and reported that some weapons appear to work.
Traced in source (keys → Fang pad inputs in `port/pc_input.cpp` → the single `MAIN1` control map
in `gamepad.cpp`, used for both the Xbox and GameCube layouts → `Hud2.cpp`): Space → `CROSS_BOTTOM` →
jump; E → `CROSS_TOP` → action; Q → `CROSS_RIGHT` → select primary (the HUD opens hand 0, weapons);
R → `CROSS_LEFT` → select secondary (hand 1, throwables); F → GameCube Z → melee. No other control map is
compiled (`Main2`-`Main4` are commented out and `player.cpp` clamps the profile's controller config to
`MAIN1`). An earlier run was reported as Q = throwables and E = weapons, which this trace can't explain:
please re-check by holding Q, then R, then E, and note which list appears. With a throwable equipped
and ammo available, right mouse maps to secondary fire and starts the throw. The HUD selection code
accepts W/S to scroll while a selection menu is held open; releasing the menu button equips the
selection. Throwable behavior has not yet been confirmed interactively.

Mouse look uses raw relative motion, applied as angular displacement without the controller's
acceleration curve or turn-speed cap. `-mouse-sensitivity 0.1` is the default, in degrees per
mouse count, before the game's look-sensitivity multiplier. Use `0.05` for half that speed.
Moving or clicking the mouse over the game during gameplay captures it; menus, Escape and losing
focus (Alt-Tab) release it, and the next movement over the game recaptures it. F1 switches
automatic capture off for a free cursor, and on again. All inputs return to neutral when the game
loses focus. Desktop defaults to non-inverted look;
loaded profiles retain their own setting.

Target assistance (reticle snapping, aim biasing, shot focusing) is tuned for sticks. By
default (`-aim-assist auto`) it is suspended while you aim with captured mouse look and returns
when the right stick aims; `-aim-assist on|off` (or `MA_PORT_AIM_ASSIST`) forces it.

XInput controllers can connect after launch. `-input-layout shared` (default) puts the keyboard/mouse
and controller 1 on port 1 and controllers 2-4 on ports 2-4. `-input-layout separate` keeps the
keyboard/mouse alone on port 1 and puts controllers 1-3 on ports 2-4, so a keyboard player and pad
players are separate players (local co-op). `MA_PORT_INPUT_LAYOUT` is the environment equivalent.
The adapter intends A for jump, Y for action, B/X for weapon selection, triggers for fire,
and RB/right-stick click for melee (the same control map as the keys above). Controller mapping has automated coverage; physical controller
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
- [x] Load retail scripts (`.sma`). All 393 are compact little-endian AMX (file version 5,
      AMX version 4) that the bundled interpreter runs without byte swapping. The loader
      validates headers and symbol tables before allocating. Across the corpus, 10 natives
      are missing from this source (`Checkpoint_Save2`, `Audio_Play2DSoundEx`,
      `Bot_IsPosessed`, `Bot_IsRecruited`, `Bot_Recruit`, `Console_Enable`, `FX_StompRing`,
      `Misc_GetDifficulty`, `Misc_GetValue`, `Misc_SetValue`); none is used by Night Sneak.
      Scripts importing them still load, the missing names are logged, and a call to one
      aborts that script callback with `AMX_ERR_NOTFOUND` instead of calling a NULL pointer.
- [x] Implement the 10 missing natives from their retail code in main.dol. All 393 retail
      scripts now bind every native. `Bot_LoadTalk` accepts the retail 5-argument form.
- [ ] Verify mission objectives, level transitions and saves.
- [x] Bink cutscenes play full screen (4:3, stretched with linear filtering) with their own
      audio. The retail movies use the `GC_` prefix.
- [ ] Game audio (GC MusyX sound banks / DSP-ADPCM streams). Still disabled.
- [ ] Vehicle controls: the RAT in `WEWHchase01` does not respond to WASD for driving or the
      turret, and mouse motion appears to steer it (user report).
- [x] Keyboard controls and direct raw mouse look, confirmed interactively in `wecdsneak01`.
      XInput mapping includes deadzones, separate triggers, focus handling, and hotplug support.
- [ ] Verify physical XInput controllers, vehicle-specific mouse aiming, and rumble.
- [ ] Save games. The front end uses the Xbox flow with the PC's single "hard disk" device
      (`dx/fdx8storage.cpp`). That backend was rewritten: per-user save directory, safe file names,
      atomic writes, and `ValidateProfile` now reports a missing profile (it always said "valid",
      so `CPlayerProfile::IsOnCard()` could never fail). Syntax-checked only; creating, loading,
      renaming, deleting and in-game saving still need a run.
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
- Retail `w_laser.csv` (73 fields; a charge/burst redesign) is mapped onto the source's
  primary-fire laser from retail evidence: the `LaserL1` vocabulary in `main.dol` and the retail
  `CWeaponLaser` InitSystem/ClassHierarchyBuild/fire code. Mesh, muzzle bone, tracer texture
  and RGBA color, clip (reserve forced infinite, as retail does), fire rate, tracer
  speed/length/width/range, target-assist range, cull distance, recoil, damage profile and
  decal come from the retail fields. The recharge rate is inferred from the retail refill step
  (fields 39/40). Retail L3 fires tracers, so the WINGC build uses the tracer path instead of
  the source's continuous L3 beam. Not implemented: the charged burst (fields 19-28, 60-64,
  67-72), particle muzzle/smoke effects (47-58) and sound groups (65-70).
- Retail `afDataTable` (the float table vocabulary min/max indices point into) inserts `4.0`
  after `3.0`, so retail indices from 12 up are one higher than this source's
  `F32_DATATABLE_*`. Data files are unaffected; compare retail vocabularies with
  `tools/dol_vocab.py`, which uses the DOL's own table.
- Retail GameCube AI graphs have 5 edge slots per vertex. `InitEdgeToPoiLookup` skipped free
  vertices with a hard-coded 6, misaligning (and overrunning) the edge-to-POI table and
  asserting thousands of times during mission AI setup; it now uses the slot constant.
- Grunts carrying lasers previously failed to build, so mission scripts could not find them
  (`E_Find() : Could not find entity named 'added_grunt3c'`). Other retail damage profiles
  (`Debris`, `SentinelCannon`, `SpewSmoke`, `ScoutSiren`) are still missing from the loaded
  damage tables.
- The blaster
  loader now reads its 45-field layout and handles the three available player variants without
  initializing absent military variants. Its two extra numeric fields are retained but their
  behavior is not implemented. Blaster resource creation, firing and upgrades need runtime
  confirmation. Other item tables still report unsupported retail collectable names.
- Retail inventory startup slots now follow recognized items when unsupported names are skipped;
  array/count validation prevents invalid table indices and empty-inventory underflow.
- Retail data layouts recovered from main.dol and now read: goodies.csv (collectables; every
  pickup was unknown before), materials.csv (surface class field), deb_group.csv (debris; the
  random-orientation flag is gone and priority widened), sw_wall.csv (wall sentry; smoke fields
  not implemented). `fgamedata_GetTableDataRemapped` reads a source vocabulary from mapped retail
  fields. Retail barter EUK kits and battery upgrades have no source collectable types yet.
- The retail particle keyframe's extra 8 bytes sit before `NumPerBurst`, not at the end.
  Removing the wrong bytes had corrupted counts, colors and timings in 375 of 377 effects and
  produced out-of-range bubble chances that asserted every frame (a frozen-looking mines level).
- Bone-mask index tables must end with 255. Five did not (rat gun x2, Glitch fire-1 lower,
  Predator left arm, Corrosive summer) and the reader ran into adjacent data; the rat gun
  crashed the chase level.
- The retail elite guard has seven dismemberable limbs (the source had only the head).
- The retail zombie boss level looks up only `ZombieBotBoss`; the cage/Mozer lose rule and the
  `mozer01` lookup are skipped in the GC build.
- Assert and `/RTC` reports are rate-limited (first 10, then 100, 1000, ...) with a stack on
  the first occurrence, and world entities that fail to build are logged by name.
- Known: `CFQuatTang3::Calculate` (scripted carts in `WEDTtown_01`) unitizes a zero XZ tangent
  and produces NaNs that reach collision; the same math would do so on the GameCube, so the
  input path (path tangent) needs checking. `-level we01multi01`-era notes predate these fixes.
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
  treat a firing assert as a real bug to fix, not background noise. The first occurrence of
  each distinct assert also logs a symbolized call stack, so a repeating assert names its caller.
- Found via the above: `CBotGlitch::_InitInventory()` was passing a retail inventory's current-
  weapon slot straight to `_ChangeWeaponIndex()` with no bounds check against this source's
  reduced weapon set (see the laser/blaster schema note above) - a real out-of-bounds array read
  that the original `FASSERT` alone did not prevent (`FASSERT` compiles out entirely in
  Release/Production builds, so this was always a live bug, not just a debug-build nuisance).
  Fixed at both the call site (fall back to slot 0 when the retail slot is out of range) and in
  `_ChangeWeaponIndex()` itself (bounds-checked no-op instead of undefined behavior).
- Script event masks are `u64`, but events were set and tested with a 32-bit `1 << n`. For events
  32-63 the GameCube's PowerPC gives 0 (never delivered) while x86 wraps the count (event 40 fired as
  event 8; event 63 matched events 31-63). `fevent_Bit()` (`FEventListener.h`) now gives the GameCube
  result everywhere (`FScriptSystem.cpp`, `FEventListener.h`, `GeneralCorrosiveGame.cpp`,
  `SpaceDock.cpp`, `ColiseumMiniGame.cpp`). The remaining C4334 warnings (`fcoll_kDOP.cpp`) shift by a
  kDOP vertex index, at most 23, so they are harmless.
- 32-bit only: 150+ inline-asm blocks, MASM collision code, x86 Bink import lib.

## Legal

The source and the game data are proprietary. This repository is kept private and does
not contain any retail game data (`gamedata/` and disc images are git-ignored).
