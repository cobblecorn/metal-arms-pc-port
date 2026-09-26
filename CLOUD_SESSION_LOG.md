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
