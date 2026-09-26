# HANDOFF - Metal Arms Windows port (read this first)

Current state for the next session, local (Windows) or cloud (Linux). Other documents:

| File | What |
|---|---|
| `PORTING.md` | Reviewer-facing: build, run, controls, what changed and why, status checklist, known issues. |
| `CLOUD_SESSION_LOG.md` | Every change made from a cloud session, each with what to verify in a real run. |
| `docs/handoff-history.md` | The old chronological HANDOFF (sessions 1-24), kept verbatim for the reasoning behind decisions. |
| `docs/coop-audit.md` | Single-player assumptions to remove for campaign co-op. |
| `docs/mouse-menus-design.md` | Design for mouse-driven front-end menus. |

## Goal

A working native Windows build of *Metal Arms: Glitch in the System* that runs the user's retail
**GameCube** disc data (disc ID `GM5E7D`, rev 0). The user also wants mouse-driven menus and,
later, local co-op.

## Repository

- Private GitHub repo `cobblecorn/metal-arms-pc-port`. `main` is the verbatim source drop; all work is
  on `x86-port`, so `git diff main..x86-port` is the whole port.
- **Never commit retail data.** `gamedata/`, disc images and anything extracted from them are
  git-ignored; tool reports that contain asset values go under ignored `build/`. Before pushing, check
  `git ls-files | grep -iE '^gamedata/|\.rvz$|\.iso$'` prints nothing.
- Pushing to `x86-port` is fine. Local and cloud sessions both push there: fetch and rebase your own
  unpushed commits before pushing.
- Commit messages end with a `Co-Authored-By:` trailer naming the parallel session model that made the change.
- Sources under `ma/` are **CRLF**; keep them CRLF (edit byte-safely, or normalize after editing).
  Port files (`port/`, `tools/`, docs) are LF.

## Layout

| Path | What |
|---|---|
| `ma/` | Original Swingin' Ape source: engine in `ma/Lib/Fang2`, game in `ma/App/ma`, tools in `ma/App/*`. Port changes are small and in place. |
| `port/main_win.cpp` | Win32 entry point (replaces the MFC launcher): options, crash/CRT diagnostics, boot. |
| `port/gcdata.cpp`, `port/gcmesh.cpp`, `port/gcaudio.cpp` | GameCube data converters: tables, textures, meshes and collision, worlds, animations, particles, fonts, camera animations, sound banks and streams. |
| `port/pc_input.cpp` | Keyboard/mouse and XInput mapped onto Fang's pads; mouse look; input layouts. |
| `port/compat/` | Direct3D 8 API on top of D3D9 (`d3d8_compat.cpp`), minimal D3DX. |
| `tools/` | Build helpers and retail-data tools (below). |
| `gamedata/` (ignored) | `sys/main.dol`, `files/` (`mettlearms_gc.mst`, `Movies/*.bik`, `*.wvs`), `mst/` (extracted files). |

## Build and run (Windows)

Visual Studio 2022 (x86 tools), CMake 3.20+, Python 3. 32-bit only.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Debug --target ma_port -- -nologo -v:m
    build\Debug\ma_port.exe -data gamedata\files -mission wecdsneak01 -log build\logs\run.log

All options are in `PORTING.md` and at the top of `port/main_win.cpp` (`-level`, `-mission`,
`-world-only`, `-res`, `-fullscreen`, `-no-audio`, `-shots`, `-mouse-sensitivity`, `-aim-assist`,
`-input-layout`, `-save-dir`). `-mission <world>` is the normal way to test a campaign level.

## Debugging (Windows)

- **A run that looks hung is usually a dialog.** `main_win.cpp` routes CRT asserts, `/RTC` failures,
  pure-virtual calls and invalid-parameter errors to the log, but check anyway:
  `Get-Process -Name ma_port | select MainWindowTitle`, and `taskkill /F /IM ma_port.exe` to clear it.
- Crashes and the first occurrence of each assert log a symbolized stack. Asserts are rate-limited
  (first 10, then 100, 1000, ...). Treat a firing assert as a real bug.
- Engine captures: `-shots <dir> -shot-every <frames>`.
- `tools/mission_sweep.sh <secs> <world>...` runs each mission and counts loads, crashes, asserts,
  script/data errors and audio problems (logs under `build/logs/sweep/`). Run it in the foreground.
- `tools/audio_meter.ps1` reads a process's audio peak meter (is the game audible?).
- Retail formats and schemas: `tools/mst_list.py` (list/extract the `.mst`), `tools/gamedata_dump.py`
  (binary `.csv` tables as JSON), `tools/dol_vocab.py` (retail table vocabularies from `main.dol`),
  `tools/dol_xref.py` (PowerPC code referencing an address). Use them to settle schema questions from
  retail evidence instead of guessing.
- Git Bash turns `/flag` arguments into paths: use `-nologo -v:m`. PowerShell wraps native stderr as
  errors.

## Working without Windows (cloud / Linux sessions)

No MSVC, no retail data, no game runs or logs. What works:

- `python3 tools/syntax_check.py [--changed REF | FILE...]`: clang against MinGW headers with the
  MSVC build's settings; all 403 C/C++ files pass. Catches type errors and API misuse; not a
  substitute for an MSVC build. Needs `clang mingw-w64-i686-dev g++-mingw-w64-i686-win32`.
- `python3 tools/mathdiff/mathdiff.py`: runs the GC-layout math (`dx/fdx8gcmath_*.inl`, what this build
  uses) and the shipped SSE math on the same inputs and reports differences; known ones are explained
  in the tool. Needs `clang gcc-multilib g++-multilib`.
- Everything a cloud session changes goes in `CLOUD_SESSION_LOG.md` with what to verify; the user
  builds, runs and reports back.

## Current state

Working (verified by runs or by the user, see `PORTING.md` for detail):

- Boot, the GameCube master file, and conversion of every retail asset type the game loads: tables,
  textures (GX formats), static/skinned/streamed meshes and kDOP collision, worlds and visibility,
  animations, AI graphs, particles (v8), fonts, camera animations, scripts (all 393 bind every native).
- All 44 campaign worlds swept with `-mission`; Night Sneak (`wecdsneak01`) is the interactive
  baseline. Rendering with textures, lighting and HUD; some scenery surfaces are still solid black.
- Keyboard/mouse (raw mouse look, auto capture) and XInput; mouse aiming for vehicles and manned guns.
- Audio: sound effects (MusyX banks decoded to PCM), music and speech streams (DSP-ADPCM), with the
  GameCube mix (MusyX distance model, stream gains); 160 virtual emitters.
- Bink movies; checkpoints (1 MB streams on Windows).
- Many retail schema changes mapped from `main.dol` (weapons, bots, goodies, materials, debris,
  sentries, AA gun, scout, corrosive boss, Spy vs Spy).

Changed from a cloud session (details in `CLOUD_SESSION_LOG.md`). Built with MSVC and run on
2026-09-26: it compiles cleanly; `wewchold_01`'s failed load now exits cleanly (it crashed before);
the save directory is logged at startup; `wecdsneak01`, `webccolis04` (script errors 6 -> 0),
`wesrrepair1`, `wesccorros1`, `wessstatn01`, `wewhchase01`, `wewccomm_03`, `weshhangr01` and
`-level wecdsneak01` load and run. **Still unverified**: the save flow itself (create, load, rename,
delete, in-game save, kill mid-save), which needs the menus.

- PC save backend rewrite: `%APPDATA%\Metal Arms PC Port\Saves` (`-save-dir`), safe file names,
  atomic writes, `ValidateProfile` now reports missing profiles.
- Failed level loads tear down before releasing memory (`game.cpp`, `level.cpp`).
- GC-layout math: `CFVec4A::ReceiveUnitXZ` and `CFMtx44A::Mul( rM, f )` now match retail.
- Script event masks: events 32-63 behave as on the GameCube (ignored) instead of misfiring.

## Open work, roughly in priority order

1. **Verify the save flow** from the menus (create, load, rename, delete, in-game save; kill the game
   mid-save). The rest of the cloud-session changes are verified (see above).
2. `wewchold_01` (Hold Your Ground): the retail `Mini_Game` table has 104 fields where the source
   expects 62, so the level fails to load. Map it from `main.dol` like the other schemas.
3. `CFQuatTang3::Calculate` NaN (scripted carts in `WEDTtown_01`, `wessstatn02`): a zero XZ tangent is
   unitized. Check the path tangent input.
4. User confirmations pending: chase-level AI driver (probably fixed by the XZ math fix), RAT controls,
   vehicle reticle, dialog balance, throwables, and the new keys: Q = throwables list, R = weapons list
   (tap to reload), E = action, with lists opening after a 0.5 s hold (`PORTING.md` controls).
5. Mouse-driven menus: follow `docs/mouse-menus-design.md` (clickable button prompts first).
6. Remaining black scenery surfaces.
7. Failed-load teardown beyond what `CLOUD_SESSION_LOG.md` entries 5 and 10 cover: systems
   created during `level_Load()` other than alarms/spawns (e.g. `aimain_InitSystem()`) have no
   explicit teardown on that path; check a failing level's log for asserts after the failure.
8. Retail features loaded but not implemented: laser charged burst, particle/sound fields of several
   weapons, `Difficulty.csv` (20 fields vs 8), barter EUK kits and battery upgrades.
9. Missing damage profiles retail lacks too (`Debris`, `SentinelCannon`): probably nothing to do.
10. Co-op: follow `docs/coop-audit.md`.
11. Later: 64-bit (150+ inline-asm blocks), widescreen, rumble.

## Things that cost time before

- Don't re-investigate solved problems: fonts, skinned meshes, the world-origin bone, world collision.
  The `we01multi01` restart loop was never explained (likely multiplayer-specific: it is a multiplayer
  map launched through the debug path); test with campaign levels (`-mission`) instead.
- The Debug CRT's modal dialogs looked like hangs (see Debugging).
- Tool inputs can collapse backslash escapes; build C string edits with the Edit tool.
- Editing CRLF files with some tools leaves mixed line endings; normalize afterwards.
- A backgrounded sweep loop outlived the task that started it and kept opening windows: run sweeps
  in the foreground.
