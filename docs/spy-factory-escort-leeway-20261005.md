# Spy Factory escort, rebuilt camera and instructor tolerance (2026-10-05)

The previous party patch retained a strict retail formation test and continued
validating the old target while the instructor spoke the next command. Partners
also stayed under human control during P1's authored escort to the demonstration.

## Changes

- The post-assembly search stage explicitly resets P1's pitch to neutral and
  clears its borrowed view link before initializing its own third-person camera.
  The rebuilt inspection/transmission view starts from P1 even if a partner
  was looking upward. Other players' input settings are not changed.
- At the search stage's authored escort, partners receive AI-controlled routes
  to separate positions behind P1. Each keeps an independent third-person
  camera through the walk and NPC demonstration. The normal crane transition
  restores partner human controls. Stage restore also clears the escort state.
- Regular instructor play places partners four feet apart in a line behind P1.
  Targets are still per player, using fixed lane offsets. Destination holograms
  use green/cyan/orange/magenta to distinguish the four players' targets.
- Co-op target radius increases from about 2.45 to 3.5 feet, with 0.6 seconds
  extra movement grace and 0.75 seconds extra alignment/position/look tolerance.
  Formation and facing thresholds are more forgiving, and turning the wrong
  way briefly no longer triggers the retail instant failure.
- Spoken upcoming commands do not adjudicate position/facing against the old
  target. Jump anticipation and fake-command behavior remain active. Persistent
  wrong targets, wrong turns, and missing jumps still fail; failure retries the
  team instead of entering the solo death/crusher route.
- Placement clears a stale first-press jump from the current human-control
  sample as well as the input device samples. This avoids immediate repeated
  rejection when reset and validation happen in the same frame.
- Failure diagnostics identify the rejected player and movement/position/look
  timers. A partner's failure no longer leaves the shared jump state owned by
  that partner.

These behavior/tolerance changes apply only to local campaign co-op. The solo
validator retains its retail limits, instant opposite-facing rejection and
original punishment. The authored NPC demonstration remains unchanged.

## Verification

- `tools/test_spy_ddr.py`: **23 production-validator checks**, covering solo
  limits, co-op 2.8-foot tolerance, independent lane offsets for four players,
  large persistent misses, turn grace, sustained wrong facing and spoken-command
  timing. It compiles and runs the actual `_CheckAlignment` implementation.
- Existing production checks: party/modes/objective **149**, guide **13**,
  checkpoint/placement **296**, campaign playthrough **328**.
- Native candidate logs: `spy-leeway-installed-candidate-20261005.log`,
  `spy-escort-installed-candidate-20261005.log`, and
  `spy-leeway-solo-candidate-20261005.log`. Targeted fixtures use isolated saves.
  The escort fixture seeds the scripted post-assembly equipment before entering
  search, then waits for the real transmission and exercises the actual escort
  and automatic release. It does not replay the entire claw puzzle.
- A four-player native attempt reached the expected missing-controller dialog;
  only two physical input devices were available. Four-player lane math is
  covered offline; physical 3/4-player instruction remains unverified.

Final candidate native checks: **34 instructor PASS, 11 escort/camera PASS,
9 solo opening PASS**, zero failures/assertions/crashes/audio/script errors.
These checks do not certify every random instruction sequence or an
uninterrupted full factory mission.

## Installation

Installed 2026-10-05T13:03:11.7399491-04:00; live hashes match the tested staged files.

- EXE: `38DFD1A6AA8B3C2580CBD6CA0D418E9C4404CA53EDA85D52296608E23B506041`
- PDB: `EB6836D72BEFD59F0A267C2244D9E8F66E7385EFE4F375E766C033865BC01C23`
- Backup: `build/backups/pre-spy-escort-leeway-20261005-130311`
- Install record: `build/logs/spy-escort-leeway-install-20261005.json`

Bink unchanged. Temporary Release output overrides removed. Only isolated
test processes and save directories were used; no game/test process remains.

