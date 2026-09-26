# HANDOFF - Metal Arms Windows port (read this first)

Written as a chronological handoff. Older addenda describe the state at that time; see the last addendum for the current state.

## 0. The goal

User goal (set via `/goal`): **"metal arms windows"** = a working native Windows build of *Metal Arms:
Glitch in the System*, running from the user's retail **GameCube** disc data. A session Stop hook with
that condition may still be active; it is **not met yet**. Current progress and remaining format gaps
are recorded in the latest addendum and `PORTING.md`.

The user is nearly out of usage: be efficient, batch tool calls, avoid re-deriving what is here.

## 1. Where everything is

Project root: `D:\Documents\metal arms source port`  (git repo, branch **`x86-port`**, `main` = pristine drop)

| Path | What |
|---|---|
| `ma/` | The original Swingin' Ape source drop (Fang2 engine in `ma/Lib/Fang2`, game in `ma/App/ma`, tools in `ma/App/*`), with focused Win32 port changes on `x86-port`. |
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
Earlier commits established the ignored data workflow, engine build, executable, and initial port notes;
continue with small local commits on `x86-port` using the required trailer.

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

## 7. Suggested first prompt from the initial handoff (superseded)

> Read HANDOFF.md and PORTING.md in `D:\Documents\metal arms source port` (branch `x86-port`). Continue the goal "metal arms windows":
> implement section 4a (GC master file loading in `ffile.cpp` `_ReadMasterDir`), then the `fresload` conversion hook (4d) starting with
> csv/mtx/sma/gt/fnt, then textures, then meshes/worlds. Build with the commands in section 2, run `ma_port.exe -log build/logs/run.log` after each
> step, commit small steps on `x86-port`, and keep PORTING.md current. Set `bInstallAudio = FALSE` until audio is handled.

---

## 8. ADDENDUM (end of session 1, after the sections above were written)

**Done since section 4a was written (committed):**
- `ffile.cpp` `_ReadMasterDir()` now reads the GameCube master file under `FANG_WINGC`: swaps header + directory entries, accepts
  `FDATA_PRJFILE_GC_VERSION`, and only *warns* about the newer retail data-compiler versions. Verified: the log now prints
  `NOTE: ... built by newer data compilers ... (file/source) tga 7/6 ape 57/55 mtx 21/20 csv 12/11 fnt 11/10 sma 4/3 gt 12/10 wvb 4/3 fpr 4/3 cam 1/0`
  and the game proceeds past the old "Unrecognized signature / must select a valid master file" wall.
- `port/main_win.cpp`: added a crash handler (`SetUnhandledExceptionFilter`) that logs the exception and a **symbolized stack** (uses the .pdb; links dbghelp).
- `port/compat/d3d8_compat.cpp`: `CreateVertexShader/CreatePixelShader` now log the HRESULT on failure.

**Current run result (`./build/Debug/ma_port.exe -log build/logs/run.log > build/logs/run.out 2>&1`):**
1. Master file loads. Engine starts, D3D device created on the RTX 5070 Ti.
2. **Every `CreateVertexShader` fails with `0x8876086c` (D3DERR_INVALIDCALL)**, version token `0xfffe0101` (vs_1_1), even trivial pass-through shaders.
   Device is created with `D3DCREATE_HARDWARE_VERTEXPROCESSING`. So the cause is either how the shaders were assembled or a device-level issue.
3. Then the process **segfaults** (exit 139). The new crash handler should print `*** CRASH` + stack, but the output was flooded by the shader errors and
   I did not get to read it: run and `grep -v _SetupVertexShaders build/logs/run.out | grep -A40 CRASH`.

**Immediate next steps:**
- Diagnose (2). A standalone probe is at `build/probe/probe.cpp` (build/ is git-ignored; recreate if missing). It creates a HAL device with HW/SW/MIXED vertex
  processing, prints caps (VS version, MaxVertexShaderConst), and tries: the generated `dwFdx8PassThru_1tcVertexShader`, a trivial `vs.1.1` assembled with and
  without `D3DCOMPILE_SKIP_VALIDATION`. It did **not compile yet**: `D3DAssemble` was "not found" (fix: check `d3dcompiler.h` include / declare via
  `LoadLibrary("d3dcompiler_47.dll")`+`GetProcAddress`, or use the same ctypes path as `tools/build_shaders.py`). Build/run it with `cmd //c build\probe\build.bat`
  (the .bat calls vcvarsall x86; ignore the harmless `vswhere` message).
  Hypotheses: (a) D3D9 runtime rejects `SKIP_VALIDATION` output (try assembling with validation ON and fixing/predeclaring inputs, or emit a `dcl`-free form);
  (b) constant register range: check `CV_*` max in `fdx8vshader_const.h` vs `MaxVertexShaderConst`; (c) vertex declaration/ shader mismatch.
  Fallback: translate the vs_1_1 assembly to HLSL (vs_2_0/3_0) at build time, or use legacy `D3DXAssembleShader` if `d3dx9_43.dll` is installed.
- Then read the crash stack (3) and fix; then continue with section 4d (per-type GC data conversion via the `fresload` hook).

**Tooling gotchas learned late:**
- The harness collapses backslash escapes in tool inputs: a Python string `"\n"` became a real newline inside C literals. For C string edits use the
  Edit tool, or build backslashes with `chr(92)`. Python `open(p,"w")` on Windows writes CRLF; read/write with `newline=""` to control it.
- Editing CRLF files with the Edit tool can leave mixed endings: normalize afterwards (read bytes, `\r\n`->`\n`->`\r\n`).

## 9. ADDENDUM (session 2, 2026-09-26; historical)

Work is still on local branch `x86-port`. This session advanced startup through master-file reading,
texture creation, CSV table loading, and sound-group setup. The current blocking point is the first
GC mesh (`gpdmwpnunkn.ape`): `fdx8mesh.cpp` passes its GC-format payload to `fdx8load_Create`, which
expects an `FDX8Mesh_t` and crashes while reading `pMeshIS`.

**Changes made in this session:**
- `port/compat/d3d8_compat.cpp` synthesizes D3D9 `dcl` tokens from D3D8 vertex declarations so
  shader inputs validate. `fdx8sh.cpp` derives the pixel-shader count from the table sentinel,
  avoiding the out-of-range NULL shader entry.
- `port/main_win.cpp` initializes Fang's asset log (`ma_port_asset_log.txt`) and leaves audio
  installation disabled until the GC audio path exists.
- `fdx8padio.cpp` skips controller-emulation matching if its optional device name or map is absent.
- `port/gcdata.cpp` converts GameCube CSV tables, validates their offset tables, byte-swaps wide
  strings, and decodes GX tiled TGA data to linear ARGB for D3D. `fgamedata_LoadFileToFMem` calls
  the CSV converter too because several tables (including `sounds.csv`) bypass `fresload`.
- `fdx8tex.cpp` routes WINGC texture loads through that decoder.

**Verified run result:**
- `ma_port` builds successfully as Win32 Debug and opens a responsive window on the RTX 5070 Ti.
- The retail `.mst` is accepted. `tfh2hudall1.tga` and `tfp1smoke01.tga` decode and create D3D
  textures. Sound and damage CSV tables load, and the engine builds 355 sound groups.
- Boot then crashes at `fdx8load_Create` for `gpdmwpnunkn.ape`. This is the next port milestone:
  adapt the GC mesh representation and display lists to the DX mesh/VB/IB representation. Useful
  references are `gc/fGCload.cpp`, `gc/fGCmesh.h`, `gc/fGCdisplaylist.cpp`, `dx/fdx8load.cpp`,
  and `dx/fdx8mesh.cpp`.
- `.fpr` particle files still fail the current source-version check. Audio remains disabled.

Temporary boot/input trace prints were removed after finding the blockers. `PORTING.md` reflects
the current status. In PowerShell, rebuild and launch with:

```powershell
cmake --build build --config Debug --target ma_port -- /nologo /verbosity:minimal
build/Debug/ma_port.exe -log build/logs/run.log
```

**Git remote search:** the checkout has no configured remote or upstream. The authenticated
`cobblecorn` GitHub account has no matching Metal Arms repository/fork. Do not add an unrelated
public project or push: this is a private source/data port and the earlier handoff explicitly says
never to push. Continue with local commits on `x86-port`; ask the user for the intended fork URL if
they want remote synchronization.

## 10. ADDENDUM (session 3, 2026-09-26)

The mesh and particle startup walls are now passed for the boot-time assets:

- `port/gcmesh.cpp` converts supported static, unskinned, non-streaming GX triangle lists and
  strips into the DX mesh/VB/IB layout. It checks source vertex, color, normal-basis, UV,
  display-list, and output-buffer ranges. Mesh collision data is cleared because the GC kDOP
  representation is not translated. Skinned and streaming display lists are rejected with a log.
- The converter is called from the normal mesh resource handler, so standalone `.ape` resources
  and meshes extracted by the world loader both pass through the same conversion.
- `port/gcdata.cpp` adapts the retail FPR version 8 layout to the source version 7 structure by
  removing one 8-byte tail from each keyframe, then byte-swapping the structure. The retail corpus
  has 377 files, all version 8 / 1164 bytes; their shifted texture-name slot, 0.05–1.0 sampling
  interval, zeroed keyframe extensions, and zeroed list state were checked. Runtime startup now
  loads the first particle's `tf_1respwn1.tga` texture with no particle-version error.
- The flamethrower CSV source vocabulary ends at `pMeshEjectClip`; newer GC tail columns are not
  represented in this source and remain zeroed.

Latest build succeeds with:

```powershell
cmake --build build --config Debug --target ma_port -- /nologo /verbosity:minimal
```

Latest startup (`build/logs/continue-run-final.log`) loads the translated meshes through `gf_emp02`,
converts particles, reaches `LOAD MARKER - END OF BOOTUP`, and remains running. `Difficulty.csv`
still has 20 `Diff` fields while this source expects 8; it falls back to default susceptibility
values. Remaining major format work is world files, animation, tables, scripts, fonts, and audio.

`PORTING.md` is updated to match. Git remains local on `x86-port`; there is no configured remote,
and the earlier search found no matching fork under the authenticated account. Never push. If the
user wants a specific fork updated, ask them for its URL.

## 11. ADDENDUM (session 4, 2026-09-26)

The GC world-resource adapter is implemented and verified against `we01multi01.wld`:

- `port/gcdata.cpp` byte-swaps and range-checks the WLD header, mesh offset/size tables,
  visibility tree/portal/cell/light records, and world-init shapes and embedded CSV data.
- `ma/Lib/Fang2/fresload.cpp` invokes the adapters while loading WLDs, validates mesh ranges,
  and supplies translated visibility and init data to the runtime.
- `fvis` retains a host copy of the world's streamed GX display lists, and `port/gcmesh.cpp`
  resolves streaming draw commands from that copy.
- `ma_port -world-only we01multi01` exits 0 after translating 22 portals, 22 volumes, 22 cells,
  22 world meshes, and 101 init shapes. This diagnostic uses a temporary no-op shape callback:
  it verifies the resource conversion path and skips gameplay entity creation.
- Added `-level <name>` for the full generic debug-level startup path and `-world-only <name>` for
  the isolated resource load. At this checkpoint, the full path still stopped in `fsndfx.cpp`;
  session 5 below records the no-audio bypass and progress beyond that point.
- The diagnostic exits successfully but emits debug allocator warnings for live game-system
  objects during early teardown. It does not prove normal gameplay teardown or rendering.

Latest Debug Win32 build succeeds with `cmake --build build --config Debug --target ma_port -- -nologo -v:m`.
The latest world-only run is in `build/logs/wld-world-only.log` and its process output in
`build/logs/wld-world-only.out`. The next major runtime blocker is the GC sound-bank/audio path;
after that, resume full level setup and check entity/game-data parsing and rendering.

The local branch remains `x86-port`, with no Git remote or upstream configured. The earlier
account search found no matching fork. Do not push; ask for the intended fork URL if remote
synchronization is requested.

## 12. ADDENDUM (session 5, 2026-09-26)

The direct GameCube level launch now passes the wrapper setup and reaches boot completion:

- `port/gcdata.cpp` converts retail `.mtx` animations: big-endian headers and bone records,
  key-time arrays, scale data, and compressed or floating-point translation/orientation tracks.
  The converter validates offsets, counts, data bounds, and non-overlapping track ranges before
  changing bytes. Character animations loaded during `we01multi01` entity creation converted.
- Retail wrapper phrases have trailing values. `wpr_system.cpp` now accepts the known prefix and
  ignores trailing phrases in the WINGC build. Retail `Screen_Table_Names` has six fields per
  screen (text, mesh, and button variants for Xbox/GC), plus two additional screens; the adapter
  selects Xbox entries and loads the first 35 source screens.
- With audio disabled, `fsndfx.cpp` returns before parsing GC SFX bank bytes. The loader no longer
  crashes in the Xbox/PC bank parser. Missing sound definitions are expected until GC audio is
  implemented.
- Retail `w_laser.csv` and `w_blaster.csv` do not match this source’s user-property vocabularies.
  In the WINGC build, their incompatible systems remain unavailable and those entity instances are
  skipped. `CBotGlitch` handles an inventory hand with no supported weapon without dereferencing a
  null weapon.
- `fang_Assert()` now writes the assertion file, line, and expression to the engine log before
  showing its dialog, so any later runtime assertion is diagnosable in headless launch logs.
- Debug Win32 build succeeds. The latest `-level we01multi01` run reaches
  `LOAD MARKER - END OF BOOTUP`; `build/Debug/ma_port.exe` remained running after the log reached
  that marker. Log: `build/logs/wld-level-assert-log.log`.
- Remaining port gaps include GC audio, newer weapon and collectable schemas, unsupported
  streamed/skinned mesh data, GC collision-tree conversion, and save/input/video integration.
  Animation resources with overlapping track ranges or more than 127 bones are rejected pending
  format-specific review.

The latest work is local on `x86-port`. There is no configured remote/upstream, and the earlier
authenticated account search found no matching fork. Continue with local commits and never push;
ask for the intended fork URL if the user wants remote synchronization.

## 13. ADDENDUM (session 6, 2026-09-26)

Retail GameCube `.gt` AI graph conversion is implemented in the runtime path:

- `AIGraph.cpp` validates the graph header, vertex/POI counts and ranges, active edge counts, and
  edge targets before converting the GameCube byte order and pointerizing graph data.
- The retail corpus analysis found 71/71 `.gt` files use a 40-byte graph header, 128-byte vertices
  with five edge slots, and 8-byte POIs. The WINGC build now uses five edge slots; other builds
  retain their existing six-slot definition.
- `AIMain.cpp` converts graph bytes before pointerization and reports short reads or invalid graph
  data instead of continuing with malformed offsets.

The Win32 Debug build succeeded with:

```powershell
cmake --build build --config Debug --target ma_port -- -nologo -v:m
```

The full `we01multi01` launch converted a graph with 153 vertices and reached
`LOAD MARKER - END OF BOOTUP`, then proceeded into bot-dispenser and AI setup. Logs:
`build/logs/wld-level-gt-conversion.log` and `.out`. This confirms the graph conversion and startup
path through that marker; stable rendering and a clean gameplay run still need confirmation. One
`fvid_Swap()` warning appeared at the start of the game loop, before the first begin/end frame pair.

The current working tree contains uncommitted AI graph changes in `ma/App/ma/Ai/AIGraph.cpp`,
`AIGraph.h`, and `AIMain.cpp`. It also contains a separate pending edit in `port/main_win.cpp`
that routes Debug CRT diagnostics to the log; the recorded graph run should be repeated with that
diagnostic change to identify the modal runtime error observed during launch. Keep that edit when
reviewing the tree. `PORTING.md` was updated alongside this addendum.

Next: rebuild, then rerun with the explicit data path and inspect any CRT assertion or run-time
check captured by `port/main_win.cpp`:

```powershell
& .\build\Debug\ma_port.exe -data gamedata\files -mst mettlearms_gc.mst -level we01multi01 -log build\logs\wld-level-gt-conversion.log *> build\logs\wld-level-gt-conversion.out
```

Confirm whether the initial `fvid_Swap()` warning clears after the first frame. Then review line
endings and `git diff --check` before committing. Git remains local on `x86-port`; there is no
configured remote and no matching fork was found in the prior account search. Do not push without
the intended fork URL.
