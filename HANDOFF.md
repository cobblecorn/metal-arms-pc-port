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
local co-op. A CLI-only campaign player-slot prototype exists; gameplay rules and bot selection are
not implemented.

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
- `tools/port_run.py` runs a muted, Discord-off test build and summarizes its log; `--keep` and `--stop`
  support menu-driving runs. `tools/menu_drive.py` posts mouse messages to a selected game PID and can
  wait for a pause-menu screenshot. `tools/eol.py` checks or fixes line endings against Git.
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
- Pause menu with the pointer (user confirmed 2026-09-27), Discord Rich Presence (connects under the
  port's own application by default), typed profile names, keyboard/Xbox/PlayStation prompts.

## Session of 2026-09-27 (follow-up: mission error cleanup)

1. **Tracer alpha clamp** (`ma/App/ma/tracer.cpp`): `_GroupWork` now clamps normalized distance and
   computed alpha to `[0,1]`. The approximate reciprocal used for max distance could put a tracer a
   little past full distance immediately before its kill check, producing negative vertex alpha and
   reaching the float-to-color assertion in `tracer_Draw`. Four Debug mission runs (60 seconds each)
   reached end-of-loading without crashes, asserts, allocation failures, script errors, or data
   warnings. The earlier intermittent assert was not reproduced.
2. **Grunt `dropweapon` property** (`ma/App/ma/botgrunt.cpp`): after parsing this recognized property,
   `CBotGruntBuilder::InterpretTable` now returns success instead of letting the base builder report
   it as unknown. Two focused runs and a four-mission follow-up had no `Unknown command 'dropweapon'`
   messages.
3. **Retail barter response** (`ma/App/ma/BarterTypes.cpp`): `NOSOUPFORYOU` now maps to the existing
   `PURCHASE_ABORT` Shady state. The retail Generic_Bot_Talks table pairs it with the valid
   `bd_nothing` response, so it had been an unsupported keyword causing barter initialization to
   fail. In `WEMCcity_01` and `WEWRresrch4`, generic response initialization and `Barter_MoveToPoint`
   failures no longer appear. `WERMmorbot1` has no barter data for that level; its no-barter message
   is expected.
4. The Debug `ma_port` build succeeded. All ten mission runs for these fixes were **muted**; no
   audio behavior was checked. The runs covered `WEWJjourn01`, `WEWCcomm_01`, `WERMmorbot1`,
   `WEMCcity_01`, and `WEWRresrch4`.
5. **Open log/retail-data leads; no code changes made:** barter tables contain unrecognized `EUK`
   weapon/scope names and `Battery 2`–`Battery 6`; resolve how barter purchases preserve EUK mesh and
   battery-count semantics before adding aliases. `WEMCcity_01` also uses `megawasher` and
   `disablevelocityimpulses`, which have no source implementation; it sets `Shield=on` on grunts even
   though only Titans parse it, and two liquid `ColorRed` values are malformed three-string fields.
   In `WEWRresrch3`, retail `AI_Race=Evil` actors target players as enemies; `MIL` is the compatible
   port race, but retail exposes no explicit `EVIL` alias. These findings came from read-only retail
   and log audits and were not implemented in this pass.
6. No commits or pushes were made for this follow-up. Preserve the other dirty working-tree changes.

## Session of 2026-09-27 (current uncommitted pass)

1. **Quiet tests mute Bink without freezing its video.** `-no-audio` now sets Bink track volume to zero
   instead of calling `BinkSetSoundOnOff(FALSE)`. That API stopped the silent movie clock: the mines
   intro stayed on black frame 2. The Debug build now plays the muted intro through at least frame 900;
   its screenshot shows the movie, and the per-process audio meter stayed at 0.000 for 115 samples.
   Normal user launches keep movie and game audio enabled unless `-no-audio` is supplied.
2. **Per-instance testing:** `-asset-log <file>` gives every process a separate asset log. The new
   `tools/mission_parallel.py` can queue several missions with a `--jobs` cap, unique engine/asset/save
   paths, and concise load/script/assert/performance summaries. `port_run.py` also reports script and
   data-load errors; `audio_meter.ps1 -ProcessId PID` measures one process among multiple instances.
   Two-player mission smoke runs reached end-of-loading in `WEDTtown_01` and `wedmmines01`; both ran
   around 140 fps with no asserts or crashes. Tests were muted. The Mines script emitted one invalid
   SFX-handle error under `-no-audio`; gameplay audio is intentionally unavailable in those runs.
3. **Basic campaign co-op initialization prototype:** `-mission WEDTtown_01 -coop 2` (2–4) creates
   multiple local campaign player slots, keeps `bSinglePlayer=TRUE` and uses the existing split-screen
   setup. It defaults to separate keyboard/controller ports, has no profile pointers or persistent
   saves, and is for initialization experiments only. It does not add a bot selector or solve campaign
   combat, cutscenes, pause, death/checkpoint, or progression behavior. A run loaded two players at
   distinct positions. A later capture showed the main view, but the lower split-screen view was
   malformed. The user asked to keep this prototype basic and not pursue those gaps in this pass.
4. **Button prompt preference** was added to PC Advanced Settings (Auto, Keyboard, Xbox, PlayStation),
   saved under Local AppData. A valid command-line/environment override locks the choice for that run.
   Prompt changes also cover start text, message buttons, and the vehicle-exit prompt. It compiles;
   menu placement, persistence and visuals still need a user-visible check. XInput cannot identify a
   PlayStation controller, so Auto uses Xbox glyphs for pad-only ports.
5. **Checkpoint write hardening:** reserve flush alignment padding, retain backend write/flush errors,
   and do not mark a failed checkpoint save as complete. The Debug build compiled these changes;
   checkpoint failure behavior has not been runtime-tested.
6. **Resolved handoff items:** the Hold Your Ground `Mini_Game` 104-field loader fix is already in
   `d696cbc`; retail-only fields 62–103 are intentionally ignored. The `CFQuatTang3` fallback for a
   degenerate/vertical cart tangent is already present; the town mission reached end-of-loading with
   no assert/crash, but cart motion was not visually verified.
7. Debug build command used:
   `cmake --build build --config Debug --target ma_port -- /nologo /verbosity:minimal`.
   It succeeded after changing enum stepping in the prompt option to an explicit cast. No changes from
   this pass have been committed or pushed. Keep the pre-existing `windows icon/` files. Three other
   active UI files (`PauseScreen.cpp`, `win/screenshot.cpp`, `wpr_drawutils.cpp`) still have mixed
   line endings; check and normalize after that UI edit pass settles.

## Session of 2026-09-27 (latest): test tools in the repository

Code is committed and pushed as `59dd5bf` (`Test tooling in tools/; test windows say they are muted;
gameplay-relative test keys`).

1. `tools/port_run.py` starts Release by default (Debug is optional), adds `-port-diag`, disables
   Discord and audio by default, and uses `-no-vsync` for timing. It can capture `-shots`, summarize
   performance, hitches, stalls, asserts/crashes and audio errors, and stop only the PID it started.
   `--keep` leaves the game running for menu interaction; `--stop PID` ends that test and summarizes it.
2. `tools/menu_drive.py` posts mouse messages to the game window without moving the desktop cursor.
   Pass `--pid PID` to select a test window. It supports pixel or fractional coordinates, clicks,
   wheel input, and `waitpause` for detecting the pause screen in captured frames.
3. `tools/eol.py` checks and fixes line endings to match the committed file (or the majority ending for
   a new file). Use it after editing original CRLF sources with tools that may normalize line endings.
4. Test windows launched with `-no-audio` show `[TEST RUN - NO AUDIO]` in their title, so they are
   distinguishable from the user's audible session.
5. Test keys can now be gameplay-relative: `-test-keys "g8:0x1B"` presses Escape eight seconds after
   gameplay begins. This avoids timing the key from process launch, since muted runs load the level
   faster and can otherwise press Escape during the intro. The pause test succeeded with this form.
6. In the last menu recipe, mouse navigation selected Audio Levels when Controller Map was intended.
   Treat fixed menu coordinates as layout-dependent; use captured frames to confirm the destination.
   The scripted key path is the dependable way to time keyboard actions.

## Session of 2026-09-27 (later): settings clicks, controller chart, hitches, Release

All committed and pushed (last code commit `9f56059`). Newest first:

1. **Back keeps settings** (`_PcSettingsBackKeeps` in `wpr_system.cpp`): leaving Audio Levels or
   Advanced Settings with Back (Esc, right click) used to cancel (console convention); the user lost a
   volume change that way. Now Back = Accept there, front end and in game.
2. **Movies**: the frame loop waited on Bink's file reads mid-movie (600+ ms; the intro's stutter when
   the disk is busy). PC Bink now has a 16 MB read-ahead (`_FMOVIE2_PC_IO_BYTES`, `BINKIOSIZE`) and heap
   allocations (`_MovieAlloc`) instead of the consoles' 2 MB pool. One ~1 s pause remains *between* the
   front end's logo movies (opening the next one); not chased.
3. **Hitches** (`b2793a8`): the log was written/flushed on the game thread per line (1+ s stalls on a
   busy disk) -> background writer in `main_win.cpp` (`_LogAppend`/`_LogFlush`/`_LogWriter`, crash and
   exit paths flush synchronously). Streams (`CFAudioStream::Create`) read the header and made a
   track-sized DirectSound buffer on the game thread -> the whole load runs on a worker
   (`_StreamJob_t`, `_LoadStream`, reference counted; destroy-while-loading abandons the job to its
   worker; `faudio_Uninstall` waits via `_WaitForStreamLoads`). Result on `wedmmines01`: no stalls
   except one driver `Present` on a level's first frames; worst frames 14-30 ms.
4. **Measuring tools** (keep using them): `-no-vsync`; under `-port-diag` a `PORT-PERF` line every 10 s
   (fps, worst frame, work before Present) and the **stall sampler** (`_StallWatchdog` in
   `main_win.cpp`): when a frame passes 100 ms (`MA_PORT_STALL_MS` to change) it suspends the game
   thread, copies its frame-pointer chain and logs it symbolized as `PORT-STALL`. Needs frame pointers:
   Debug, and Release now builds with `/Zi /Oy-` and links `/DEBUG` (`CMakeLists.txt`).
5. **Release build** works (front end and a mission tested): ~1.5 ms of frame work vs ~5 ms in Debug.
   `cmake --build build --config Release --target ma_port` -> `build/Release/ma_port.exe`. The user
   should play Release from now on (it's what was launched for them last).
6. **Controller map** is a generated chart (`_PcControllerMap`/`_PcMapIcons` in `wpr_system.cpp`):
   each label's input comes from its position key (`"A"`, `"LeftY"`, `"Black"`; this build loads the
   Xbox layout, B = right face button). Keys + mouse glyph for keyboard, Xbox/PS glyphs for pads.
   `wpr_drawutils_DrawMouseGlyph`, `wpr_drawutils_DrawFaceButton`.
7. **Settings take clicks**: selection arrows, On/Off and 2-way/4-way values, and level bars are click
   zones that carry their row (`_MouseAddZone`, `_MouseAddToggle`, `_MouseAddTickBar`,
   `_MouseAddSelectionArrows`; a bar click walks the value to the clicked tick one step a frame).
8. Key caps/glyphs center on text using the prompt font's measured line (`_MeasurePromptFont`,
   defaults 0.041 / 0.0115 per unit of scale); if icons sit off-center, check those.

## Session of 2026-09-27: another model's commits, reviewed and continued

An intervening session added:
Direct3D 9Ex (`Direct3DCreate9Ex`/`CreateDeviceEx`, managed pool mapped to default + dynamic, because
plain D3D9 HAL caps failed on the user's RTX 5070 Ti after a driver change), pause-menu mouse and Q/E
pages, `-button-prompts auto|keyboard|xbox|playstation`, typed profile names (`pcinput_SetTextInput`,
WM_CHAR queue), Discord asset options, and **the fix for Glitch's dark legs**: `fmesh_InitNormalSphere()`
was never called on the PC, so every unskinned GameCube normal decoded as straight up (verified on
screen: the legs are lit now). Reviewed; kept, with these fixes and additions on top:

1. **Pointer in the pause menu actually works**: the pause menu switches to the menu control map only
   while it samples buttons, so `pcinput_BeginFrame` saw the gameplay map and kept the mouse captured.
   `gamepad_Sample()` now passes `allowLook = MAIN1 map && !FLoop_bGamePaused`. With that, Escape is
   Back in the pause menu (it resumes), and an Escape press that began before a gameplay/menu switch
   is ignored until released (`escapeHeldOver` in `pcinput_Sample`), so pausing never unpauses.
2. **Settings screens opened from the pause menu** (Advanced Settings etc., the in-game wrappers) lost
   the pointer: `_MouseDrawOverlay` drew in `_pViewportOrtho3D`, which only exists in the front end.
   In-game it now uses the viewport `wpr_system_IG_Draw` drew with (`_pMouseIGViewport`), from the
   overlay hook after the text. **Not yet confirmed by the user.**
3. Q is no longer Back in menus (GameCube map: CROSS_LEFT = Back, so Q both flipped the pause page and
   closed the pause menu). The pause keys read through `pcinput_KeyHeld` (focus-aware) instead of raw
   `GetAsyncKeyState`.
4. **Prompt layout** (user request: "flush left instead of a weird off angle"): every icon style is
   sized to its text line, centered on it and flush left of the text (`wpr_drawutils_DrawButtonOverlay`,
   PC version; tuning constants `_PROMPT_ICON_SIZE/_CENTER/_ART_FILL`). **Not yet seen by the user.**
   The pause menu draws key caps (Space/Esc at the bottom, right-aligned to the art; Q/E on the tabs).
   PlayStation glyphs are now solid round buttons with thick symbols (`wpr_drawutils_DrawPlayStationGlyph`).
5. Menu prompt wording follows the port it addresses (`pcinput_PromptStyleForPort`): other players'
   pads never get key names (co-op).
6. Typed names accept only characters the on-screen keyboard has (`_ProfileName_HasKey`; a stray
   profile "Profile1&&&" was made while the old filter let `_`/`'` through); Enter is Done and is not
   also START while typing.
7. **Discord**: the user's application ID `1553650972218363985` is the default
   (`_szDefaultDiscordAppId` in `main_win.cpp`; `-discord-app-id off` disables). The worker logs
   "Discord: connected", a refused handshake, or Discord's error answer to SET_ACTIVITY. Verified: it
   connects and Discord accepts the activity. No Rich Presence image asset is set (Discord shows the
   application's icon).
8. `-test-keys "62:0x1B,70:0x51"` (`MA_PORT_TEST_KEYS`): scripted key presses the game reads without
   focus, for unattended `-shots` tests. `port/compat/d3d8.h` restored to CRLF (the D3D9Ex commit
   rewrote its line endings).

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
  read logs. **Run your own test/benchmark instances with `-no-audio`** (they otherwise blast full-volume
  default-profile audio over the user's session) and `-discord-app-id off`.
- Run private test sessions with `python tools/port_run.py --mission wedmmines01 --seconds 60`;
  they are muted and keep Discord off by default. Add `--shots N` for screenshots or
  `--test-keys "g8:0x1B"` to pause eight seconds into gameplay. For scripted menu work, use
  `--keep`, then `python tools/menu_drive.py --pid PID ...`, and finish with
  `python tools/port_run.py --stop PID --name NAME`. Mouse messages do not move the desktop cursor.
  Keys still require focus unless sent through `-test-keys`; verify mouse-selected screens in captures
  because fixed coordinates can land on a neighboring menu item. Screenshots cost frame time; do not
  leave them on in sessions the user plays.
- A running game locks its executable (`build/Release/ma_port.exe` for the normal player build).
  Stop the specific PID before relinking, then relaunch the user's session when needed — the user asked
  for this explicitly; don't make them wait.
- `sed -i` in this Git Bash strips CRs from CRLF files (most sources are CRLF); edit with a script that
  keeps line endings, or the Edit tool.
- Commit trailer: `Co-Authored-By: parallel session Opus 5.5 <noreply@collaborator.com>`; push to `origin x86-port`.
  Never commit retail data (`gamedata/`, `main.dol`, dumps) — keep derived reports under `build/`.

## Open work, roughly in priority order

1. **Confirm with the user**: settings clicks (arrows, On/Off, bars) and the controller chart; the
   flush-left prompt layout; movie stutter with the 16 MB read-ahead (#4); that Esc out of Audio Levels
   now keeps the volume. The user reported audio "super loud" — their profile's lower volume had been
   lost to Back-cancels (fixed); the front end before a profile loads plays at default volume (retail).
2. Pause menu hit boxes for its page tabs and bottom prompts are fixed fractions (`CPauseScreen::Work`);
   derive them from `m_avtxButton` and the text areas if the layout ever changes.
3. The PC Button Prompts setting is implemented but still needs menu, persistence and visual
   confirmation. XInput does not identify PlayStation pads, so Auto resolves them as Xbox.
4. D3D9Ex: default-pool resources survive device resets on Ex, but alt-tab / fullscreen switching and
   window resizing have not been retested since `4d13270`.
5. Performance: done for now (see the later 2026-09-27 section). Remaining: the ~1 s pause between
   logo movies; a Bink movie open still reads on the game thread (short, before playback).
6. Discord: optional Rich Presence image (needs an asset uploaded to the application).
7. Pause-menu page flips with a pad's shoulders work as before; check they still do with the keyboard
   map change (Q no longer CROSS_LEFT in menus).
8. Older items: verify the save flow from the menus; laser charged burst and other weapons' particle/
   sound fields; `Difficulty.csv` extra fields' meaning; barter EUK kits; failed-load teardown beyond
   `CLOUD_SESSION_LOG.md` 5/10; expand campaign co-op only when requested (`docs/coop-audit.md`);
   64-bit, widescreen, rumble.

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
