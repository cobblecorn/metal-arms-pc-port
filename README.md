# Metal Arms: Glitch in the System - PC port

Native Windows source port of Metal Arms using the retail USA GameCube data
(disc ID `GM5E7D`). The current source version is **1.0.11**.

This repository contains the engine, game, PC compatibility code and regression
tools. Supply your own supported game data; disc images and extracted retail
assets are not included.

## Recent changes

- Mining Laser L2/L3 and possessed-MIL charge shots: mouse right-click hold
  charges, release fires; a quick tap still uses the selected grenade/item.
  Controllers retain analog primary-trigger charging and normal full-pull fire.
  Each charged burst uses 25% of a full energy bar on every player/input path.
- Rivet L3: right-click hold fires and heats the embedded explosive rivet;
  release detonates it. Existing left-click and controller behavior remain.
- Shop ammo delivery, personal co-op batteries/story gifts, weapon-selector
  movement, Slosh aim, vehicle weapon profiles and tank machine-gun volume.
- Level 15 arcade possession/fabrication recovery and guard-scene fallback,
  friendly Miner-model death effects, and the Mil City doorway slope trap.
- Co-op Secret Rendezvous panel/tether camera ownership and washer HUD layout.
- Native mod resource, CSV, map, media and randomizer support. Exavolt PC is a
  separate manager; its app and mod downloads are maintained separately.

See [1.0.11 changes and verification limits](docs/release-1.0.11.md) for the
source entry points, intentional gameplay differences and remaining reports.

## Install or update

For a packaged build, follow the [setup instructions](port/setup/README.txt).
When updating an existing installation, close the game and replace the program
files in that installation. Keep `gamedata`, textures and save/profile folders;
there is no need to extract the disc again.

To use the separate mod manager with your existing game folder, follow
[Exavolt PC installation](docs/exavolt-installation.md). Game version 1.0.11
already includes native mod support; skip the manager's optional runtime installer.

## Build from source

Use Visual Studio 2022 with the C++ workload and x86 tools, CMake 3.20+ and
Python 3. The game is currently **32-bit**.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
cmake --build build --config Release --target ma_port ma_setup ma_input_tests -j 4
.\build\Release\ma_input_tests.exe
```

Additional source/data regression scripts are under `tools/`. Some require
locally extracted retail tables or scripts. Native test clients are opt-in and
use isolated saves; offline checks alone do not prove a full mission playthrough.

[PORTING.md](PORTING.md) contains the detailed port/build notes.
[HANDOFF.md](HANDOFF.md) and [docs/](docs/) retain engineering history.
