# Shop upgrade crash repair and native purchase tests

## Cause

The earlier EUK pickup repair incorrectly assigned all 23 retail EUKs one
collectable ID. The shop's catalog lookup uses that ID as product identity,
so weapon families and upgrade variants could resolve to the first EUK entry.
`TypeToHUDString()` also assumed an EUK was present in the item repository,
although retail EUKs are independent goodies. Selecting one could dereference
a null item while formatting its caption. This is a regression in the earlier
EUK repair. The reporter's exact upgrade, version, and crash log remain unknown.

## Repair

- Reserve 23 distinct IDs after the existing weapon IDs: seven three-variant
  families, followed by two scope variants. Existing weapon/inventory IDs and
  the disk save format are unchanged. All world/scoped pickup paths recognize
  the whole range and retain one-step retail upgrade behavior.
- Shop eligibility checks the buyer's owned weapon and next upgrade. Numbered
  stock only offers its matching next level, and EUKs cannot be offered as ammo.
- EUK captions resolve the matching weapon and upgraded level through the item
  repository. Missing metadata returns failure instead of dereferencing null.
  Shop formatting uses initialized buffers and skips failed caption formatting.
- EUK displays use upgrade-box scaling; their existing weapon-icon remapping
  remains active. Existing solo/co-op grants, claim masks, and private shop
  ownership remain in place.
- The other chat's resource version changes to 1.0.5 are preserved.

## Native gameplay evidence

Executed `tools/test_vendor_upgrades_native.py` with:

1. `--players 1 --owner 1`
2. `--players 2 --owner 1`
3. `--players 2 --owner 2`

Each separate game process loaded `WEDMmines03`, opened the real vendor session,
displayed all 23 retail EUK catalog entries, and bought Rivet Gun 2/3 and Ripper
2/3 using the normal purchase-input latch. That is 69 selections and 12 actual
purchases across the three runs. The fixture puts real retail catalog entries
into the active offer slot and supplies test washers/weapons; it does not
bypass the shop purchase handler or apply the purchased upgrade itself.

All runs completed without logged crashes, assertions, or failed checks.
Purchases raised the live weapon level and the inventory item fields used by
saves, charged exactly one price, and left the co-op partner's wallet/weapon
unchanged. Exiting restored the shopper's visible model and entity control.
Snapshots were inspected, including P2's acquired-upgrade message, caption,
and wallet alongside P1's independent view. Saves/settings were isolated under
`build/test-saves/vendor-upgrades-*`; only test-owned game processes were closed.

Logs:

- `build/logs/vendor-upgrades-1p-owner1-20261012.log`
- `build/logs/vendor-upgrades-2p-owner1-20261012.log`
- `build/logs/vendor-upgrades-2p-owner2-20261012.log`

The opt-in test lives in `port/coop_regression.cpp`. A read-only readiness
helper mirrors the actual shop input condition. Neither runs during ordinary
play without `MA_PORT_TEST_COOP_POLISH=shop-upgrades1` or `shop-upgrades2`.

## Other validation

The Release build passes. Previous checkpoint/EUK fixtures still pass 1,774
checks, and the Slosh pickup fixture still passes 118 PC checks while reproducing
the old failure in its legacy configuration. Whitespace diff check passes.
Patcher packaging remains the user's step.

## Deployment

Installed the tested EXE and matching PDB into `D:\Documents\Metal Arms PC Port`.
Build/installed SHA256:
`BB6A369D9917E7940DE0DB464C2A7573F2AEB93DD24BE9B9DA564A6DC1DB42ED`.
Previous 1.0.5 binaries and a manifest are backed up under
`build/backups/vendor-upgrades-20261012-*`.
Patcher input is the updated `build/Release/ma_port.exe`; no patcher was built.
The test processes are closed. Four compressed evidence frames per run remain
in the matching `build/shots/vendor-upgrades-*` directories; intermediate BMPs
were removed to avoid retaining hundreds of megabytes of duplicate frames.
