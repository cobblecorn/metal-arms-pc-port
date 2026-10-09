# Workspace cleanup — 8 October 2026

The workspace decreased from **94.65 GiB to 10.07 GiB**, freeing **84.58 GiB**
(90.8 GB in decimal units). The figures include the compressed evidence retained
below. No game was launched and no game, profile, or gameplay source was edited.

## Removed and compacted

| Generated files | Original size | Retained evidence |
| --- | ---: | --- |
| 21,469 automatic `shot_NNN.bmp` captures | 83.25 GiB | 1,200 selected frames in 255 lossless ZIPs, 1.40 GiB |
| 1,091 secondary compiled build outputs | 1.75 GiB | Source, build configuration, diagnostic logs, and test fixtures remain |
| 142 older executable/debug-library backups | 1.66 GiB | Complete lossless backup ZIP, 684.3 MiB |

Captured frames retained include the first, middle, and last frame in each test
folder, plus filenames referenced by the Markdown notes. All PNG captures and
other non-automatic bitmap assets remain. Repeated raw capture frames were deleted.

Older binary backups are in `build/backups/older-builds-20261008.zip`, retaining
their original folder names. Its embedded manifest records original paths, sizes,
and SHA256 hashes. The latest two rollback sets remain unpacked:

- `build/backups/pre-retail-mp-blaster-20261008-150545`
- `build/backups/pre-movement-cheats-20261007-030840`

## Preserved

- All source, Git history, current working changes, notes, scripts, and test source.
- Main Release and Debug builds, their symbols, and main incremental build caches.
- Release ZIPs and patcher/package inputs.
- Retail disc image, extracted game data, HD texture pack, asset/model/audio exports.
- All save files, test-save fixtures, profile backups, and the downloadable complete save.
- Diagnostic logs, JSON reports, manual PNG captures, and selected automatic captures.

The remaining space is approximately 5.20 GiB in `build` (including asset exports,
archives, current builds, and logs), 2.27 GiB in `gamedata`, 1.24 GiB in the HD
texture pack, 1.02 GiB in the original disc image, and 0.30 GiB in Git history.

## Recovering archived evidence

Each former automatic capture folder has a `capture-evidence-20261008.zip`.
Extract that ZIP into its containing folder to restore the selected BMP filenames
used in older notes. `build/cleanup-records/capture-evidence.json` maps the retained
original image paths to their archives and entries.

To restore older binary backups, extract `build/backups/older-builds-20261008.zip`
into `build/backups`. This restores all 142 older binaries. Individual folders can
also be extracted when only one rollback version is needed.

## Verification and audit

ZIP entry names and CRCs passed before original files were removed. Every deletion
target was checked as a regular file inside the resolved workspace `build`
directory, with the inventoried size and modification time; linked paths were
rejected. No recursive directory deletion was used.

All **1,856 protected source, notes, and current release files passed SHA256
verification**. Every original file outside the deletion plans remained present;
the inventory scan reported no errors or unplanned missing files. The current
Release EXE remained SHA256
`5218CA2E45EE7D4164E52D607470691431BBF1780C0E9C9C2E8FBCE378595F24`.

Inventories, deletion plans and receipts, evidence manifests, protected hashes,
and the verification summary are preserved in `build/cleanup-records`.
