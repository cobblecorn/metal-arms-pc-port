Metal Arms: Glitch in the System - PC port 1.0.11
==========================================

This download contains only the PC port's program. It has no game data: you need your own copy
of the USA GameCube release of Metal Arms (disc ID GM5E7D) as a disc image.

Updating an existing installation
---------------------------------
Close the game and extract this download over your existing Metal Arms PC Port folder,
replacing its program files. Keep your gamedata, textures and save/profile folders.
You do not need to extract the disc again if the game's data is already installed.

Included in 1.0.11
-----------------
- Campaign co-op: Secret Rendezvous recognizes the actual security-panel operator;
  tether targeting uses its owning player's camera; story-gift batteries and weapon
  upgrades use per-player collection; the spinning washer icon fits its HUD counter.
- L3 Rivet Gun mouse controls: right-click hold fires/heats its remote-explosive rivet;
  release detonates it. A quick tap still uses the selected grenade or scope. Existing
  left-click behavior and controller input remain available.
- Native Exavolt mod support: converted resource/CSV ZIPs, custom multiplayer maps,
  loading movies/music, the native randomizer and the optional Super Mining Lasers
  adapter. Get the separate Exavolt PC mod pack to manage mods. This runtime already
  includes mod support: skip Exavolt's Install mod support button for this release.
  No mods or randomizer options are enabled by launching the base game normally.
- Retains the earlier Mining Laser quarter-bar charge rule, Slosh aim, Level 15 console
  safeguards, Miner death effects, co-op elevator/checkpoint safeguards and prior fixes.

The reported Level 2 enemy-lift stall was not reproduced in the earlier 2P/4P checks;
no new level-specific elevator patch is claimed. Focused native checks have passed;
this is not a completed 42-mission manual test with every co-op/mod combination.

Setup (once)
------------
1. Extract this folder anywhere you like.
2. Run ma_setup.exe. Choose your disc image with Browse (or drag it onto the window, or onto
   ma_setup.exe), then press Install.
   - .iso and .gcm images work as they are.
   - .rvz, .gcz, .wia and .ciso images are converted with DolphinTool.exe, which comes with the
     Dolphin emulator (dolphin-emu.org). Setup finds an installed Dolphin by itself; for a
     portable Dolphin it asks you to point at DolphinTool.exe in the Dolphin folder.
     You can also convert the image to .iso in Dolphin yourself: right-click the game, Convert File.
3. Setup copies the game's files into gamedata\files in this folder and checks every one against
   the retail disc. A modified, damaged or different-region image is refused. It takes about a
   minute and needs roughly 1.3 GB (about 3 GB while converting a compressed image).
4. Press Play, or start ma_port.exe from now on.

Keep ma_port.exe, binkw32.dll and the gamedata folder together.
If setup has a problem, ma_setup.log in this folder says what happened.

Display, field of view, anti-aliasing, controls and the rest are in the game's own settings menus.

Texture packs
-------------
Extract a texture pack into the "textures" folder here (see textures\README.txt). Images named after
the game's textures replace them; nothing else needs setting up. With the Mapping Project pack:

    Metal Arms PC Port\
    |-- ma_port.exe
    `-- textures\
        |-- dev-dump\
        `-- dolphin-mapped\

About antivirus warnings
------------------------
A few heuristic and machine-learning scanners may flag ma_port.exe or ma_setup.exe. They are new,
unsigned programs that read the keyboard and mouse directly (the game), restart themselves to
apply display settings (the game) or start another program (setup running DolphinTool), which
these scanners treat as suspicious.
