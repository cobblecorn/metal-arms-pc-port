Texture packs go in this folder.

Drop image files (or whole folders of them) here. An image named after one of the game's textures
replaces it the next time the game loads that texture, for example "trdggltch00.png". Folders inside
this one are searched too, so a pack can be extracted here as it is.

- Formats: TGA, PNG, BMP, JPG. When a texture has several, TGA is used first (it keeps transparency).
- A replacement must have the same shape as the original (a 256x128 texture can be replaced by
  512x256 or 1024x512, not by 512x512). Sizes are rounded to a power of two, at most 2048.
- Transparency follows the original: a texture without any stays solid; one with transparency
  keeps the original's where the replacement has none.
- ma_port.log lists every replacement used ("PORT-TEX") and any refused, with the reason.

To use a different folder for one run, start the game with: ma_port.exe -textures "<folder>"
