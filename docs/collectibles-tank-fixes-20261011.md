# Secret chips, retail weapon upgrades, and vehicle recovery reset

Based on release 1.0.4 (`430bf97`). Latest installed game inspected at
`D:\Documents\Metal Arms PC Port`; original EXE SHA256:
`63A522C272CBB97A3E7BC15BAE25BAB713221DAE7E4F983D8503448161C9DD9C`.

## Repairs

- Secret-chip pickups and inventory already restore on checkpoint rollback, but
  `CPlayer::m_GameStats.m_SPStats.m_nSecretChipsCollected` did not. The global
  checkpoint now saves/restores this counter for all four player slots. This
  prevents collecting the same respawned chip repeatedly across deaths from
  inflating the level-completion count. Disk save format is unchanged. Already
  inflated completed-level records are not retroactively changed.
- Retail `goodies.csv` defines 23 separate EUK pickups absent from the older
  source's item classifier and item repository. They now map to eight weapon
  families, upgrade an owned weapon once, persist its new item/weapon level,
  and show the normal upgrade message/effect. Unowned or maxed weapons cannot
  consume these pickups. World pickups use the existing per-player co-op claim
  masks; scripted grants use the existing scoped recipient. Applies in solo
  and co-op. Other weapon damage/ammo behavior is untouched.
- EUK world/HUD meshes now use the existing weapon atlas offsets on texture
  layer 50. The scope keeps its separate mesh mapping.
- Physics `_SetDefaults()` now invalidates the cached recovery state. Reused
  vehicles cannot recover an old position from before their reset. Existing
  ground-normal, wheel-angle, and corrupt-state safety checks remain intact.

## Retail evidence

Decoded `gamedata/mst/goodies.csv` and `itemlist$.csv` confirm EUK names and
owned-weapon item tags (including `RLauncher L1` for `euk rocket`). Retail DOL
upgrade routine at `0x8017eb48` finds the matching owned weapon and, at
`0x8017ebd4` through `0x8017ec00`, grants its current upgrade level plus one.
This applies to numbered EUKs too; their suffix does not cause a level jump.
The enum-to-family helper is at `0x8017f1c0`.

## Validation and limits

- `python tools/test_checkpoint_euk.py`: 1,774 offline checks using extracted
  production methods. Covers 100 death/restore cycles, four player slots,
  later checkpoint retention, all authored EUK names/base inventory tags,
  upgrades through each weapon's cap, item persistence, recipients, untouched
  ammunition, atlas offsets, and physics recovery validity.
- `python tools/test_vehicle_math.py`: 118 production ground/wheel checks pass.
- `python tools/test_collectable_skiplist.py`: legacy overflow reproduced;
  180,016 PC pickup collision checks pass.
- Release `ma_port` build and diff whitespace check pass. Build log:
  `build/logs/collectibles-fixes-build-20261011.log`.
- These are source/offline checks, not in-level gameplay proof. The reported
  level-3 tank pause has not been reproduced; the reporter's pause timing and
  entry/driving state are still unknown. There is no timed immobilization in
  the recovery guard. Test entering/driving that tank and check for
  `Port: physics object ... blew up` in the game log if it pauses again.
- No user profile, texture pack, or patcher package is modified by this work.

## Deployment

Installed the Release EXE and matching PDB into `D:\Documents\Metal Arms PC Port`
after verifying the game was closed and the original EXE had not changed.
Installed EXE SHA256:
`4DF9F413935211B5B6FC57EDEA61B3BC2D61EFACDB8DCB0CFC0C2EB0BD8349AB`.
Original files and deployment manifest are retained under
`build/backups/collectibles-tank-20261011-*`.
The patcher was not built; its existing packaging input is the updated
`build/Release/ma_port.exe`.
