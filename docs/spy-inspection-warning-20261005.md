# Factory inspection co-op softlock (2026-10-05)

The live normal Release process 11016 was in SEARCH (2), inspector KILL_GLITCH
(14), with five lockers open (9–13), four attempted lockers, chip placement
already enabled, and no remaining state timeout. Native back-buffer capture:
`build/shots/spy-battle-current-20261005/latest.png`. The imposter dialogue is
an authored inspection failure, not random aggression or the DDR instructor.

The earlier co-op `_CheckDeath` guard covered movement/rotation mistakes, but
multiple open lockers or a repeated locker violation directly entered
FAULTY_GLITCH from WALK_BACK. That takes P1 control, then enables combat and
waits for P1 to die. A surviving partner prevents a normal mission reset.

`CSearchStage::_SetGuyState` now applies a co-op-only failure policy before
entering either lethal state. Open lockers route to the authored warning; an
empty-locker failure resumes the inspector's patrol and restores P1's ordinary
camera/control. The warning closes all open lockers in co-op and returns along
the original camera animation. Solo retains its lethal failure rules and its
one-locker warning. Search attempts, collected/placed chips, and locker contents
are retained. This does not auto-pass inspection or the following DDR challenge.

Verification:
- Release build succeeds; 85 production inspection policy/closure/mode checks pass.
- Existing party/objective 149, DDR 117, and camera/menu 1202 checks pass.
- Native regression mode `spy-inspection` reproduces five open lockers, a second
  violation, checks returned control/visibility/health, and requires a real P2
  chip for progression. It is opt-in via MA_PORT_TEST_COOP_POLISH, uses isolated
  profile storage, and never changes normal starts.
- Initial native failure was fixture setup: forcing unit time to 1 left lockers
  paused by Restore. Corrected fixture unpauses them with the original opening
  speed. The production closure uses the same authored animation as before.
- A 65-second run stopped just after the initial failure; longer final run is
  required because the normal opening and post-assembly briefing precede it.

Final native run: **17 PASS, zero FAIL**, no assertion/crash/allocation,
audio, or script errors. Both first/repeated warnings return P1 control and
retain visible/controllable partners. The fallback resumes search. A genuine
P2 item pickup permits the ready/escort transition, verifying no auto approval.
The native setup uses two players; physical 3/4-player sessions remain unverified.
Earlier final-chip check used the weapon grant helper instead of item pickup, then checked before the deferred world pickup;
the corrected fixture uses GiveToPlayer(COLLECTABLE_CHIP), with no production
chip behavior change. Final log: `build\logs\inspection-warning-pickup-native-20261005.log`.

Installed normal Release, both staged directories, and Desktop EXE: `F078E3D4F070570A540734A0C98D60B1996AD49D94201CB6193A90C11E5B8A47`.
Matching PDB: `4D14BDCFE03124D5E13345FFEF863DF6F086653B47AF0032BB399EE3D712C322`. Bink unchanged: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`.
Backup: `D:\Documents\metal arms source port\build\backups\pre-inspection-warning-20261005-230353`. Record: `build/logs/inspection-warning-install-20261005.json`.
The old stuck normal run and own fixture processes were closed before install.
Normal starts use the existing profile/checkpoints; no fixture flags persist.
