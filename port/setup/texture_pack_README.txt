Metal Arms Mapping Project - texture pack for the Metal Arms PC port
=====================================================================

Textures by zstorm4 and the Metal Arms HD team (the Metal Arms Mapping Project), used and repackaged
for the PC port with zstorm4's permission. All credit for the texture work goes to them.

What it is
----------
The Mapping Project was made for the Dolphin emulator. This copy is rearranged for the PC port:
- textures\dolphin-mapped: the project's mapped and enhanced textures, renamed from Dolphin's
  hash-based names to the game's own texture names.
- textures\dev-dump: images from the project's included developer dump, for the textures the
  mapping does not cover.
Where both had a texture, the mapped version is used. Images that do not fit the game's texture
shapes were left out, so the game keeps its original for those.

Install
-------
Extract this zip into the folder with ma_port.exe, so that its "textures" folder merges with the
game's. Afterwards the game folder should look like this:

    Metal Arms PC Port\          (the folder with ma_port.exe)
    |-- ma_port.exe
    |-- ma_setup.exe
    |-- binkw32.dll
    |-- gamedata\
    `-- textures\
        |-- README.txt
        |-- dev-dump\            (images, straight inside)
        `-- dolphin-mapped\      (images, straight inside)

If you copy the folders by hand, put dev-dump and dolphin-mapped inside the game's "textures"
folder. Start the game; ma_port.log lists every replacement ("PORT-TEX").

To remove it, delete the dolphin-mapped and dev-dump folders from "textures".
