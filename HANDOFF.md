# HANDOFF - Metal Arms Windows port (read this first)

Written at the end of session 1. Everything below was verified in that session unless marked **(unverified)**.

## 0. The goal

User goal (set via `/goal`): **"metal arms windows"** = a working native Windows build of *Metal Arms:
Glitch in the System*, running from the user's retail **GameCube** disc data. A session Stop hook with
that condition may still be active; it is **not met yet**. The game boots but cannot load any data.

The user is nearly out of usage: be efficient, batch tool calls, avoid re-deriving what is here.

## 1. Where everything is

Project root: `D:\Documents\metal arms source port`  (git repo, branch **`x86-port`**, `main` = pristine drop)

| Path | What |
|---|---|
| `ma/` | The original Swingin' Ape source drop (Fang2 engine in `ma/Lib/Fang2`, game in `ma/App/ma`, tools in `ma/App/*`). Edited minimally (13 files, +241/-62 vs `main`). |
| `CMakeLists.txt`, `cmake/sources_*.cmake` | Build. Source lists generated from the original `.vcproj` by `tools/gen_sources.py`. |
| `port/main_win.cpp` | New plain-Win32 entry point (replaces MFC launcher). |
| `port/compat/` | `d3d8.h`+`d3d8_compat.cpp` (D3D8 API on top of D3D9), `d3dx8.h`, `d3dx8tex.h`, `xgraphics.h` (Morton swizzle helpers), `sas_user.h`. |
| `port/screenshot_port.cpp` | No-op stub for the old screenshot module. |
| `tools/mst_list.py` | Parse/extract the GC `.mst` master file. `build_shaders.py`: assembles `.nvv/.nvp` -> headers. `gen_sources.py`. |
| `gamedata/` (git-ignored) | `sys/` (main.dol, apploader), `files/` (**`mettlearms_gc.mst`** 342 MB, `Movies/*.bik` x39, `*.wvs` x166, `.str`, `.rel`), `mst/` (12,048 loose files extracted from the .mst), `mst_index.csv`. |
| `Metal Arms - Glitch in the System (USA).rvz` (git-ignored) | Source disc image (GameCube, ID `GM5E7D`, rev 0). |
| `PORTING.md` | Reviewer-facing status/known issues (keep in sync). |
| `build/` (git-ignored) | Build tree; logs in `build/logs/`; generated shaders in `build/shaders/`. |

Git: local only, **never push** (proprietary source + retail data; user wants an Opus cloud review later via
`/code-review ultra` on `x86-port` vs `main` - the user launches it, you cannot). Repo-local git identity is set.
Collaborative commits used a `Co-Authored-By:` trailer when recording a collaborator.
Commits so far: `bb47474` baseline (main) -> `a797c2d` gitignore+mst tool -> `280589e` engine builds ->
`388f7fe` links/runs -> `6368b7f` PORTING.md. Tree was clean after that; this file is the next commit.

## 2. Build / run / debug

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32          # 32-bit ONLY (inline asm, x86 Bink)
    cmake --build build --config Debug --target ma_port -- -nologo -v:m > build/logs/x.log 2>&1
    ./build/Debug/ma_port.exe -log build/logs/run.log                 # run from repo root (default -data gamedata\files)

Options: `-data <dir> -mst <file> -res WxH -fullscreen -log <file>`. Full rebuild takes a few minutes (game = 230 files).
Triage errors with: `grep -E 'error [A-Z]+[0-9]+' log | sed ...` grouped by code/file; that workflow was very effective.

Gotchas that cost time:
- **Git Bash mangles `/flag` args** (`/nologo` -> path). Use `-nologo -v:m`. Prefer Bash for tools; PowerShell wraps native stderr as errors.
- Source files are **CRLF**. Patch with binary-safe Python (read bytes, normalize, replace, restore CRLF) or the Edit tool.
  Inline `python - <<EOF` with backslash-heavy regexes got mangled twice: use the Edit/Write tools for those.
- `timeout N ./ma_port.exe` for runs; it opens a real window (1280x960 windowed) and exits when the game exits.
- Toolchain: VS 2022 Community (14.44) x86 cl/ml, CMake 4.3, Python 3.12 (also used for tools), Windows SDK 10.0.26100. No DirectX SDK, no MFC.

## 3. What is DONE (all verified)

1. Fang2 engine + SmallAMX + all game/AI code compile and link -> `build/Debug/ma_port.exe`.
2. D3D8->D3D9 shim works: real device/mode enumeration on the user's RTX 5070 Ti.
3. Shaders: 122/122 `.nvv/.nvp/.vsh` assemble via `D3DAssemble` (SKIP_VALIDATION; vs.1.0->1.1; `oFog.x/oPts.x` unmasked).
4. MFC removed: `fdx8loop.cpp` `CGameThread` is now a Win32 thread class (waits for msg queue creation).
5. Built in **`_FANGDEF_WINGC`** mode (GC struct alignment, no SSE) so in-file struct layouts match GC data. This mode was
   tools-only before; added 12 missing scalar math funcs to `dx/fdx8gcmath_{vec,quat,mtx}.inl` (from `gc/fGCmath_*.inl`).
6. Run result today: engine boots, video picked, then:
   `ffile _ReadMasterDir(): Unrecognized signature in master file ...` -> `You must select a valid master file before you can run.` -> clean exit(0).
   (It fell back to "Directory Mode" and found no loose files.)

## 4. THE NEXT WALL: teach the PC runtime to read GameCube data

### 4a. Master file (start here; small)
`ma/Lib/Fang2/ffile.cpp` `_ReadMasterDir()` (~line 1163). Facts about the retail `.mst` (parsed & verified by `tools/mst_list.py`):
- Big-endian. Header = 108 bytes: `FVersionHeader_t{sig,version}`, then u32 nBytesInFile, nNumEntries(12048), nNumFree(4342), nNumSupport(3117),
  nNumFreeSupport, nDataOffset(0xE0800), 10 compiler-version u32s, 36 reserved bytes. Entries = 36 bytes each (name[16], u16 flags, u16 pad,
  u32 start, u32 size, u32 mtime, u32 crc). Entry table starts at file offset 108. Files are 2048-aligned. Names lowercase, <=15 chars.
- The signature "GNAF" bytes == `FVERSION_FILE_SIGNATURE` read big-endian. On PC: read header, call `Header.ChangeEndian()`, then compare.
- Version word `0x18010800` = `FDATA_PRJFILE_GC_VERSION` (platform GC|TOOLS, 1.8.0). Under WIN the code checks `FDATA_PRJFILE_XB_VERSION`; add a WINGC branch.
- Entries: `ChangeEndian()` each (nStartOffset, nNumBytes are u32). Compiler-version check must be **relaxed** (see 4b) or the retail file is rejected.
- Keep everything after that as is (CRC name sort etc.). Reads of file *contents* still go through `ffile_*` and return raw big-endian bytes.

### 4b. Version drift (real risk)
Retail data was built by newer tools than the source snapshot (`fdata.h` constants):
tga 7 vs 6 | ape 0x39 vs 0x37 | mtx 0x15 vs 0x14 | csv 0xC vs 0xB | fnt 0xB vs 0xA | sma 4 vs 3 | gt 0xC vs 0xA | wvb 4 vs 3 | fpr 4 vs 3 | cam 1 vs 0.
The runtime only enforces equality in `_ReadMasterDir` (tools use them to decide recompiles). Accept the retail values with a warning, then
**diff actual layouts** as each type is implemented; `gamedata/sys/main.dol` (retail GC binary) is ground truth if a layout differs from the source.

### 4c. Contents of the master file (counts): 
tga 6237 (124.9 MB) | wld 59 (levels, 97 MB) | rdg 166 | ape 2231 (38.5 MB) | mtx 1069 (animations, 25 MB) | gt 71 | csv 1291 | sma 393 | fpr 377 | cam 38 | sfb 106 | fnt 10.
(`.wvs` streams + Bink `.bik` are separate files on disc, not in the .mst.)

### 4d. Architecture for conversion (recommended)
All assets go through the resource layer: `fresload_Load(type, name)` (`Lib/Fang2/fresload.cpp`) reads the file, then calls the type's registered
`pFcnCreate(hRes, pLoadedBase, nLoadedBytes, name)`. Handlers (`fresload_RegisterHandler`), by ext:
`wld` fworld.cpp | `mtx` fanim.cpp | `ape` **dx/fdx8mesh.cpp** (GC twin: gc/fGCmesh.cpp) | `csv` fgamedata.cpp | `tga` **dx/fdx8tex.cpp** (GC twin: gc/fGCtex.cpp) |
`fpr` fparticle.cpp | `cam` fcamanim.cpp | `sfb` fsndfx.cpp | `wvb`/`rdg` audio (dx/fdx8audio.cpp; GC twin gc/fgcaudio.cpp, MusyX).
Not yet located (check): loaders for `.sma` (SmallAMX bytecode), `.gt`, `.fnt` (ftext.cpp?).

Plan: add a central hook (new file, e.g. `port/gcdata.cpp` or `Lib/Fang2/fgcdata.cpp`) called from `fresload` right after the file bytes are read and
before the handler runs: `gcdata_Convert(resType, buf, nBytes)`. Per type:
1. **Easy, in-place byte-swap** using the structs' existing `ChangeEndian()` (they are their own inverse). The *tool-side writers* show the exact
   swap sequence that produced the GC files: `ma/App/pasm2/*.cpp` (e.g. `ApeToWorldFile.cpp:~1804`, `CreateGCTgaFile.cpp`),
   `ma/App/*/MasterFileCompile.cpp`, `fdata.h` (all `FData*` structs w/ `ChangeEndian`). Do csv/gt/sma/mtx/fnt/cam/fpr first; they are platform-independent.
2. **Textures (.tga)**: GC formats (GX 4x4/8x4 tiled I4/I8/IA4/IA8/RGB565/RGB5A3/RGBA8/CMPR + palettes). Decode/untile -> D3D formats
   (A8R8G8B8 is the safe universal target). Reference: `gc/fGCtex.cpp`, `pasm2/CreateGCTgaFile.cpp`. DX side to feed: `dx/fdx8tex.cpp`
   (it already has a PC path copying pitched pixels; note `_aTexelInfoTable`).
3. **Meshes/worlds (.ape/.wld)**: GC stores GX display lists + separate position/normal/color/UV arrays; DX side wants vertex buffers/index buffers
   (`dx/fdx8mesh.cpp`, `fdx8vb.cpp`, `fdx8load.cpp`). Need a GC->DX vertex adapter (decode display lists into triangle lists, rebuild
   `FDX8` mesh structs). Reference: `gc/fGCmesh.cpp`, `gc/fgcDisplayList.cpp`, `gc/fGCload.cpp`, and the DX writers in pasm2 for the target layout.
   This is the biggest item; do it after textures. Watch collision data (`fmesh_coll`, `fdx8mesh_coll.cpp` uses SIMD packets on DX).
4. Pointer fixups inside blobs are offset->pointer tables (`nOffsetToFixupTable/nNumFixups` in fdata.h); pointers are 32-bit so fine on x86.

### 4e. Do this to reach pixels sooner
- Pass `bInstallAudio = FALSE` in `port/main_win.cpp` until audio is handled (GC audio = MusyX `.rdg/.sfb` + DSP-ADPCM `.wvs`; PC audio code
  is DirectSound + ACM for `.wvb` and will not read it). Expect to write a MusyX/DSP-ADPCM decoder later (see `App/GCSndBankUtil/MusyXData.cpp`).
- Read `ma/App/ma/gc/main.cpp` (GC boot config: paths/heap/flags) and mirror relevant `Fang_ConfigDefs`; also `game.cpp/gameloop.cpp` for the first
  resources requested (fonts, front-end menu, `wpr_*.cpp` screens) so you convert only what boot needs first, in the order it asks.
- A good first milestone: front-end/menu renders (fonts + textures + a mesh), then a level (`we01multi01.wld` is a small multiplayer world).

## 5. Other TODO after data loads
- **Input**: `pauInputEmulationMap` is NULL; keyboard/mouse->gamepad emulation from old `win/InputEmulation.cpp` + MFC `Settings` not ported. `dx/fdx8padio.cpp` uses DirectInput8 pads. Add keyboard/mouse + XInput mapping.
- **Save games**: `nMemCardUsageFlags = NONE`; memcard layer needs a file-backed implementation (`gamesave.cpp`, `save.cpp`).
- **Movies**: Bink 1 (`BIKi`, 512x448) via the x86 `binkw32.dll` (copied next to the exe) - `dx/fdx8movie2.cpp`; untested. GC .bik should be standard.
- **Sources excluded by the original PC project** (still not compiled): `BlinkShell.cpp DestructEntity.cpp botsniper.cpp botsniper_data.cpp eproj_linear.cpp grapple.cpp user_elliott.cpp`.
  If retail levels/scripts reference sniper/grapple/blink-shell content, add them (may need fixes).
- `screenshot_port.cpp` is a no-op stub. Video is windowed 1280x960 default; the game is 4:3 (Xbox was 640x480).
- Later: 64-bit (150+ inline-asm blocks), widescreen, XInput rumble, replace Bink with FFmpeg.

## 6. Known caveats / review flags (also in PORTING.md)
- `D3DRS_ZBIAS`->`DEPTHBIAS` scale is a guess (`port/compat/d3d8_compat.cpp`, `-Value*1.5e-5f`); may z-fight decals.
- D3D8-only render states silently ignored; `D3DXLoadSurfaceFromMemory` only supports same-format / 32-bit copies (will need real conversion for GC formats).
- C4334 warnings (`1<<n` widened to 64-bit) at `fcoll_kDOP.cpp:1458/1462`, `GeneralCorrosiveGame.cpp:955-997`, `SpaceDock.cpp:142`, `fEventListener.h:53`, `ColiseumMiniGame.cpp:2669`: same behaviour as original, possible latent bug.
- Vertex declaration translation maps D3D8 input-register numbers to D3D9 semantics by the fixed vs_1_x table (v0 POS, v1 BLENDWEIGHT, v2 BLENDINDICES, v3 NORMAL, v4 PSIZE, v5 COLOR0, v6 COLOR1, v7-14 TEXCOORD0-7, v15 POSITION1). Validate when real geometry draws; if a shader misreads inputs, look here first.
- Pixel shaders are ps_1_1 via D3D9; fine on the RTX card but ps_1_x is emulated by the driver.

## 7. Suggested first prompt for the next session

> Read HANDOFF.md and PORTING.md in `D:\Documents\metal arms source port` (branch `x86-port`). Continue the goal "metal arms windows":
> implement section 4a (GC master file loading in `ffile.cpp` `_ReadMasterDir`), then the `fresload` conversion hook (4d) starting with
> csv/mtx/sma/gt/fnt, then textures, then meshes/worlds. Build with the commands in section 2, run `ma_port.exe -log build/logs/run.log` after each
> step, commit small steps on `x86-port`, and keep PORTING.md current. Set `bInstallAudio = FALSE` until audio is handled.
