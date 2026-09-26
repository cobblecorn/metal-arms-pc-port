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

    ma_port -data <dir> [-mst <file>] [-res WxH] [-fullscreen] [-log <file>]

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
| Language | C++ rule changes since 2003: anonymous-union members made implicit copies deleted (`CFSphere`, `CFRect2D`), implicit-int declarations, `Lock(void**)`, modern MASM operand sizes. |

## Status

- [x] Engine (Fang2), scripting VM, game logic and AI compile and link.
- [x] Executable starts; engine boots; D3D device/mode enumeration works on a real GPU.
- [ ] **Load the GameCube master file.** The runtime rejects it (big-endian header, GC
      platform flag). Every asset type inside is GC-format and needs a loader/converter:
      textures (GX tiled/CMPR), meshes (GX display lists -> vertex buffers), animations
      (`.mtx`), world files (`.wld`), tables (`.csv`/`.gt`), scripts (`.sma`), fonts, particles.
- [ ] Audio (GC MusyX / DSP-ADPCM streams) and Bink video hookup.
- [ ] Input: keyboard/mouse and XInput mapping onto the game's pad layer.
- [ ] Save games (memory-card layer -> files).
- [ ] Screenshot capture (currently a stub, `port/screenshot_port.cpp`).

## Known issues / things worth a reviewer's eye

- `SetRenderState(D3DRS_ZBIAS)` is mapped to `D3DRS_DEPTHBIAS` with a guessed scale
  (`d3d8_compat.cpp`); decals/coplanar geometry may z-fight until tuned.
- `D3DXLoadSurfaceFromMemory` supports only same-format (and 32-bit interchange) copies.
- D3D8-only render states with no D3D9 equivalent are silently ignored.
- Retail data was built with newer tool versions than this source snapshot (e.g. mesh
  compiler 0x39 vs 0x37 in `fdata.h`); the runtime doesn't enforce these, but layouts
  may differ slightly.
- Compiler flags a few `1 << n` results widened to 64 bits (C4334: `fcoll_kDOP.cpp`,
  `GeneralCorrosiveGame.cpp`, `SpaceDock.cpp`, `fEventListener.h`, `ColiseumMiniGame.cpp`).
  Behavior is the same as the original 32-bit shift, but it may be a latent bug.
- 32-bit only: 150+ inline-asm blocks, MASM collision code, x86 Bink import lib.

## Legal

The source and the game data are proprietary. This repository is kept private and does
not contain any retail game data (`gamedata/` and disc images are git-ignored).
