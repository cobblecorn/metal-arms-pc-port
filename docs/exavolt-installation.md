# Use Exavolt PC with an existing game installation

Exavolt PC uses the game folder you select. It does not require another copy of
the game's data or a new disc installation. The manager is distributed separately
from this source repository.

## Setup

1. Close the game. Inside your existing Metal Arms PC Port folder, create an
   `Exavolt PC` subfolder and extract the manager package's contents there.
2. Open `ExavoltPC.exe`. Beside **Game folder**, click **Browse** and choose the
   existing game folder containing your usual `ma_port.exe`.
3. **On game version 1.0.11, skip Install mod support.** This version already
   has the required native mod API.
4. Add the converted PC mod ZIPs, or enable the included entries if using the
   community pack. Start with one mod. The Randomizer tab creates a preset ZIP.
5. Click **Launch modded**, then select a mission or multiplayer match normally.

For example:

```text
Metal Arms PC Port/
  ma_port.exe                 <-- your installed game; select this folder
  binkw32.dll
  gamedata/
  textures/                   <-- optional HD pack
  Exavolt PC/
    ExavoltPC.exe              <-- open this to manage mods
    assets/
    runtime/
      ma_port.exe             <-- optional support payload for older builds
      ma_port.pdb
      runtime.json
    user_data/
```

The manager may also live elsewhere: use **Browse** to select the actual game
installation. Keep the manager in a writable location. Do not select its
`runtime` subfolder as the game folder.

## Why does the manager include another ma_port.exe?

The `runtime/ma_port.exe` is an optional native-mod-support upgrade for known
older game builds. **Launch modded** runs the EXE in the selected game folder;
it does not launch that bundled payload.

Exavolt PC 0.3.1 bundles an earlier 1.0.10 runtime with API v4. Game 1.0.11
already supports that API, so do not manually copy the older runtime over it.
The installer checks exact supported build hashes and refuses an unrecognized
build instead of downgrading it.

For a supported older build, **Install mod support** backs up the existing
EXE and any existing PDB in `exavolt-runtime-backups/<old-exe-sha256>/`, then
replaces the program pair in the selected installation. It leaves retail data,
texture packs and normal saves in place. Close the game before doing this.
If rollback is needed, restore the backed-up program files together.

## Saves and existing manager users

Modded launches use separate saves under
`Exavolt PC/user_data/profiles/<profile-id>/saves/`. This is the same game
installation with isolated modded progress. Changing mods, load order or seed
can select a different profile. Keep the same setup to continue the same run.

**Launch vanilla**, or opening the installed game directly, uses normal saves.
It does not undo a runtime upgrade. Back up saves before manually transferring
them between profiles, and close the game first.

When replacing an existing manager package, preserve its `user_data` directory.
Extract a fresh community pack into a new folder so its default settings do not
overwrite your enabled list or old modded runs.

Original Exavolt: [dj0wns and contributors](https://github.com/dj0wns/Exavolt).
Original Randomizer: Avohkii, with generator contributions by dj0wns. Converted
mods retain their original authors; the PC compatibility code is separate work.
