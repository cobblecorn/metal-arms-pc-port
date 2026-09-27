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
`-input-layout`, `-save-dir`, `-debug-info` for the on-screen script monitors and debug overlays). `-mission <world>` is the normal way to test a campaign level.

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
- All 44 campaign worlds swept with `-mission`. Rendering with textures, lighting and HUD; world objects
  get their baked vertex lighting (GameCube color streams remapped onto the converted vertex buffers).
- The retail front end boots (language → logo movies → main menu → campaign/multiplayer setup) and is
  **mouse-driven** (see below). Keyboard/mouse (raw mouse look, auto capture) and XInput; mouse aiming
  for vehicles and manned guns.
- Audio: sound effects (MusyX banks → PCM), music/speech streams (DSP-ADPCM), and **the GameCube volume
  chain** (below). User confirmed 2026-09-26 on the first mission (`wedmmines01`): droids now talk, the
  laser is audible, music no longer drowns dialog ("still slightly loud" was before the 2D fix below).
- Bink movies; checkpoints (1 MB streams on Windows). Saves in `%APPDATA%\Metal Arms PC Port\Saves`.
- Many retail schema changes mapped from `main.dol`.

## Session of 2026-09-26 (evening): what changed, and where it stopped

All of this is committed and pushed on `x86-port` (last commit `6cfcd81`). Newest first:

1. **Audio, the real GameCube chain** (`ma/Lib/Fang2/dx/fdx8audio.cpp`, block starting "The retail
   GameCube mix"). From `gc/fgcaudio.cpp` + MusyX: every volume passes `_GetVolume()` (≈ 0.38·(v^¼+v^½)),
   ×0.8 for 3D effects or ×0.6 for stereo streams, MusyX fades 3D linearly to 0 at 1.25× radius, then
   MusyX's DLS table squares it (`main.dol` 0x3de80c, entry i = (i/127)²). `_GCMusyxVolume`,
   `_GC3DDistanceGain`, `_GCStreamGain`, emitter volume at "MIDI volume as fgcaudio.cpp and MusyX
   compute it". Full-volume music ≈ 0.21 amplitude, full-volume effects/speech ≈ 0.58.
   Also removed (for FANG_WINGC) the DX layer's `fVolume *= 0.1f; // Hack to attenuate 2D sounds` in
   `CFAudioEmitter::SetVolume` — it cut the player's weapon and 2D dialog to a tenth. **The user has
   not heard this last change yet** (build `6cfcd81`); ask whether music is now balanced, and whether
   anything 2D is now too loud (UI sounds, footsteps).
2. **Laser firing sound**: retail `w_laser.csv` field 65 is a sound *group* ("LaserFire"); resolved in
   `CWeaponLaser::ClassHierarchyBuild` and played via `CFSoundGroup::PlaySound` (`weapon_laser.cpp`).
3. **Streamed bot dialog**: `BotTalkInst.cpp` used Win32 `PlaySound()` on .wav files that don't exist;
   now uses the console path `level_PlayStreamingSpeech` (the first droids in the mines were silent).
4. **Movies (#4 of the user's list, not yet confirmed by the user)**: Bink now uses the game's
   DirectSound device (`fdx8audio_GetDirectSound()`, handed over in `fmovie2_Play`) instead of opening
   a second one, and `fmovie2_Draw` never spins waiting for the next movie frame (vsync paces it).
   The front end's logo movies now use the GameCube names (`GC_*_logo.bik`; it asked for `XB_*`).
5. **Mouse menus** (`wpr_system.cpp` "mouse pointer (PC port)" block, `pc_input.cpp` menu pointer,
   `ftext_GetLastPrintBounds()` in `ftext.cpp`, button hit boxes in `wpr_drawutils.cpp`): hover
   selects, click picks, right click = Back, prompts clickable, wheel steps lists/adjusts settings,
   clicks queued with positions, main-menu items hit-tested from their 3D meshes. Pointer = HUD reticle
   `tfh_cross01`. `MA_PORT_POINTER_DEBUG=1` outlines hit boxes. User confirmed it mostly works; the pause
   menu (`PauseScreen.cpp`/`MenuTypes.cpp`) is **not** mouse-enabled yet.
6. **Retail phrase-table drift fixed**: `wpr_system.cpp` `_anRetailPhraseField` (menu phrases) and
   `game.cpp` `_anRetailGamePhraseField` (in-game phrases) map the source's enums to retail fields
   (the retail tables were reordered; e.g. "Delete" showed "You will not be able to save...", the MP
   join screen showed "head").
7. **PC wording and prompts**: `wpr_datatypes_PcText()` (storage text: save folder / reset / free
   space); game phrases `_aPcPhrases` in `game.cpp` name keys ("Press E to drive vehicle") or Xbox
   buttons, switched by `game_PcPromptWork()` from `pcinput_PromptsForPad()`; menu prompts draw
   generated key caps (Enter/Esc/E/R) in `wpr_drawutils.cpp` `_DrawKeyCap`. In menus Esc = Back,
   Enter = accept (`PcInputState::menus`); Esc still pauses in gameplay and skips movies.
8. **Windowed exe** (`/SUBSYSTEM:WINDOWS`, `-console` to get a console) — the console full of
   script prints ("NONETRIPWIRE ENTER EVENT") is gone. **Diagnostics off by default**:
   `Fang_bPortDiag` (`-port-diag` / `MA_PORT_DIAG=1`) gates all `PORT-*` logging; the periodic ones
   caused the ~5 s lag spikes. With it on: PORT-HITCH, PORT-SND, PORT-MIX (2 s snapshots of every
   sound's level), PORT-TALK, PORT-DUCK.
9. **Discord Rich Presence** (`port/discord_rpc.cpp`, IPC pipe, no SDK): `-discord-app-id <id>` /
   `MA_PORT_DISCORD_APP_ID`. **Untested end to end: the user still has to create a Discord
   application and give its ID.** Discord runs on the user's PC (`\\.\pipe\discord-ipc-0` exists).
10. Exe icon from the user's art (`port/res/ma_port.ico`, `.rc`); Q/R weapon-list hold now 0.3 s;
   the miner bot no longer loads a nonexistent 'Miner' bank.

## How to work with this user (important)

- The user plays the game windows you launch, **while you work**, and reports by ear/eye; they cannot
  read logs. For subjective issues launch a logged session in the background:
  `./build/Debug/ma_port.exe -data gamedata/files -mission wedmmines01 -port-diag -log build/logs/<name>.log`
  (or no `-mission` for the front end) and read its `PORT-*` lines afterwards.
- A running game locks `build/Debug/ma_port.exe`. **Close it yourself** (taskkill by PID, from
  `Get-CimInstance Win32_Process -Filter "Name='ma_port.exe'"`) whenever you need to relink, then
  relaunch a session for them — the user asked for this explicitly; don't make them wait.
- Driving the menus without touching the real mouse: the scratchpad script `drive.py` posted
  WM_MOUSEMOVE/WM_LBUTTONDOWN to the game window (client pixels); pointer input comes from those
  messages, so this works. Screenshots: `-shots <dir> -shot-every N` writes back-buffer BMPs.
- Commit trailer: `Co-Authored-By: parallel session Opus 5.5 <noreply@collaborator.com>`; push to `origin x86-port`.
  Never commit retail data (`gamedata/`, `main.dol`, dumps) — keep derived reports under `build/`.

## Open work, roughly in priority order

1. **Confirm with the user** (build `6cfcd81`): music vs dialog balance after the 2D fix; movie
   micro-stutter and cutscene audio clipping (#4); key-cap prompts; Esc/Enter in menus; PC wording.
2. **Dark textures on Glitch's legs/feet** (user report, not investigated): take a `-shots` capture of
   the player; suspect lighting/color streams on the skinned mesh (`SetColorStreams` warnings in logs)
   or a material pass.
3. **Pause menu mouse support** (`PauseScreen.cpp`/`MenuTypes.cpp`, `CMenuMgr`), same approach as
   `wpr_system.cpp` (record item boxes when drawn, hover/click in the input read). Its L/R shoulder
   page flip currently only works with the mouse buttons as triggers; give it keys (e.g. Q/E or Tab).
4. **PlayStation-style prompts** (user asked for Xbox/PS/keyboard icons): Xbox art exists (`tfh_a`..),
   keyboard key caps are drawn; PS would need generated glyphs and a way to detect or choose the pad
   type (XInput can't tell; maybe an option).
5. **Discord**: get the application ID from the user, test (`-discord-app-id`), maybe bake a default.
6. Name keyboard: accept typed characters (WM_CHAR) — needs a text-entry mode in `pc_input` so letters
   don't also fire their game bindings (Q = Back in menus).
7. Performance: the user plays the **Debug** build; a Release/RelWithDebInfo configuration has not been
   tried. Worth trying for smoother play.
8. Older items: verify the save flow from the menus; Hold Your Ground's retail features (Mini_Game
   fields 62-103); `CFQuatTang3` NaN on scripted carts; laser charged burst and other weapons'
   particle/sound fields; `Difficulty.csv` extra fields' meaning; barter EUK kits; failed-load
   teardown beyond `CLOUD_SESSION_LOG.md` 5/10; co-op (`docs/coop-audit.md`); 64-bit, widescreen, rumble.

## Things that cost time before

- Don't re-investigate solved problems: fonts, skinned meshes, the world-origin bone, world collision.
  The `we01multi01` restart loop was never explained (likely multiplayer-specific: it is a multiplayer
  map launched through the debug path); test with campaign levels (`-mission`) instead.
- The Debug CRT's modal dialogs looked like hangs (see Debugging).
- Tool inputs can collapse backslash escapes; build C string edits with the Edit tool.
- Editing CRLF files with some tools leaves mixed line endings; normalize afterwards.
- A backgrounded sweep loop outlived the task that started it and kept opening windows: run sweeps
  in the foreground.
- Retail data drift is the usual cause of "wrong text/sound/value": the retail tables were
  reordered or extended after this source snapshot. Dump the retail table (`tools/gamedata_dump.py`)
  and compare with the source enum before changing code; check `main.dol` for names/tables.
- The Bash tool mangles backslash escapes in heredocs (a backslash-n became a real newline inside C
  strings): write edit scripts with the Write tool and run them, or use the Edit tool.
