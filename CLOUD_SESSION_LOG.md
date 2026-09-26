# Cloud session change log

Every change made from a Linux cloud session. That session has no
Windows machine, no MSVC, no retail game data and no game logs, so:

- **Nothing here has been built with MSVC or run.** "Checked" means `tools/syntax_check.py`
  (clang + MinGW headers) accepted the changed files, plus reading the code.
- Each entry lists what to verify in a real run. Please build, run, and report back anything
  that fails; the entry says where to look.

Newest entries at the bottom.

---

## 1. PC save backend rewrite (`ma/Lib/Fang2/dx/fdx8storage.cpp`, `port/main_win.cpp`)

**Why.** Saves go through Fang's `fstorage` layer. On PC the front end uses the Xbox flow with one
"hard disk" device, backed by `dx/fdx8storage.cpp`. That backend existed but had these bugs:

1. `fstorage_ValidateProfile` always returned "valid", even for a profile with no file.
   `CPlayerProfile::IsOnCard()` depends on it, so the in-game save flow (`_IG_SaveGame_Work`)
   could never detect a missing profile and always treated a "roaming" (not yet saved)
   profile as present, showing the overwrite warning.
2. `fstorage_CreateProfile` wrote `uSize` bytes (the whole profile, several KB) from a
   260-byte stack buffer with one initialized byte: a stack over-read.
3. Profile names were used raw as file names (`profile-<name>`, in whatever directory the game
   was started from). The name keyboard comes from retail data and can produce characters
   Windows rejects, names that differ only by case (the same file on Windows), and trailing
   spaces (which Windows strips).
4. `fstorage_GetProfileInfos` copied a whole directory-entry name into a 32-character buffer
   (overflow for any long `profile-*` file), ignored the start offset limit the Xbox version
   checks, and counted directories and stray files as profiles.
5. Writes happened in place, so a crash mid-save left a corrupt profile (the game's CRC check
   then rejects it and the profile is lost).
6. At install the free-space figures stayed 0 until the first 0.25 s device poll, so an
   immediate "room for a new profile?" check could fail.

**What changed.**
- Save directory: `-save-dir <dir>` / `MA_PORT_SAVE_DIR`, else
  `%APPDATA%\Metal Arms PC Port\Saves`, else `saves` in the working directory. Created if missing.
  The absolute path is logged at startup (`[ FSTORAGE ] Saving profiles in ...`).
- File name: `profile-<hex>.sav`, the name's UTF-16 units as 4 hex digits each ("Bob" ->
  `profile-0042006F0062.sav`). Case-sensitive like the Xbox; any character works.
- Writes (create and update) go to `<file>.tmp`, are flushed, then replace the profile with
  `MoveFileEx(REPLACE_EXISTING | WRITE_THROUGH)`.
- `ValidateProfile` fails when the profile file is missing, a directory, or empty.
- `GetProfileInfos` validates names, respects the offset, and never returns unfilled entries.
- A missing profile on read is not logged (the menus probe names with reads; see
  `_DoesNameExistOnCard`); other failures log the Windows error code.
- A save directory that cannot be created leaves the device connected but unavailable, so the
  menus report an unusable device instead of the game failing to start.
- Migration: profiles saved by earlier builds (`profile-<name>` in the working directory, first
  4 bytes = `PROFILE_SIGNATURE`) are copied into the save directory at startup. Existing new-format
  profiles are never overwritten, and the originals stay where they were.

**Checked.** Syntax check of both files; no warnings from the new code under `-Wall -Wextra`.

**Verify in a run.**
- Log shows `Saving profiles in '<path>'`, and a `Copied profile ...` line for each old profile.
- Create a profile, save in-game, quit, relaunch, load it. Rename and delete a profile.
- Names with punctuation, and two profiles differing only by case.
- Kill the game during a save: the previous profile should still load.

## 2. `tools/syntax_check.py` (new)

**Why.** The cloud session cannot run MSVC. This lets it (or anyone on Linux) catch compile
errors before pushing.

**What.** Runs `clang -fsyntax-only` with MinGW-w64 headers and MSVC-like settings: 32-bit,
`wchar_t` as `unsigned short` (`/Zc:wchar_t-`), MS extensions/compatibility (VS 2022 version),
delayed template parsing, the CMake defines and include paths. Windows-only habits are handled in
a symlink mirror under `build/syntax-check/`: include names in another case or with backslashes
get aliases, the shader headers get empty stand-ins named like `build_shaders.py`'s output, and
two constructs MSVC's permissive mode accepts are rewritten in the mirror's copies only
(`Name = Name` default arguments that mean the global, and `CNiIterator<T>::s_ReturnError`
specializations without `template<>`). The real sources are never modified.

**Result.** All 403 C/C++ files in the CMake targets pass (baseline at commit time). An injected
type error is reported. Limits: files that include a C++ standard library header are checked with a
native `wchar_t` (GCC's library can't use the typedef); MSVC-only acceptance/rejection
differences remain; nothing is linked.

**Setup.** `apt-get install clang mingw-w64-i686-dev g++-mingw-w64-i686-win32`

## 3. Housekeeping

- `.gitignore`: ignore `__pycache__/`; untracked the accidentally committed
  `tools/__pycache__/mst_list.cpython-312.pyc`.
- `PORTING.md`: `-save-dir` in the usage line, a Saves section, the checker, and the save status.

## 4. GameCube-layout math audit (`tools/mathdiff/`, `dx/fdx8gcmath_vec.inl`, `dx/fdx8gcmath_mtx.inl`)

**Why.** The WINGC build runs `dx/fdx8gcmath_*.inl`, scalar code that was tools-only in the original
project and never ran in the shipped game. One bug there already froze turret yaw (`docs/handoff-history.md` section 24).
A text diff against `gc/fGCmath_*.inl` is useless (paired-single assembly), so this compares behaviour.

**What.** `tools/mathdiff/mathdiff.py` generates a program that calls every inline method both
`fdx8gcmath_*.inl` (GC layout) and `fdx8math_*.inl` (shipped SSE/x87 PC/Xbox code) define, 366 methods
of CFVec3A/CFVec4A/CFMtx43A/CFMtx44A/CFQuatA/CFTQuatA (all but constructors), plus the out-of-line
`CFMtx43A::ReceiveInverse`/`Invert`, on the same deterministic inputs, builds it for both layouts as
32-bit Linux code, runs both, and diffs the results. Clang needs two adjustments to build the SSE
code, both in the tool only: an MSVC-style `__m128` (the intrinsics map to the real SSE instructions),
and `mov reg, <reference parameter>` in inline assembly rewritten to `lea` (MSVC loads the reference's
address; clang loads the object).

Every difference was checked by hand against the retail GameCube code, which is what this port should
match.

**Fixed (GC layout disagreed with retail GC and with SSE):**
- `CFVec4A::ReceiveUnitXZ` kept `y` from the argument and left `w` untouched; retail zeroes both.
  Current callers use `CFVec3A` (correct), so this was latent.
- `CFMtx44A::Mul( rM, fVal )` went through `CFMtx44::operator*( f32 )`, which returns a `CFMtx43` and
  drops the `w` column (`[3][3]` became 0). Now scales all 16 elements. No current game callers.

**Explained, left alone (13; listed in `KNOWN_DIFFERENCES` in the tool):**
- `CFVec3A` XZ normalize (`UnitAndMagXZ`, `SafeUnitAndMagXZ`, `SafeUnitAndInvMagXZ`): retail GC sets
  `y = 0`, as the GC layout does; the SSE code keeps `y`.
- `CFVec4A` XZ normalize variants: commented out in retail GC, so no game code relies on them.
- `CFMtx43A::Mul33( rM, f )`: retail GC copies `rM`'s position, as the GC layout does.
- Shipped SSE bugs (Xbox/PC only; the GC layout matches retail GC): `CFVec4A::Sub( rV, f )` computes
  `f - rV`; `CFMtx44A::MulPoint`/`MulDir` on `CFVec4A` read `x, y` where they should read `z, w`.

The quaternion methods, matrix inverse and everything else agree within float tolerance (the SSE code
uses ~12-bit `rcpps`/`rsqrtps` approximations).

**Checked.** The tool reported both bugs before the fix and not after; syntax check of files that include
the math. **Verify in a run:** nothing specific to look for (no current callers of the fixed methods).

**Use it.** `python3 tools/mathdiff/mathdiff.py` (about 90 s); run it after touching `fdx8gcmath_*.inl`.
Requires `clang gcc-multilib g++-multilib`.

## 5. Failed level loads: tear down before releasing memory (`ma/App/ma/game.cpp`)

**Why.** `docs/handoff-history.md` section 24: after a level fails to load (e.g. `wewchold_01`'s Mini_Game table),
teardown "trips over other objects left in the released frame".

**Cause.** `_PostWorldLoadGameInit()` took a resource frame before creating the player bots, and on
failure released it at once. Both callers then run `game_UnloadLevel()`, which uninitializes every
level system: HUDs, debris and explosion pools, barter, level cameras, test bots, mesh-part manager,
level minigames, scripts, checkpoints. Everything created after that frame was torn down from released
memory. The HUD and debris/explosion lines added earlier worked around three of those objects.

**What changed.**
- `_PostWorldLoadGameInit()`'s error path no longer releases that frame (the frame variable is gone).
  `game_UnloadLevel()` tears the systems down in order while their memory is valid, then releases the
  level's frame. The earlier HUD/debris/explosion workarounds were removed; `game_UnloadLevel()` does
  those same calls.
- `game_LoadGenericDebugLevel()` (`-level`) called `level_Unload()` before `game_UnloadLevel()`. That
  freed the world and all post-world allocations before the level's systems were torn down. Removed:
  `game_UnloadLevel()` calls `level_Unload()` at the right point.
- `game_UnloadLevel()` passed `Level_aInfo[Level_nLoadedIndex].nLevel` to `CPlayer::UninitLevel` even
  when the level itself failed to load (`Level_nLoadedIndex == -1`, an out-of-bounds read). It now passes
  `LEVEL_DEVELOPMENT_LEVEL` then, which `CPlayer::UninitLevel` already treats as "no stats to save".

**Not changed (reasoned, not verified):**
- The `CFWorldAttachedLight` "Undeleted C++ class" FRES warning when a grunt fails to build is benign.
  The light was already removed from the world (`RemoveFromWorld()` right after `Init()`), and its memory
  is reclaimed with the frame. This is the engine's normal ownership of world-mesh lights created while
  a world exists.
- `level_Load()`'s own error path (`_LevelLoadError`) releases the world frame without
  `CEntity::RemoveAndDestroyAll()`, and `game_UnloadLevel()` later uninitializes the alarm and spawn
  systems created in that frame. The fix would mirror `level_Unload()`, but `RemoveAndDestroyAll()`
  requires a live `FWorld_pWorld`, and whether a failed WLD load leaves one needs a run to check.

**Checked.** Syntax check of `game.cpp`. **Verify in a run:** `-mission wewchold_01` (or any level that
fails after world load) exits or returns cleanly, with no crash or assert during teardown. Also check
that a normal level still loads and quits cleanly.

## 6. Co-op audit (`docs/coop-audit.md`, new; no code changes)

A source-only survey of what already works per player (the split-screen multiplayer path: HUD,
reticle, camera, viewports, audio listeners, inventory, input layouts) and every place found that
assumes one player, with file/line references and a suggested fix for each:
- the scripts' "player" (`Bot_GetPlayer` returns the current-player global);
- death rolling back the whole level to a checkpoint;
- progress only saved with exactly one player;
- one start point per campaign level;
- pause, barter, EUK display, swarmer/grunt/AI-brain use of player 0;
- minigames and boss levels written for one Glitch.

It ends with a suggested implementation order. Claims I could not confirm from source are marked as
such in the document.

## 7. Mouse menu design (`docs/mouse-menus-design.md`, new; no code changes)

A design for mouse-driven front-end menus, from reading `wpr_system.cpp`, `wpr_datatypes.h`,
`wpr_drawutils.cpp`, `msgbox.cpp`, `ftext.cpp` and `port/pc_input.cpp`:
- pointer state added to `pcinput_WindowMessage()`, and the conversion from window pixels to ftext's
  unit space (derived from `ftext.cpp` ~1680);
- tagged `ftext` prints that report their laid-out boxes, for accurate hit tests;
- clickable button prompts first (they make every screen usable), then hover and click on list items,
  wheel, right-click = back, and handling for sliders, the name keyboard, level select and message boxes;
- pointer art options (menu font arrow glyphs, the reticle textures, or a search of retail `tfm*`/`tfh*`
  textures), since the consoles had no pointer;
- rules so mouse and pads coexist, a step-by-step plan, and what to verify in a run.

## 8. HANDOFF.md rewritten as a current-state guide

The old `HANDOFF.md` (905 lines of chronological session addenda, sections 1-24) contradicted itself:
"never push" alongside the private remote, blockers long since fixed, two sections numbered 13, and a
commit trailer naming one specific model. It moved **verbatim** to `docs/handoff-history.md` (with a
note at the top), so nothing was lost. The new `HANDOFF.md` has:
- the repository rules (retail data check, pushing, trailers, CRLF);
- the layout, build/run, debugging and retail-data tools;
- a section for Linux/cloud sessions;
- what works, what is changed but unverified, a prioritized open list (including the resolved
  checkpoint and emitter items from section 24 removed), and the pitfalls that cost time before.

## 9. Script event masks beyond event 31 (`FEventListener.h`, `FScriptSystem.cpp`, three minigames)

**Why.** Event masks are `u64`, but every site built the bit with `1 << n` on a 32-bit `int` (the
C4334 warnings in `PORTING.md`; noted as "not live yet" in the history, section 21). For events 32-63:
- the GameCube's PowerPC `slw` gives 0, so those events were never delivered;
- x86 wraps the shift count, so event 40 fired as event 8, and event 63 produced
  `0xffffffff80000000`, which matches event 31 and every event bit above it.

**What.** `fevent_Bit( n )` in `FEventListener.h` returns exactly the GameCube value: the same
sign-extended bit as before for 0-31 (checked against the old x86 result for 0, 5, 30 and 31), and 0 for
32 and up or negative. It's used at every site: script `event_SetNotify`/`event_StopNotify` and dispatch in
`FScriptSystem.cpp`, `CFEventListener::SetNotify/StopNotify`, and the listener registrations in
`GeneralCorrosiveGame.cpp` (4), `SpaceDock.cpp` and `ColiseumMiniGame.cpp`. `FScriptSystem.h` now
includes `FEventListener.h` for it.

I chose GameCube behaviour over delivering events 32-63 properly because the retail game shipped and was
tested without them.

**Not changed.** `fcoll_kDOP.cpp`'s C4334 shifts use a kDOP vertex index (`FKDOP_MAX_VERTS` = 24), so
they never reach 32.

**Checked.** Syntax check of the changed files and the script system; a native test of the helper.
**Verify in a run:** nothing new expected; a level with 32+ script events would now ignore the upper
ones instead of misfiring.
