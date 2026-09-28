# HANDOFF - Metal Arms Windows port (read this first)

Current state for the next session or the next model, local (Windows) or cloud (Linux). It is written
so a model that has never seen the project can pick it up: what exists, how to build, run and test it,
the diagnostic commands, what is open, and how this user works. Other documents:

| File | What |
|---|---|
| `PORTING.md` | Reviewer-facing: build, run, controls, what changed and why, status checklist, known issues. |
| `CLOUD_SESSION_LOG.md` | Every change made from a cloud session, each with what to verify in a real run. |
| `docs/handoff-history.md` | Every earlier HANDOFF, chronological (sections 1-25), for the reasoning behind past decisions. |
| `docs/coop-audit.md` | Campaign co-op: what is done, what single-player assumptions remain. |
| `docs/mouse-menus-design.md` | Design for mouse-driven front-end menus. |

## Goal

A working native Windows build of *Metal Arms: Glitch in the System* that runs the user's retail
**GameCube** disc data (disc ID `GM5E7D`, rev 0), with mouse-driven menus and, later, local campaign
co-op. The whole campaign is playable today; the work now is polish, the remaining log errors, and
co-op.

## Latest local pass (2026-09-27)

- `SetDamageable=false` on four inactive grunts in `WEDMmines01` now sets the existing invincibility
  flag. `NoLiftBlockChecking=true` on two lifts in that world skips only the bot blockage check during
  line movement; ordinary physical collision remains. `useby=Mil` on three `WEWCcomm_02` switches
  was already applied, but the switch parser fell through and emitted false unknown-command warnings;
  it now returns success.
- Debug and Release `ma_port` builds passed. Three parallel, isolated, muted Debug runs of
  `wedmmines01`, `wewccomm_02`, and `wemccity_01` loaded, with no crash, assert, allocation failure,
  or audio error. Mines property warnings fell from 13 to 7; Communications fell from 4 to 1.
  The remaining seven Mines warnings are malformed two-field `goodie1` tables followed by valid
  four-field drop tables. Communications has a six-field chip drop (`chip,1,1,1,X,safe`) that the
  current parser skips; the same authored pattern appears in `WESSstatn02`. Neither has been changed
  without gameplay evidence of the intended drop. City still reports two malformed `ColorRed`
  entries; the same pattern occurs in other retail worlds. Leave its color fallback alone until the
  rendered tint can be compared with retail. `dropfreq` on two front-end LiquidMesh entries remains
  unimplemented; the source only supports it on LiquidVolume.
- Actual lift blockage behavior and the newly invincible grunts still need gameplay observation;
  automated level loads confirm property parsing, not those events.

## Repository

- Private GitHub repo `cobblecorn/metal-arms-pc-port`. `main` is the verbatim source drop; all work is
  on `x86-port`, so `git diff main..x86-port` is the whole port.
- **Never commit retail data.** `gamedata/`, `main.dol`, disc images (`.rvz`/`.iso`) and anything
  extracted from them are git-ignored; tool reports that contain asset values go under ignored `build/`.
  Before pushing: `git ls-files | grep -iE '^gamedata/|main\.dol|\.rvz$|\.iso$'` must print nothing.
- Push to `origin x86-port` after the change is built and tested. Fetch and rebase your own unpushed
  commits first if another session may have pushed.
- Use a `Co-Authored-By:` trailer only when a collaborator is explicitly credited.
- Sources under `ma/` are **CRLF**; keep them CRLF. Port files (`port/`, `tools/`, docs) are LF.
  `python tools/eol.py check` lists files whose endings differ from the committed file;
  `python tools/eol.py fix FILE...` repairs them. Run it before every commit.
- The untracked `windows icon/` folder is the user's; leave it alone.

## Layout

| Path | What |
|---|---|
| `ma/` | Original Swingin' Ape source: engine in `ma/Lib/Fang2`, game in `ma/App/ma`, tools in `ma/App/*`. Port changes are small and in place, under `#if FANG_WINGC` (the Windows build using GameCube data) or `MA_PC_INPUT`. |
| `port/main_win.cpp` | Win32 entry point (replaces the MFC launcher): options, logging, crash/CRT diagnostics, stall sampler, test keys, boot. |
| `port/gcdata.cpp`, `port/gcmesh.cpp`, `port/gcaudio.cpp` | GameCube data converters: tables, textures, meshes and collision, worlds, animations, particles, fonts, camera animations, sound banks and streams. |
| `port/pc_input.cpp` | Keyboard/mouse and XInput mapped onto Fang's pads; mouse look; menu pointer; typed text; input layouts; prompt style. |
| `port/discord_rpc.cpp` | Discord Rich Presence over the IPC pipe (no SDK). |
| `port/compat/` | Direct3D 8 API on top of D3D9Ex (`d3d8_compat.cpp`), minimal D3DX. |
| `tools/` | Test runners, log tools, retail-data tools (see Diagnostics). |
| `gamedata/` (ignored) | `sys/main.dol`, `files/` (`mettlearms_gc.mst`, `Movies/*.bik`, `*.wvs`), `mst/` (extracted files). |
| `build/` (ignored) | Build output; `build/logs/` test logs; `build/shots/` captures; `build/test-saves/` test profiles. |

## Build and run (Windows)

Visual Studio 2022 (x86 tools), CMake 3.20+, Python 3 (Pillow for screenshot helpers). 32-bit only.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Release --target ma_port -- -nologo -v:m
    cmake --build build --config Debug --target ma_port -- -nologo -v:m
    build\Release\ma_port.exe -data gamedata\files -mission wedmmines01 -log build\logs\play.log

- **Release** (`build/Release/ma_port.exe`) is what the user plays: ~1.5 ms of frame work. It is built
  with `/Zi /Oy-` and linked `/DEBUG`, so crash stacks and the stall sampler still symbolize.
- **Debug** has the engine's asserts (`FASSERT`) and the CRT's checks: use it for sweeps and bug hunts.
- Input unit tests: `cmake --build build --config Debug --target ma_input_tests -- -nologo -v:m`, then
  `build\Debug\ma_input_tests.exe` (must print that all tests passed). Run after touching `pc_input`.
- A running game locks its exe: close that PID before relinking.
- Git Bash turns `/flag` arguments into paths: pass MSBuild options as `-nologo -v:m`. PowerShell wraps
  native stderr as errors.

### Game options (all in `port/main_win.cpp`'s header comment)

| Option | What |
|---|---|
| `-data DIR` / `-mst FILE` | data directory (default `gamedata\files`) / master file (default `mettlearms_gc.mst`) |
| `-mission WORLD` | start a campaign level with its mission data (the normal way to test a level) |
| `-coop 2..4` | with `-mission`: local campaign co-op, 2-4 players (shared or separate inputs) |
| `-level WORLD` / `-world-only WORLD` | debug-launch a world / load a world and exit |
| `-dev-menu` | boot into the development level picker instead of the retail front end |
| `-res WxH`, `-fullscreen`, `-no-vsync` | window size (default 1280x960), fullscreen, present without vsync (for measuring) |
| `-mute` | all audio runs (so audio errors are logged) but plays silently; **use for every test run** |
| `-no-audio` | skip audio setup entirely (faster loads; audio errors are then meaningless) |
| `-log FILE`, `-asset-log FILE`, `-console` | engine log, resource-loading log, a console window showing the log |
| `-port-diag` | the port's `PORT-*` diagnostics (perf, hitches, stalls, audio mix); also `MA_PORT_DIAG=1` |
| `-debug-info` | the game's on-screen debug overlays (script messages/errors, fps, AI and checkpoint drawing) |
| `-shots DIR -shot-every N` | save the back buffer as `DIR\shot_NNN.bmp` every N frames (default 300) |
| `-test-keys "S:VK,..."` | press virtual key VK S seconds after launch; `gS` counts from the first gameplay frame. Works without focus |
| `-discord-app-id ID\|off` | Rich Presence application (default: the port's own); `off` for tests |
| `-input-layout shared\|separate`, `-button-prompts auto\|keyboard\|xbox\|playstation`, `-mouse-sensitivity N`, `-aim-assist auto\|on\|off`, `-save-dir DIR`, `-instance-label NAME` | input, prompts, saves, a window-title label |

Useful virtual keys for `-test-keys`: Esc `0x1B` (pause; skips movies), Space `0x20` (jump; skips
movies), Enter `0x0D`, E `0x45` (use/drive), Q `0x51` / R `0x52` (weapon lists), W `0x57`.

Environment variables: `MA_PORT_DIAG`, `MA_PORT_STALL_MS` (stall threshold, default 100),
`MA_PORT_TEST_KEYS`, `MA_PORT_SHOTS`/`MA_PORT_SHOT_EVERY`, `MA_PORT_POINTER_DEBUG=1` (outline menu hit
boxes), `MA_PORT_TEXPROBE=1` (dump texture instances), `MA_PORT_INPUT_LAYOUT`,
`MA_PORT_BUTTON_PROMPTS`, `MA_PORT_MOUSE_SENSITIVITY`, `MA_PORT_AIM_ASSIST`, `MA_PORT_SAVE_DIR`,
`MA_PORT_DISCORD_APP_ID`, `MA_PORT_DISCORD_LARGE_IMAGE`/`_TEXT`.

## Where development is (2026-09-27)

| Area | State |
|---|---|
| Data | Every retail asset type converts: tables, GX textures, static/skinned/streamed meshes and kDOP collision, worlds and visibility, animations, AI graphs, particles, fonts, camera animations, sound banks, DSP-ADPCM streams. All 393 scripts bind every native. |
| Missions | All 42 campaign missions load and run under the sweep (`-mission`, Debug, 90 s each, scripted jumps). See "Mission sweep" below for the last result. |
| Rendering | Direct3D 9Ex behind the D3D8 API. Baked vertex lighting, HUD, particles, decals. Glitch's dark legs fixed (`fmesh_InitNormalSphere`). Not rechecked since D3D9Ex: alt-tab, fullscreen switching, window resize. |
| Front end | Retail front end (language, logo movies, main menu, profiles, campaign) with the mouse: hover, click, right click = Back, wheel. Pause menu and its settings screens take the pointer; On/Off, arrows and bars take clicks (user confirmed). Generated controller chart. Back keeps settings changes. |
| Input | Keyboard/mouse (raw mouse look, auto capture) and XInput; mouse aiming in vehicles and manned guns including the AA gun (Hold Your Ground, user confirmed). Keyboard/Xbox/PlayStation prompts, flush left of the text. Typed profile names. |
| Audio | MusyX banks, streams, and the GameCube volume chain. Gameplay audio after the intro movies fixed this session (39 of 39 samples audible in a meter run); the user confirmed audio in play. |
| Movies | Bink on the game's DirectSound device, 16 MB read-ahead, heap allocations. |
| Performance | Log writes on a background thread, stream loads on a worker; no stalls in normal play except a rare vertex-buffer lock wait (Open work 2). |
| Saves | Profiles in `%APPDATA%\Metal Arms PC Port\Saves`; checkpoints (1 MB). Checkpoint write-failure handling compiles but is untested. |
| Discord | Connects under the port's application; Discord accepts the activity. No image asset. |
| Co-op | Experimental PC menu entry (2-4 players, selectable controls); command line (`-mission W -coop N`): start points beside player 1, respawn beside a standing partner, scripts use player 1. Two-player menu launch verified; physical multi-pad play pending; no progress saving. See `docs/coop-audit.md`. |

### Co-op page styling and retail bot impulse property (2026-09-27)

- The PC Co-op page now has a dark blue framed panel over the animated front end.
  Its rows are spaced above the standard Accept/Back prompts, with a separate
  network-status line. `build/shots/pc_coop_page_verified/latest.png` and
  `build/shots/pc_coop_page_wide/latest.png` visually confirm 1280x960 and
  1920x1080 layouts. The page is still an experimental entry; no
  networking or campaign progress saving was added.
- The retail `DisableVelocityImpulses=true` bot property is parsed into a
  checkpointed bot flag. `CBot::HandleVelocityImpulses` clears pending impulses
  for flagged bots while leaving normal movement intact. Retail uses the
  property on six `botblink` miners in `WEMCcity_01`, `WEDMmines01`, and
  `WEWCcomm_02`. The exact retail runtime branch is unverified; a flagged
  miner's cable-release impulse is also suppressed by this implementation.
- Debug and Release build. Parallel isolated muted Debug mission runs loaded all
  three levels with no crash, assert, audio or script error. None logged
  `disablevelocityimpulses` as unknown. The front-end capture showed no overlap
  or literal formatting escapes after correction. Automated navigation proved
  the page rendered, not physical pointer/controller use.
- The run logs still have other retail property warnings: malformed `ColorRed`
  in `WEMCcity_01`; `setdamageable`, `noliftblockchecking`, and short `goodie`
  tables in `WEDMmines01`; `useby` in `WEWCcomm_02`. `tools/port_run.py` now counts
  "Error interpreting Max User Properties" blocks and prints their reason.

### Retail city goodie, grunt shields, and draw buffer (2026-09-27)

- The retail `megawasher` goodie now has a distinct PC collectable ID after the
  existing weapon range. Its retail table supplies 25 washers on pickup; ordinary
  washers still supply one. This keeps goodie-bag spawning on the correct table.
- Grunts now parse the retail `Shield=On/Off` property. Shielded grunts use the
  `GruntShield` armor profile and the recharge fields in the retail `b_grunt`
  table, following the same `CEShield` lifecycle as Titans. Shield appearance
  and combat behavior have not been inspected interactively.
- The PC dynamic fdraw vertex buffer is 8,192 vertices instead of 2,048, reducing
  expected DISCARD wraps. This has not been proven to eliminate rare D3D lock
  waits. The front-end static mesh-load lock stall is a separate path.
- Debug and Release build. An isolated 50-second muted Debug `WEMCcity_01` run
  loaded, displayed gameplay (`build/shots/retail_city_fixes/latest.png`), and
  logged no crash, assert, script or audio error, or long-frame stall. Its old
  `megawasher` and grunt `Shield` parser errors are gone. Two retail
  `disablevelocityimpulses` commands still log as unknown.

### Information item counts and pause-page scope (2026-09-27)

- The active pause build sets `_4_SCREEN_SETUP` to FALSE, intentionally using
  Options and Information only. Primary/Secondary Equipment screens in source
  are inactive in this configuration; previous attempts to reach them were
  misclassified as incomplete navigation tests. Do not enable four-page mode
  solely for a visual test.
- PC item counter text now uses the slot's text-area scale rather than fixed
  retail texels. Isolated muted Debug captures `pc_info_counts_wide`
  (1920x1080) and `pc_info_counts_classic` (1280x960) show the counters readable
  inside their item slots. The Washer description and objective remain contained.
- Debug/Release builds pass. No crash/assert/audio/script errors in these runs.
  Physical pointer and longer item descriptions are still pending.

### Information page readability (2026-09-27)

- PC pause Information page item descriptions and mission objectives no longer
  force one-to-one retail font pixels. They use their existing text-area line
  sizing and wrapping; console builds retain the retail formatting.
- Debug/Release builds pass. Isolated muted Debug captures
  `pc_info_readable_wide` (1920x1080) and `pc_info_readable_classic`
  (1280x960) show the Washer description and Mines objective readable and
  contained within their panels. Logs show no crash/assert/audio/script errors.
- `pc_inventory_primary` and `pc_inventory_info` both reached Information via
  scripted pause-page keys; the later section clarifies the two-page build.
  Longer item text, physical pointer and live resize remain pending.

### Widescreen pause layout correction (2026-09-27)

- Pause fdraw layout and mouse cursor now scale x from half-width and y from
  half-height / 0.75, matching the retail normalized coordinate range. The old
  width-only scale pushed highlights and pad prompts off their text at 16:9.
- The helper supplies matching full forward/inverse matrices, confined to 2D
  fdraw. CFXfm uniform-scale metadata is not used for sphere/normal calculations
  in these passes. Generated PlayStation symbols additionally compensate their
  local x scale to stay round while the layout fills the display.
- Debug/Release builds pass. Debug captures `pc_pause_wide` (1920x1080,
  PlayStation) and `pc_pause_classic` (1280x960, keyboard) confirm aligned Options
  highlights, frame and unclipped prompts. `pc_pause_wide_final/options.png`
  confirms round PlayStation symbols after the final correction.
  Its final capture also verifies Advanced Settings Cross/Circle footers at
  1080p. No crashes/asserts/audio/script errors; loading/Present stalls remain.
- Tests use muted isolated instances. Physical pointer alignment, inventory
  pages beyond Information, Xbox art proportions, alternate aspect ratios and
  live resize still require coverage. This is not a claim of complete widescreen UI support.

### Xbox dialog alignment and two-button coverage (2026-09-27)

- Xbox dialog prompts retain their retail atlas art but now use the measured
  action label's center and height, with allowance for the atlas cell padding.
  This matches the placement used by the generated PC themes.
- Debug/Release builds passed. Debug `pc_dialog_xbox_confirm` and
  `pc_dialog_playstation_confirm` visibly reached the Quit confirmation at
  1920x1080: A/B and Cross/Circle align with Accept/Cancel. Final captures are
  in build/shots/<run>/latest.png; corresponding logs have no crash/assert,
  audio or script errors. Existing loading/render hitches remain.
- Runs were muted and isolated, ended after 57 seconds, and did not accept the
  quit confirmation. Physical pads and alternate Y/Triangle remain unverified.

### Widescreen pause-menu issue found (2026-09-27)

- 1920x1080 captures in `pc_dialog_xbox_pair` and
  `pc_dialog_playstation_pair` show the pause Options highlight displaced from
  its label and bottom/top pad prompts clipped. This is outside the fixed dialog
  coordinate path: do not claim the full pause screen is widescreen-correct.
- Starting point: CPauseScreen::Draw uses xfmTemp.BuildScale(HalfRes.x) for both
  axes of a layout whose y range is +/-0.75. The cursor repeats that uniform
  scale. Audit those transforms, the ortho viewport and wrapper entry/return
  state together; preserve the working 4:3 layout and mouse hit coordinates.
- Script navigation: W/S drive pause-menu selection. Arrow keys drive the right
  stick and did not move selection in the initial pair of runs.

### PlayStation and widescreen dialogs (2026-09-27)

- Fixed PlayStation message-box glyphs using centered coordinates in a top-left
  pixel viewport. The glyph helper now accepts y-down drawing (including an
  upright Triangle); regular wrapper callers keep y-up coordinates.
- Widescreen verification exposed dialog text using display aspect instead of
  the PC ftext 0..0.75 vertical range. Corrected title, body and all three action
  labels under MA_PC_INPUT. This prevents text drifting above its dialog at 16:9.
- Dialogs now measure each action label before drawing generated prompts. Their
  center and right edge follow the actual text bounds; PlayStation icon radius
  follows the text height. Keyboard keycaps retain their capped readable scale.
- Debug/Release builds pass. Muted isolated Debug captures verify the Accept
  Cross at 1920x1080 (`pc_dialog_ps_aligned`) and keyboard Space at 1280x960
  (`pc_dialog_keyboard_aligned`), under build/shots and build/logs. Both reached
  the settings write-failure warning without crashes/asserts/audio/script errors.
  Loading/render hitches remain. Physical controller input, Circle/Triangle
  dialogs and the Xbox dialog theme still need runtime coverage.

### Dialog keyboard keycaps (2026-09-27)

- Message boxes used top-left screen pixels but the generated keyboard keycap
  helper drew centered y-up geometry. Text appeared, while the raised key and
  border were misplaced. Added an explicit screen-pixel mode to both keycap
  helpers; message boxes select it and wrapper callers retain their default.
  Keycap drawing restores culling after supporting either coordinate direction.
- Debug/Release builds passed. Muted, isolated Debug `pc_dialog_keycap` capture
  `build/shots/pc_dialog_keycap/warning-keycap.png` confirms the Space keycap
  border and label align next to Accept; earlier frames show normal wrapper
  keycaps intact. No crash/assert/audio/script errors; existing stalls remain.
- The PlayStation coordinate follow-up is addressed in the newer section above;
  that section records the remaining theme coverage limits.

### PC settings write-failure feedback (2026-09-27)

- Advanced Settings now checks both PC preference writes before leaving, in the
  frontend and pause wrapper. Locked launch overrides are skipped. A failed
  write shows the existing warning dialog once per visit; dismissing it and
  selecting Back again retries and permits leaving with session values intact.
- Fixed keyboard message-box prompts overlapping their action text: key labels
  now have a capped scale and align to the retail glyph's right edge. Xbox and
  PlayStation glyph paths are unchanged.
- Debug/Release builds passed. Isolated Debug `pc_settings_final` deliberately
  blocked the settings directory with a file. Visually confirmed the fitted
  warning (`build/shots/pc_settings_final/warning.png`). The earlier
  `pc_settings_warning_fixed` run confirmed dismissal and return to Options.
  No crash/assert/audio/script errors. Loading/Present stalls still occur.
- Test uses -mute, isolated saves and LOCALAPPDATA. Physical pad operation and
  frontend runtime coverage of this warning remain pending; both wrapper paths
  share the same helper. No changes to actual checkpoint write-failure handling.

### Save-folder failure UI (2026-09-27)

- Verified the Campaign shortcut's error fallback with a file deliberately used
  as `-save-dir`: backend logged No usable save directory and the menu stayed
  usable. This fixture is under ignored build/test-saves; user saves unaffected.
- On PC, the no-device selection now reads Play Without Saving, hides meaningless
  free-space/profile zeroes, and explains an unavailable folder plus the lack of
  progress saving. If the folder is available, it instead explains how to choose
  Save Folder. Existing confirmation/launch behavior is unchanged.
- Release/Debug builds passed. Debug `pc_save_failure_ui` screenshot confirms the
  final layout and failure message (`build/shots/pc_save_failure_ui/latest.png`).
  No crashes/asserts/script/audio errors; the injected storage initialization error
  is expected. Startup stalls remain. This checks the fallback UI, not checkpoint
  write failures or the full no-save campaign flow. All test processes closed.

### Input handoff and profile verification (2026-09-27)

- `_MouseOtherInput` now clears the wrapper's cached same-frame hover, click,
  right-click, wheel and tick-drag state after keyboard/controller navigation.
  Previously it only hid the pointer, leaving `_MouseHoverSelect` able to apply
  cached mouse input after a key/stick selection. This is a source-confirmed
  conflict; it is not established as the cause of earlier test-route variations.
- Release and Debug builds passed. `pc_profile_direct` (muted, isolated settings
  and saves; keys `10:0x20,14:0x1B,22:0x20`) visibly reached Select Profile directly
  from Campaign, with proper space between Save Location and Save Folder.
  Screenshot: `build/shots/pc_profile_direct/latest.png`. No crash/assert/script/
  audio errors; startup stalls remain. This closes the profile-label visual check.
- Earlier `pc_profile_spacing` stopped at the main menu, so it is not profile
  verification. All runs ended and no game/test processes remain. Simultaneous
  physical mouse + controller interaction is still not exercised.

### Main-menu style follow-up (2026-09-27)

- User requested matching the existing label style. Generated Co-op and Quit labels
  now use the already loaded angular display font (slot 3), italic tilt, a blue
  outline and layered gold face/shadow. Selected labels reuse the retail green
  highlight mesh centered on the measured text bounds. Labels remain gold when
  selected. The font uses uppercase glyphs; these approximate the mesh lettering,
  rather than replacing or shipping extracted retail assets.
- Release/Debug builds passed. Muted `pc_menu_style` visually confirms the labels
  and selected Quit highlight in `build/shots/pc_menu_style/latest.png`; Quit
  exited by itself with code 0. No crashes/asserts/audio/script errors; startup
  and shutdown stalls remain. Test used isolated settings/saves and closed.
- Added and passed input checks for all four controller slots, separate keyboard
  routing, restoring configured layout, and invalid session-layout fallback.
  These validate mapping logic, not physical multi-controller gameplay.
- User is away: continue automated verification, record physical pad/audio checks
  as pending, and do not wait for user verification to advance independent work.

### Desktop exit follow-up (2026-09-27)

- Added PC-only Quit to Desktop to the main menu, using gameloop_ScheduleExit()
  for ordinary game-loop and application teardown. Mouse hit target and existing
  keyboard/controller navigation include the new fourth item.
- Main-menu drawing makes a temporary copy of the three retail mesh layouts and
  moves the two selection meshes up. Co-op and Quit fit below them; loaded retail
  layout data is not modified. No change to console menu entries.
- Release and Debug builds succeeded. Muted `pc_desktop_quit` with isolated saves
  and settings selected Quit and exited by itself with code 0 before the runner
  timeout. No crashes/asserts/audio/script errors; startup loading stalls remain.
  `build/shots/pc_desktop_quit/latest.png` visibly confirms all four menu entries
  and the selected Quit button. Physical controller and pointer acceptance were
  not exercised by this scripted-key run. No game/test processes remain.

### Controller-first co-op menu follow-up (2026-09-27)

- User requires controller-only co-op. Menu now defaults to Controllers, mapping
  pads 1-4 to P1-P4 (keyboard optional for P1), with a Keyboard + controllers
  alternative and Players 2-4 selector. No progress saving; networking inactive.
- CLI co-op no longer rejects shared layout or forces separate layout. Normal
  configured layout applies, shared by default. Use `-input-layout separate` for
  keyboard P1 + pads P2-P4. Temporary menu routing restores configured input at
  startup reset. `docs/coop-network-plan.md` describes current behavior.
- Added sparse main-menu navigation logs. `pc_menu_trace` proved co-op entry.
  `pc_coop_controls` proved Back, re-entry and a two-player controllers-layout
  launch into Mines. Split-screen rendered; the expected missing-P2-controller
  prompt appeared. No crashes/asserts/audio/script errors; loading stalls remain.
  The scripted route did not prove four-player launch or switching controls.
- All finite runs used mute, Discord off, isolated saves/settings and ended.
  Physical multi-pad play remains unverified; do not expand gameplay scope.
- Co-op page now uses standard Accept/Back prompts. Removed an attempted borrowed
  panel mesh after its capture showed rendering artifacts; use a dimmed animated
  background instead. Final dimming change builds, visual check pending.

### Co-op menu prototype (2026-09-27)

- PC main menu adds Co-op with a separate Start Local Co-op / Back screen. It
  clearly labels two-player local play as experimental, no progress saves, and
  network play as unavailable. Start uses campaign level 0 and virtual profiles.
- Keyboard/mouse owns player 1; first controller owns player 2. A temporary input
  routing override restores the configured layout at startup-menu reset.
- Release and Debug builds succeeded. Follow-up captures found the initial Co-op
  label offscreen: ftext Y uses a 0..0.75 coordinate range. Fixed the new menu's
  positions, explicit colors, and two-decimal scale escapes. Main-menu label now
  visibly renders (`build/shots/pc_coop_menu_checked/submenu-check.png` is actually
  a main-menu capture). Co-op submenu navigation/launch remains unverified: the
  scripted routes reached main/PvP join instead. Do not count these as co-op tests.
- Finite muted runs `pc_coop_menu_visual`, `pc_coop_menu_layout`,
  `pc_coop_menu_checked`, and `pc_menu_routes` ended without logged crashes,
  asserts, audio or script errors. Startup loading stalls remain. No personal
  saves/settings used. All runners and games closed.
- Source follow-up: reacquire controller selection after hotplug in the co-op
  submenu; initialize virtual profiles with InitNewProfile(TRUE), clear their
  storage metadata, and clear the prior profile device before the no-save launch.
  These changes build but still require runtime verification.
- Profile screen now positions the save-folder name after the measured label
  width (when the label is the final text row), fixing the character-count spacing
  estimate. Visual verification of this change is pending.
- Changes remain uncommitted with the earlier PC menu work.
- Read `docs/coop-network-plan.md` for source findings and staged network design.
  No networking is implemented. AI, cutscenes and campaign rules still have the
  limitations in `docs/coop-audit.md`; keep further gameplay work out of this pass.

### PC input polish follow-up (2026-09-27)

- Advanced Settings now has a **Mouse Sensitivity** row in the front end and pause settings.
  It shows a multiplier of the default 0.1 degrees/count, changes in 0.10x steps (0.10xÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œ10.00x),
  and saves on Back/Accept under Local AppData alongside Button Prompts. The profile's existing
  Look Sensitivity still multiplies mouse look. Valid launch/environment overrides lock the row
  and do not overwrite the saved preference. Invalid settings fall back to the default.
- `pcinput_Sample` previously discarded scripted test keys whenever the window lacked focus,
  contradicting the documented test recipe. Scripted keys now bypass that gate; the physical
  controller sample is zeroed first so real background input remains blocked.
- Debug and Release builds succeeded. `ma_input_tests` passed, including isolated settings-file
  round trips, invalid values, override precedence/protection, and background scripted input.
  The Release `pc_mouse_verified` run navigated to the new row, changed it from 1.00x to 1.10x,
  and saved `MouseSensitivity=0.110000` on Escape. No crashes, asserts, script or audio errors;
  one 110 ms Present stall remains. All test processes and runners were confirmed stopped.
  Captures and logs are under `build/shots/pc_mouse_verified/` and `build/logs/pc_mouse_verified.log`.
  All private runs used `-mute` with separate test settings; no listening check was performed.
- Pause Advanced Settings and Audio Levels now use the localized Back footer, matching their
  save-on-exit behavior and the front-end label. Debug/Release builds passed; `pc_back_footer`
  visibly confirmed the new footer without crashes/asserts/script/audio errors (one Present stall).
- `menu_drive.py` now refuses pointer actions when the target window is unfocused or minimized,
  instead of reporting success for input the game discards. Its documentation now states the focus
  requirement. Use `-test-keys` for unattended background navigation.
- Front-end checks (`pc_front_settings`, `pc_front_existing`) created an isolated profile and loaded
  it again to the Launch screen. Advanced Settings there still needs a visual check; posted pointer
  input stopped working when focus changed. One capture showed a Reset confirmation; No was selected,
  and the existing profile loaded afterwards. Its trigger was not established. All tests were muted,
  used `build/test-saves/`, and their processes were closed. Retail front-end and pause layout tables
  have identical option-row positions, but this alone does not verify front-end runtime behavior.

### This session (2026-09-27, final pass), newest first

Commits `9a0268a`, `fcdaeb0`, `8241ffb`, `84cdfae`, all pushed.

1. **Mission sweep fixes** (`84cdfae`), from running all 42 missions and grouping their log errors:
   - **Crash opening Shady's shop in `wessstatn01`** (`BarterTypes.cpp`, `CBarterLevel::InitLevel`): the
     sale-chain array was sized to the candidate items alone, so a level listing more chains wrote past
     it, and chains left without an item were read. Now sized chains + candidates, the fill is bounded,
     and `m_nSaleChains` is the number filled. **Not yet confirmed in play by the user.**
   - **Audio assert in `WEWJjourn01`**: an emitter with an outer radius below DirectSound's minimum;
     `Create3D` clamps it to `DS3D_DEFAULTMINDISTANCE` (`fdx8audio.cpp`).
   - Budgets raised for the PC (`fang.cpp`, WINGC): collision impacts 4096, decals 400 / 6000 vertices
     (both ran out in busy fights).
   - Log noise removed where the retail game ignores the same data: retired items (Mil Translator,
     Antenna, EUK, Mission Briefing) in `ItemInst.cpp`; weight class `none` and the BotDie lookup in
     `bot.cpp`; sound group `None` in `fsound.cpp`; a missing intro movie in `level.cpp`.
   - `-mute`, `tools/mission_parallel.py` options (`--quiet`, `--test-keys`, `--no-audio`) and the new
     `tools/log_errors.py`.
2. **Campaign co-op** (`8241ffb`): start points (`CStartPtMgr::InitLevel`: 3 units beside or 4 behind
   player 1 instead of inside walls), `_CoopRespawnNearPartner` (`player.cpp`: a player who dies or falls
   out comes back beside a standing partner; the level checkpoint restore runs only when nobody is
   standing), `_ScriptPlayer()` / `_ScriptSetPlayersControl` (`MAScriptTypes.cpp`: scripts and cutscenes
   act on player 1 and freeze everyone). Split screen verified; respawn not yet exercised.
3. **AA gun aims with the mouse** (`fcdaeb0`, `botAAgun.cpp`): it only read the right stick; now adds
   `TakeMouseLookDelta` to heading and pitch. User confirmed.
4. **Gameplay audio after the intro movies** (`9a0268a`, `fdx8audio.cpp`): once Bink's heap allocation
   let the 110 MB intro play, emitters paused by it came back without voices:
   - `CFAudioEmitter::Pause(FALSE)` on a voiceless emitter now requests PLAY (it asked to unpause a voice
     it did not have);
   - `_ResumeStrandedEmitters()` (start of `faudio_Work`) resumes emitters left paused above the
     current pause level;
   - 3D voices are allocated for emitters already in range (listener state PRESENT/SWITCHED), not only
     on ENTERED.
   The `PORT-MIX` snapshot now counts voiced vs. voiceless emitters and names in-range voiceless ones.
5. **Q/R weapon lists no longer flicker** (`9a0268a`, `gamepad.cpp`): the weapon list pauses the game
   loop, and mouse look (hence menu mode) was keyed off any pause; now off the pause menu only
   (`pausescreen_IsActive()`).

Earlier this day (details in `docs/handoff-history.md` section 25): D3D9Ex and the dark-legs fix
(another model), the pointer in the pause menu and its settings screens, clickable settings, the
controller chart, flush-left prompts, Back keeps settings, the hitch fixes (async log, stream worker,
Bink read-ahead), the Release build, Discord, test tooling, and another model's pass (Xbox menu map on
PC, prompt settings, `-coop`, `-asset-log`, `mission_parallel.py`, checkpoint write hardening).

### Mission sweep

The last full sweep, `sweep3`, after all the fixes above:

SWEEP3_RESULT

Warnings that remain in the logs (all reproduce retail data the source does not handle; none crash):

- **`Unknown command 'shield'`** (hundreds of lines, 12 levels): retail grunts have a shield the source
  lacks. `main.dol` has `GruntShield`; the Titan's shield (`CEShield`, `bottitan.cpp`, "TitanShield")
  shows how a bot owns one. This is a missing retail feature, the most visible item left.
- Explosion `rocket`, texture `tfa1sawstr4`, a duplicate `laserl3` decal table, sounds `Door Locked` and
  `SOM_MOpen`, the `Vehicle` bank (absent from the GameCube data), `megawasher`, `debrisshakespersec`,
  `tripwirewho`, `AI_ATTACKWHO PLAYER`, `AI_RACE good`, barter `EUK` names and `Battery 2`-`6`.
  Check the retail tables (`tools/gamedata_dump.py`) and `main.dol` strings before mapping any of them.

## Diagnostics cookbook

Everything here is muted and keeps Discord off unless it says otherwise. Logs go to `build/logs/`.

**One private test run, summarized** (Release by default; `--config Debug` for asserts):

    python tools/port_run.py --mission wedmmines01 --seconds 60
    python tools/port_run.py --config Debug --mission wessstatn01 --seconds 90 --name shop_test

The summary lists `PORT-PERF` lines (fps, worst frame, work before Present), the longest hitches, each
`PORT-STALL` stack, crash/assert/run-time-check reports and audio errors.

**Screenshots** (engine captures of the back buffer; cost frame time, never in the user's session):

    python tools/port_run.py --mission WEDTtown_01 --seconds 40 --shots 120
    build\Release\ma_port.exe -data gamedata\files -mission wedmmines01 -mute -discord-app-id off -shots build\shots\mines -shot-every 150

`port_run.py --shots N` clears `build/shots/NAME/`, and saves the newest frame as `latest.png`.
Read the PNG/BMP to look at it.

**Scripted keys** (no focus needed):

    python tools/port_run.py --mission wedmmines01 --seconds 30 --shots 60 --test-keys "g8:0x1B"
    python tools/port_run.py --seconds 60 --test-keys "10:0x20,14:0x20,30:0x0D"

The first pauses 8 s into gameplay (pause-menu tests); the second skips the logo movies and presses
Enter in the front end.

**Driving menus with the mouse** (posts window messages; never moves the desktop cursor):

    python tools/port_run.py --mission wedmmines01 --keep --shots 30 --test-keys "g8:0x1B" --name pause
    python tools/menu_drive.py --pid PID waitpause build/shots/pause 60
    python tools/menu_drive.py --pid PID click 630 296
    python tools/port_run.py --stop PID --name pause

The target game must be focused for posted pointer input; `-test-keys` works in the background.
Always pass `--pid`: without it the first game window found is used, which may be the user's. Fixed
coordinates are layout-dependent; confirm the screen in a capture.

**All missions in parallel** (up to 4 instances on the user's PC; use 3 while they play):

    python tools/mission_parallel.py --config Debug --seconds 90 --jobs 3 --run-name sweepN --quiet --test-keys "10:0x20,14:0x20,18:0x20,24:0x20,30:0x20,40:0x20" MISSIONS...
    python tools/log_errors.py "build/logs/sweepN_*.log"

The 42 missions: `WECDsneak01 WEDMmines01 WEDTtown_01 WEMCcity_01 WERRreactr1 WEWCcomm_01 WEWHchase01
WEWJjourn01 WEWZzombi01 webccolis01 webccolis02 webccolis03 webccolis04 wecdsneak02 wecffacty01
wecrruins01 wecrruins02 wediinvas01 wedmmines02 wedmmines03 wemccity_02 wemccity_03 wemccity_05
wermmorbot1 wermmorbot2 werrreactr2 wesccorros1 weshhangr01 wesrrepair1 wessstatn01 wessstatn02
wewccomm_02 wewccomm_03 wewchold_01 wewjjourn02 wewjjourn03 wewkrockt01 wewrresrch1 wewrresrch2
wewrresrch3 wewrresrch4 wewtrace_01`. A 42-mission sweep at 3 jobs takes about 25 minutes; run it with
a background shell or a second terminal and wait for it to finish rather than polling. `log_errors.py` lists
crashes/asserts per log first, then each distinct message with how many logs and lines have it.

**Co-op:** `python tools/port_run.py --mission WEDTtown_01 --coop 2 --shots 120` (split screen; player
1 on keyboard/mouse, players 2-4 on pads).

**Performance and stalls:** runs with `-port-diag -no-vsync` (what `port_run.py` passes) log
`PORT-PERF` every 10 s. A frame over `MA_PORT_STALL_MS` (default 100) makes the stall sampler suspend
the game thread and log its symbolized stack as `PORT-STALL`; `--stall-ms 50` lowers it.

**Audio:**

- `-port-diag` logs `PORT-MIX` snapshots every 2 s: active and voiced emitters, free voices, pause
  levels, each sound's level, and in-range playing emitters that have no voice ("voiceless"). Also
  `PORT-SND`, `PORT-TALK` (bot dialog), `PORT-DUCK`.
- Is a process audible? Start it with `--audio` (only when the user expects sound) and meter it:
  `powershell -ExecutionPolicy Bypass -File tools\audio_meter.ps1 -ProcessId PID -Seconds 30`.
  A `-mute` run meters 0 by design; so does `-no-audio`.

**Crashes and hangs:** crashes and the first of each assert log a symbolized stack (asserts are
rate-limited: first 10, then 100, 1000...). A run that looks hung is usually a dialog: check
`Get-Process -Name ma_port | select Id, MainWindowTitle`.

**Retail evidence** (settle schema questions from the retail data, not guesses; write outputs under
`build/`): `tools/mst_list.py` (list/extract the `.mst`), `tools/gamedata_dump.py` (binary `.csv` tables
as JSON), `tools/dol_vocab.py` (table vocabularies from `main.dol`), `tools/dol_xref.py` (PowerPC code
referencing an address), and a plain string search of `gamedata/sys/main.dol`.

**Workflow for a fix:** reproduce in a private Debug run (or a sweep) -> patch -> build Debug and
Release -> rerun the affected missions -> `log_errors.py` on the new logs -> `tools/eol.py check` ->
commit (trailer) -> check no retail data is tracked -> push. Tell the user what to try in their session.

## Working without Windows (cloud / Linux sessions)

No MSVC, no retail data, no game runs or logs. What works:

- `python3 tools/syntax_check.py [--changed REF | FILE...]`: clang against MinGW headers with the
  MSVC build's settings; all C/C++ files pass. Catches type errors and API misuse; not a substitute
  for an MSVC build. Needs `clang mingw-w64-i686-dev g++-mingw-w64-i686-win32`.
- `python3 tools/mathdiff/mathdiff.py`: runs the GC-layout math (`dx/fdx8gcmath_*.inl`, what this build
  uses) and the shipped SSE math on the same inputs and reports differences. Needs
  `clang gcc-multilib g++-multilib`.
- Everything a cloud session changes goes in `CLOUD_SESSION_LOG.md` with what to verify; the user
  builds, runs and reports back.

## Where things are (code map)

| Topic | Where |
|---|---|
| Options, logging, stall sampler, test keys, crash reports | `port/main_win.cpp` |
| Input mapping, menu mode, pointer, text input, prompt style | `port/pc_input.cpp`; `ma/App/ma/gamepad.cpp` (`gamepad_Sample`: control maps, `allowLook`) |
| Mouse in menus, settings clicks, controller chart | `ma/App/ma/wpr_system.cpp` ("mouse pointer (PC port)", `_MouseAdd*`, `_PcControllerMap`) |
| Prompt icons and key caps | `ma/App/ma/wpr_drawutils.cpp` (`wpr_drawutils_DrawButtonOverlay`, `_DrawKeyCap`) |
| Pause menu | `ma/App/ma/PauseScreen.cpp` (`CPauseScreen::Work`, `pausescreen_IsActive`) |
| In-game PC wording | `ma/App/ma/game.cpp` (`_aPcPhrases`, `game_PcPromptWork`), retail phrase maps `_anRetailPhraseField` / `_anRetailGamePhraseField` |
| Audio emitters, voices, GameCube volume chain, stream worker | `ma/Lib/Fang2/dx/fdx8audio.cpp` |
| Movies | `ma/Lib/Fang2/dx/fdx8movie2.cpp` (`_MovieAlloc`, `BINKIOSIZE`) |
| D3D8-on-D3D9Ex, screenshots | `port/compat/d3d8_compat.cpp` |
| Co-op | `launcher.cpp` (`-coop` init), `MultiplayerMgr.cpp` (`CStartPtMgr::InitLevel`), `player.cpp` (`_CoopRespawnNearPartner`), `MAScriptTypes.cpp` (`_ScriptPlayer`) |
| Budgets | `ma/Lib/Fang2/fang.cpp` (WINGC block) |

## How to work with this user (important)

- The user plays the game windows you launch, **while you work**, and reports by ear and eye; they
  cannot read logs. Their session is Release, audible, with Discord on, launched like:
  `build\Release\ma_port.exe -data gamedata\files -mission WORLD -port-diag -log build\logs\play_NAME.log`
  (`-port-diag` so you can read what happened in their session afterwards).
- **Every one of your own test instances is muted** (`-mute`, or `-no-audio` when audio doesn't
  matter) with `-discord-app-id off`. The tools do this by default. Muted windows say
  `[TEST RUN - MUTED]` / `[TEST RUN - NO AUDIO]` in their title. An unmuted test window blasts
  default-volume audio over the user's game; a muted one looks like "no audio" to the user if they
  pick it up, so close your test windows when done.
- Never kill the user's game except to relink its exe: then stop that exact PID, rebuild, and relaunch
  it for them without being asked (they asked for this). Tools only ever close the PIDs they started.
- The user prefers less time on visual checks: capture a frame when a change is visual, don't iterate
  on pixel details unless they ask.
- Keep replies short and concrete: what changed, what to try in game.
- Edit CRLF sources with the Edit tool or a script that keeps line endings (`sed -i` in Git Bash strips
  CRs). Build C string edits with the Edit tool: shell heredocs mangle backslash escapes.

## Open work, roughly in priority order

1. **Vertex-buffer stalls**: the PC dynamic fdraw buffer is now 8,192 vertices;
   measure whether that reduces the rare ~110 ms `fdx8vb_Lock` wait in
   `build/logs/coop2.log` (`CEZipLine::_Draw`). The `rel_front2.log` wait is
   instead a static mesh-load VB lock, and `coop2.log` also has a separate
   Present stall. Do not attribute either to dynamic-buffer wrapping.
3. **Confirm with the user**: Shady's shop in `wessstatn01` opens; a co-op death respawns beside the
   partner; alt-tab/fullscreen/resize under D3D9Ex; the PC Button Prompts setting persists.
4. **Co-op** (`docs/coop-audit.md`): front-end entry, progress saving, barter, collectables, AI
   targeting, minigames and bosses, cutscene cameras for players 2-4.
5. One ambient in `L02_rslide3` stays voiceless (its listener state is EXITED while in range); see the
   "voiceless" lines in `PORT-MIX`.
6. The remaining retail-data warnings (see "Mission sweep" and latest pass above), including
   malformed `ColorRed` and `goodie` entries and LiquidMesh `dropfreq`. The `setdamageable`,
   `noliftblockchecking`, and `useby` world entries are handled.
7. The ~1 s pause between the front end's logo movies (opening the next movie on the game thread).
8. Older items: the save flow from the menus; the laser's charged burst and other weapons' particle and
   sound fields; `Difficulty.csv` extra fields; failed-load teardown beyond `CLOUD_SESSION_LOG.md` 5/10;
   a Discord image asset; 64-bit, widescreen, rumble.

## Things that cost time before

- Don't re-investigate solved problems: fonts, skinned meshes, the world-origin bone, world collision,
  dark legs (normal sphere), gameplay audio after the intro (paused voiceless emitters).
- "No audio" reports: first check whether the window is one of your muted test windows (title), then
  the in-game volume (Back used to cancel settings changes), then `PORT-MIX` voiced vs. voiceless.
- The `we01multi01` restart loop was never explained (it is a multiplayer map launched through the
  debug path); test with campaign levels (`-mission`) instead.
- Sweeps with `-no-audio` hide every audio error; sweep with `-mute`.
- A backgrounded shell loop outlived its task and kept opening windows: use the tools, which close
  their own processes.
- Retail data drift is the usual cause of "wrong text/sound/value": the retail tables were reordered
  or extended after this source snapshot. Dump the retail table and compare with the source enum before
  changing code; check `main.dol` for names and tables.
- `PauseScreen.h` cannot be included from `gamepad.cpp` (it redefines `_PLATFORM_XB`); use the
  `pausescreen_IsActive()` accessor.
