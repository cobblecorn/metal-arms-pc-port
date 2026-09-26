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

    ma_port -data <dir> [-mst <file>] [-res WxH] [-fullscreen] [-level <world>] [-world-only <world>] [-log <file>]

`-level <world>` starts the normal generic level path. `-world-only <world>` loads and converts
the WLD resource, then exits before localized setup and gameplay entity creation.

`tools/mst_list.py` lists/extracts a `.mst` master file (GameCube byte order).

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
- [x] Convert static, unskinned GameCube mesh display lists to D3D vertex/index buffers.
      Startup has converted weapon and effect meshes through `gf_emp02.ape`.
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
- [ ] Extend mesh support to skinned/streaming display lists and translate GameCube
      collision trees. The current adapter drops mesh collision data.
- [ ] Convert scripts (`.sma`), fonts, and other runtime resources.
- [ ] Audio (GC MusyX / DSP-ADPCM streams) and Bink video hookup.
- [ ] Input: keyboard/mouse and XInput mapping onto the game's pad layer. DirectInput gamepad
      enumeration works; remapping is disabled when no device/map is configured.
- [ ] Save games (memory-card layer -> files).
- [ ] Screenshot capture (currently a stub, `port/screenshot_port.cpp`).

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
- Retail `w_laser.csv` and `w_blaster.csv` use newer schemas than the source vocabularies. The
  Windows GC-data build leaves those systems unavailable and skips those entity instances; the
  player starts without a supported secondary weapon. Other item tables still report unsupported
  retail collectable names.
- The wrapper system accepts trailing retail phrases and reads its six-field screen-name stride,
  selecting the Xbox UI entries and ignoring the two screens this source does not define.
- A regular `-level we01multi01` launch now passes wrapper and world setup and reaches
  `END OF BOOTUP`. Audio remains disabled: `fsndfx.cpp` skips parsing GC SFX banks, so sound groups
  have no loaded sound definitions.
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
- Compiler flags a few `1 << n` results widened to 64 bits (C4334: `fcoll_kDOP.cpp`,
  `GeneralCorrosiveGame.cpp`, `SpaceDock.cpp`, `fEventListener.h`, `ColiseumMiniGame.cpp`).
  Behavior is the same as the original 32-bit shift, but it may be a latent bug.
- 32-bit only: 150+ inline-asm blocks, MASM collision code, x86 Bink import lib.

## Legal

The source and the game data are proprietary. This repository is kept private and does
not contain any retail game data (`gamedata/` and disc images are git-ignored).
