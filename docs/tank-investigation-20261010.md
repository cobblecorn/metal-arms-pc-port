# Mil Comm Center tank investigation, 2026-10-10

The report concerns the later scripted tank encounter in **Destroy Comm Arrays**
(`WEWCcomm_03`), not the opening tanks in Infiltrate the Compound. The reported
disappearance occurred while Glitch approached on foot, after shooting the grunt
who normally boards the tank. A separate report described a black screen and
checkpoint recovery near the vendor when driving through a doorway. The player
repeated the boarding-grunt kill and approach without reproducing the failure.
The reporting playtester's build version is unknown; the video was described in
chat but was not supplied as an accessible file in this workspace.

## Confirmed source defects and repairs

- `CVehicle::ComputeGroundNormal` initialized its target normal only for an
  upward-facing floor hit or a complete miss. A wall/ceiling hit left xyz unset,
  then passed that vector into quaternion construction. The upright fallback now
  precedes hit handling. Valid floor normals and height calculations are unchanged.
  This applies to single-player, co-op and versus vehicles.
- `CVehicleSentinel::AnimateWheels` wrapped angles with repeated additions or
  subtractions. With sufficiently large float values, subtracting a revolution
  no longer changes the value, so the loop never ends. The PC path now uses the
  standard floating-point remainder, and resets non-finite angles to zero.
  In-range normal animation is unchanged. This is animation containment; it does
  not restore an invalid vehicle physics state.

`tools/test_vehicle_math.py` extracts and exercises the production methods with
controlled collision/math inputs. The original ground-probe test failed on a
wall hit. After repairing that defect, the original wheel routine timed out on
an extreme finite velocity. Both corrected routines pass 118 checks, including
floor slopes, misses, walls, ceilings, frame durations, ordinary wheel motion,
large/non-finite velocities, and collision-palette animation suppression.
These are offline source regression checks, not native reproduction of the video.

Release rebuilt successfully from the current checkout. Other sessions' existing
texture, effect, damage, controller, mission and release changes were retained.
The temporary automatic tank-proximity fixture was removed after investigation;
it had tested outdoor Infiltrate tanks and did not reproduce the reported event.

During final verification, another chat added last-good-state recovery to
`fphysics.cpp`/`fphysics.h` and rebuilt Release. Those edits were preserved;
the combined build passes. The 118 checks above cover this chat's ground-normal
and wheel changes, not the other chat's state-recovery policy.

The user says their manual retry uses the latest patcher release. The visible
running process is `D:\Desktop\MetalArmsPC\ma_port.exe`, an unversioned executable
dated October 7; the rebuilt workspace executable has file version 1.0.3.0.
They have different SHA-256 hashes. Do not assume the manual retry and the
reporter used identical binaries. This chat did not replace the active Desktop
executable, touch the user's saves, or launch a new mission after the user began
their manual location/reproduction test.

## Remaining runtime evidence

The two repairs above are verified defects, but the report's exact root cause
is still unconfirmed. The black-screen doorway symptom remains unconfirmed too.
The player's current live log shows `trig_apc` and the authored `apc bots` event
in `xecc_xplode.sma`; that establishes the correct encounter, not an error.

For a further native investigation, exercise the boarding grunt's death before
entry, during the jump, and after attachment, using the same build as the reporter.
Capture `-port-diag -shots <test-directory> -shot-every 900` and stall stacks on an
isolated test save. Preserve the user's normal profile and avoid restarting their
active playtest. Current authored idle tanks are at approximately (-16,29,-305)
and (-276,33,-201). The later APC trigger is approximately (-117,45,-249).

Evidence: `build/logs/tank-investigation-20261010/`, the `tank-onfoot-*` native-run
logs/captures, and `build/backups/pre-tank-investigation-20261010/` containing the
pre-investigation Release EXE/PDB. No claim that the reported disappearance or
doorway reset is fixed should be made until the actual event is reproduced or
the reporter validates the candidate.
