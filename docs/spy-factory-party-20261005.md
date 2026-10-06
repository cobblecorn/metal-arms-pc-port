# Spy Factory party stages (2026-10-05)

The instructor stage previously relocated, disarmed and hid only P1's HUD.
Checkpoint zero still contained partners' opening-room spawns. The factory's
custom stage restore moved P1 back to training while leaving those partner
positions intact. Its execution/crusher failure also assumed a solo death
would restart the level, which left co-op waiting indefinitely for a checkpoint.

## Changes

- Factory camera transitions snapshot the owning camera controller rather than
  a borrowed spectator view. This mission consistently uses P1 as its scripted
  actor, including after a death/respawn; other campaigns retain the existing
  living-player fallback.
- Search, instructor, combat and programming transitions synchronize the active
  co-op team's HUD, allowed attacks and scripted equipment. Checked regrouping
  brings partners forward when those later stages teleport P1, including the
  programming machine's exit and restore.
- Regular instructor play gives every player a separate lane. The shared
  instruction is validated against each player's own body, jump input, timing
  and lane-relative destination. Each player receives a destination hologram.
  Partners keep their normal controls while performing the commands.
- Instructor failures retry the team without replaying the long Shhh briefing.
  An actual player death also recovers the team after the death transition.
  Stage restoration overrides partners' stale opening-room spawn positions.
  The scripted NPC demonstration and original solo failure behavior are retained.
- Leaving instruction restores normal bot collision/camera behavior, regroups
  the party in combat and equips each player with the mission launcher.
- Partners can open/search lockers with their own interaction input and proximity.
  A chip held by any active partner satisfies the guide's team objective.

The first claw/body assembly puzzle and authored cinematic actors remain P1-owned.
Partners retain independent body rigs and regroup at later scripted transitions.
This is not a claim that every remaining authored factory interaction has been
converted into a separate puzzle for each player.

## Verification

- Release build succeeds. `spy-party-release-native-20261005.log`: **31 PASS,
  zero FAIL/assertions/crashes**, using retail `WECFfacty01` with two players.
  Checks include the real checkpoint-zero restore, matching HUD/equipment,
  independent jump acceptance, detection of P2's incorrect turn while P1 is
  correct, an actual P1 death with P2 alive, and the authored exit into combat.
  Native snapshots confirm partners reach the training room and later combat.
- This targeted fixture enters the instructor through an explicitly enabled
  test-only stage entry. It does not certify a fresh uninterrupted run from the
  first claw through the entire remaining mission or every random dance sequence.
- `spy-party-solo-smoke-20261005.log`: **9 native PASS**, natural opening
  transmission/control handoff; no assertions or crashes.
- `tools/test_spy_party.py`: **149 production-method checks** for solo and
  2–4-player HUD/fire/objective behavior, including a later player's chip and
  missing inventories. Physical 3/4-controller dance gameplay remains untested.
- Existing production checks remain passing: guide **13**, checkpoint/placement
  **296**, campaign playthrough **328**.
- Both native runs report zero audio, allocation, or script errors.

## Installed build

Installed into `build/Release` at **2026-10-05 12:12 EDT**; EXE/PDB hashes match
the tested staged files. The Bink DLL is unchanged. Previous build backup:
`build/backups/pre-spy-party-20261005-121217`.

- EXE SHA256: `479D60854D576F7444D3E10B09ADF5D4D89F79B351F54FFD5CCCD8687EEE0452`
- PDB SHA256: `9D4015732AB143D4AF903E3E23F0884C7A6B0E4D6ECB05B1ECFD300E7A8E4B0B`
- Install record: `build/logs/spy-party-install-20261005.json`

Temporary CMake Release output overrides were removed. Normal builds target
`build/Release` again. Only isolated test processes/save directories were used;
the user's campaign save files were not edited.
