# Search for Krunk booth-door cutscene stall (2026-10-04)

The user pressed the room's enemy-alert button in a two-player campaign. The
scripted camera showed an elite guard at the doorway and never returned control;
the session then appeared entirely unresponsive. Game was left untouched and
was not launched during diagnosis or validation.

## Evidence

- Log snapshot: `build/logs/booth-cutscene-stall-20261004.log`, world
  `WEWRresrch2` (Mil R & D Labs: The Search For Krunk).
- The log enters `rsboothdoor.sma` and starts the booth-door camera. It records
  `trigger_group` entries for `elite1`, `elite_lacky2`, and `elite_lacky1`, but no
  entry for `elite2` and no scene completion.
- Offline disassembly of the retail SmallAMX script confirms it commands all
  four guards to walk, then waits for exactly four guard enter events before
  closing the door, releasing the camera, ending the cutscene, and checkpointing.
  The goto calls do not request AI goal-complete events. There is no timeout.
  Diagnostic output: `build/logs/rsboothdoor-20261004.asm`.
- The authored `elitegoto2` position is only approximately 1.57 feet inside the
  trigger's left edge. `CAIPathWalker::CloseEnoughXZ` accepts a stop within the
  bot radius plus one foot (capped at ten feet); path searches also allow two
  feet of destination tolerance. `CAIBotMover::FollowPath_WayPtProgress` can
  advance based on an avoided bot's radius as well. Thus a valid AI arrival can
  leave the guard's center outside the script's trigger.

The missing guard entry is confirmed. Early waypoint completion is the supported
cause, reproduced offline against the authored geometry; the live guard's exact
position, path, and avoidance object were not inspected. This does not establish
an operating-system deadlock or prove that P2 was the blocking actor.

## Correction

The PC script native `Bot_GotoE` adjusts only an unpossessed, living `elite2`
heading to `elitegoto2` during an active campaign cutscene in `WEWRresrch2`.
It uses the armed `trigger_group` center for X/Z, retaining the authored floor Y,
and verifies the point through the normal tripwire shape test. The original
waypoint entity and retail assets are unchanged. Campaign includes solo/co-op;
PvP and other scripts/actors are excluded.

Both goto branches retain look target, speed, return-fire rules, destination
tolerance, and event options. The guard walks normally; real enter events still
finish the authored door/camera/checkpoint sequence. No forced cutscene end,
teleport, artificial guard entry, or general AI tolerance change is introduced.
`CEntity::TripwireContainsPoint` is a read-only public PC query wrapping the
protected shape collision test without dispatching events.

## Validation and installation

- `tools/test_booth_arrival.py`: 7,241 checks. Reads retail world geometry and
  extracts production goal helper, native routing, point query, and path stopping
  method. Reproduces legacy completion outside the trigger; checks corrected
  stopping positions including the two-foot path tolerance, both look branches,
  unchanged options/assets, and exclusions. Physics, navigation, and event
  delivery are mocked; this is not a gameplay test.
- Checkpoint/RAT/timer fixture: 296 checks. Cinematic/menu fixture: 851 checks.
- Debug and staged Release build successfully; scoped whitespace check passes.
  CMake output overrides were reset after staging.
- Confirmed game closed before backup and again before replacing the normal
  Release EXE/PDB/Bink. All three installed hashes match staging. Backup:
  `build/backups/pre-booth-arrival-20261004-021008`.
- Installed EXE SHA256:
  `30BEEC03C654FDD79A6C532D12FA10CD4E80F35A4A8105CC18FAEEEFC8C8B325`.

Replay the button scene with the updated executable; an already-stalled script
will not be repaired in memory. Actual walking, fourth enter event, and return
of control remain to be checked in gameplay. Expected diagnostic:
`Port: booth guard 'elite2' arrival goal moved inside trigger: ...`.
