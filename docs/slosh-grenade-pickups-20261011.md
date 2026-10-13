# Slosh Magma Bomb pickups

## Cause and repair

`CCollectable::_IsPickupAllowed()` already permits Slosh (BOTCHEMBOT) to collect
Magma Bombs. `CInventory::SetToSlosh()` creates his Magma Bomb inventory weapon,
and `CBotSlosh::ClassHierarchyBuild()` links that weapon to its inventory item.
However, `_GiveWeaponAmmoToPlayer()` only added ammunition inside its Glitch
branch. Slosh fell through to `return TRUE` without receiving any ammunition,
so the pickup could be consumed while the grenade count stayed unchanged.

The PC path now refills Slosh's existing Magma Bomb clip and synchronizes the
inventory count used by the HUD/save. Full clips, zero-ammo grants, missing or
uncreated weapons, and mismatched item links fail without consuming the pickup.
Other Slosh weapon grants are not added. His existing pickup restrictions stay
in place; Glitch's primary/reserve and secondary/clip paths are unchanged.

Pickup candidate selection also skips a Slosh whose Magma Bomb clip is full,
so the nearest full Slosh cannot repeatedly prevent another co-op player from
collecting the same pickup. Solo and co-op use the same repair.

## Verification

- `python tools/test_slosh_pickups.py`: compiles the actual production ammo
  grant, pickup permission/selection, clip addition, and item-notification
  methods with small engine fixtures. The legacy path reproduces success
  without ammo (7 checks). The PC path passes 118 checks covering refill,
  actual cap clamping, recipient isolation, full/missing/mismatched weapons,
  solo/co-op selection, existing claim masks, and unchanged Glitch ammo paths.
- `python tools/test_checkpoint_euk.py`: all 1,774 previous checks still pass.
- Release `ma_port` build and whitespace diff check pass. Build log:
  `build/logs/slosh-pickup-build-20261011.log`.
- These are offline/source checks. The playable Slosh mission was not launched
  for a live pickup test. No user save or texture pack was changed.
- Patcher packaging remains the user's step; this task builds only the game EXE.

## Deployment

Installed the Release EXE and matching PDB into `D:\Documents\Metal Arms PC Port`,
with the previous binaries and a manifest backed up under
`build/backups/slosh-pickups-20261011-*`.
Verified installed/build EXE SHA256:
`CC3F30D12AB4D1E9D89ECF07A508927F2FA106EB8628B9655FA66837E8E860BC`.
This supersedes the build recorded in `collectibles-tank-fixes-20261011.md` and
includes all of that build's repairs. Patcher input remains
`build/Release/ma_port.exe`; the patcher was not rebuilt here.
