# Nearby-group encounter tripwires (2026-10-06)

Co-op players no longer have to cross the identical thin volume for **37 audited
one-shot enemy/map events across 14 missions**. A real original player crosses;
all living original players must fit within a **12-world-unit group diameter**,
with no pair more than **4 units apart vertically**. Any player can cross first.
Ordinary solo and PvP handling is unchanged.

This changes readiness, not geometry. Trigger volumes, script prerequisites,
spawn counts, floor-pad behavior and checkpoint behavior remain unchanged.
Transport, elevators, exits, cinematics, repeatable triggers, named-actor and
possession events, and unlisted events keep the existing policy. No individual
elevator changes are included.

A held encounter can release when partners approach, provided someone remains
inside its original volume or within 4 units of a real crossing endpoint. Long
warps through earlier triggers do not create distant anchors. Grouping elsewhere
does not drain queued encounters. Existing actual arrival history still works;
this does not cancel encounters or replace scripted progression requirements.
No synthetic arrival bits, automatic partner movement, or enlarged volumes.

## Audited scope

`coop-encounter-trigger-list-20261006.json` records each exact mission/trigger,
retail script and source line, and branch native calls. The audit inspected 196
available scripts in the 42 campaign CSVs. One referenced retail script,
`xewr2winlvl.sma`, is absent from the local assets; none of its triggers is relaxed.
This is a conservative list, not a guess from names such as trigger, bridge or
spawn. Mixed branches that move story actors, operate doors/lifts, start scenes,
or set unverified asynchronous work/timers were left strict. Alarm networks can
also operate doors and notify other scripts, so those have not been blanket-added.

The loaded table includes development entries before the campaign: the new
policy checks `Level_nCount` and the entry's campaign `nLevel`, rather than treating
`Level_nLoadedIndex` as a 0..41 campaign ID.

## Validation

- Normal-production gate methods: **1,871 checks pass** for 2-4 players. Includes
  all listed names/first-player orders, far/vertical/chained groups, gathering
  elsewhere, large warps, thin crossings, resets, and strict progression/solo/PvP.
- Release candidate builds successfully in `build/encounter-candidate`; a separate
  tree avoided replacing the executable being used for the user's current run.
- Real `WEMCcity_05` native fixture: **9 PASS / zero FAIL**, no assertion or crash.
  Titan 1 spawns with only P1 crossing and P2 outside; Titan 2 stays hidden with
  distant P2 and spawns when P2 approaches without crossing. Final evidence:
  `build/logs/encounter-group-native-20261006.log`.
- Opt-in fixture `MA_PORT_TEST_COOP_POLISH=encounter-group`, runnable with
  `python tools/test_coop_encounter_native.py` after installation. Its saves are
  isolated; runner closes only its own process. Never use it for normal play.
- Native testing covers those two real spawns, not every listed campaign event.

## Installation

Desktop and staged Release copies contain EXE `A850549E5933B145141618153B7834EE61DAAE87BB52FE20CF3047FC6D9915CA` / PDB `C24AF745F28FCC6982C6F249187FB0972F64E7F0987C5EFB239B07648AE20CA6`.
The root game PID 39124 was left running with its previous build. Hidden one-shot
installer PID 12060 waits for that process to exit before replacing root EXE/PDB;
it does not close the user's game. Current authoritative status is
`build/logs/encounter-groups-install-20261006.json` (`root_install`). It refuses to
overwrite a newer root binary if another update supersedes this one.

Backup: `D:\Documents\metal arms source port\build\backups\pre-encounter-groups-20261006-121521`.
No user profiles/checkpoints were modified. All isolated test processes are closed.
One-shot build scripts `patch_encounter_groups_20261006.py`,
`refine_encounter_groups_20261006.py`, `add_encounter_native_fixture.py` and
`install_encounter_groups_20261006.py` must not be rerun.
