# Yellow RAT HD texture test - 2026-10-07

User approved testing the generated yellow/blue RAT repaint on the actual model.
Installed an optional archive selected with the existing `-mst` argument; no
engine rebuild, live EXE replacement, save edit, or game launch was performed.

Launch `D:\Desktop\Metal Arms - Yellow RAT HD Test.cmd`, after closing the
current game. It opens normal menus with the current root Release EXE and
`gamedata/files/rat_hd_yellow.mst`. Replay **5 RAT race** to inspect yellow RATs;
that variant also loads in Wasteland Chase. Regular launch uses retail textures.

- `trdrrata_01`: 1024 x 512, seven mips.
- `trdrrata_02`: 512 x 512, seven mips.
- GC RGBA8 encoding retains the source material alpha exactly at mip zero,
  repeating each original alpha sample 4 x 4. Smaller mips are box filtered.
- All pixels in all mips match the production `gcdata_DecodeTga` output exactly.
- The independent archive changes only the two resource directory entries,
  archive length and appended replacement payloads. All original bytes outside
  those fields and all 12,046 unrelated entries were compared unchanged.
- Retail archive SHA256 remains
  `4dd3516e49a32dd5d47ad992cbd5e4faf33b4505fc1b742a6f890aff76045ab0`.
- Test archive SHA256:
  `6716f2b2353f619a60f42742f1deea2a15d3e9454c27c99a68bbf62260614026`.

Artifacts/manifest: `build/export/Yellow_RAT_HD_Test/`. Reproducible packer and
offline native decoder validation: `tools/package_rat_hd_textures.py`. The
packer refuses to overwrite an existing test archive. Original generated
previews/prompts remain in `build/export/RAT-HD-yellow/`.

In-game UV fit, appearance and GPU rendering await user testing. AI repainting
can shift atlas details; unchanged model UVs do not prove the new artwork fits.
The full archive is a local test convenience. A public pack should distribute
only replacements with a patcher or use a dedicated texture replacement loader.
