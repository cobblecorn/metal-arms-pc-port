# Sniper's Lair mission completion (2026-10-06)

Retail `WECRruins02` / campaign level 28 finishes after the three Agent Goff
parts, without a separate exit. The local retail `xecr_xplode.sma` subscribes to
`goffpickup` (source lines 61-62), increments its checkpointed script counter
(line 165), and invokes `Game_WinLevel("Next")` when that counter reaches three
(lines 167-170). The older source never generated this event for these pickups.

`collectable.cpp` now identifies `goffhead`, `goffleg`, and `gofftorso` after a
successful campaign pickup, skips the unsupported generic inventory repository
entry, and queues one `goffpickup` notification. Their normal pickup sound, HUD
presentation and shared consumption remain in place. This repairs solo as well
as co-op. Competitive modes and other pickup types keep their existing paths.
No retail assets or mission script binaries are edited.

## Validation

- Release build succeeds in `build/encounter-candidate`.
- Native solo and two-player tests collect all three actual authored world props
  through normal collision/pickup processing. P1 collects the head; in co-op P2
  collects the leg and torso. Both runs produce exactly three notifications and
  reach the ending movie and fully loaded mission-complete screen.
- No premature win after the first two parts, duplicate events, Fang assertions,
  or crash in those runs. Native logs: `build/logs/goff-native-1.log` and `-2.log`.
- The isolated helper skips only its own ending movie via the movie status handler.
  It stops at mission completion: quick-launch missions normally return to the
  debug level picker, unlike a campaign started through the regular menu.
- Four-player validation could not start gameplay: the normal reconnect screen
  requires a fourth controller. It is not recorded as a pass.
- Opt-in native fixture `goff-parts`; helper `tools/test_goff_parts_native.py`.
  It uses isolated saves and closes only the process it launches. Ordinary play
  does not run the fixture or skip movies.

## Deployment

Immutable EXE/PDB payload: `build/goff-install-20261006`. Desktop and staging
copies installed with backups. Root `build/Release` installation waits for the
existing user run (PID 10784) to exit; it verifies the old EXE hash before replacing
it and will not overwrite a newer build. Manifest:
`build/logs/goff-parts-install-20261006.json`. Restart/replay level 28 to collect
its parts with the repaired notifications; an old run that consumed them without
notifying its script cannot retroactively count those pickups.
