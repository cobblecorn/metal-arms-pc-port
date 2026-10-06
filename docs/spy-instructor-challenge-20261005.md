# Factory instructor commands and partner holograms (2026-10-05)

Follow-up: the persistent-guide behavior described here was replaced with the
original timed flash at the user's request. Current build and verification:
`spy-hologram-flash-20261005.md`.

Standing still could pass the co-op instruction sequence because the previous
patch's movement grace and accumulated failure timers exceeded each command's
response window. The next spoken command reset those timers before rejection.
The earlier five-second validator tests missed this shorter production window.

## Changes

- Each command now checks every active player's completed position, facing and
  jump before advancing. The 3.5-foot target radius remains forgiving, bounded
  below a whole step for smaller configurations. P1's movement or jump cannot
  satisfy another player. Incorrect commands use the existing warning/team retry.
- The co-op response window includes the same additional 0.6 seconds of movement
  grace. Input is validated once before the deadline, including a jump pressed
  on the final frame. Completed jump requirements are retired before the next
  announcement, rather than authorizing unrelated jumps.
- Formation placement clears both human-control state and velocity. A reset
  cannot carry the previous scripted movement into the new starting line.
- Resource loading previously allocated partner holograms using player count
  before players were initialized. It now reserves all three partner guides in
  co-op and shows only active player slots. Their placement also used the inverse
  world transform; it now copies the forward model-to-world transform.
- P1's green guide and partner cyan/orange/magenta guides stay visible through
  the response window. Fake-command preview timing is retained. Each guide uses
  its player's fixed lane offset, and has no collision.

These gameplay changes are restricted to local campaign co-op. Solo validation,
punishment, command timing and NPC demonstrations retain their authored behavior.

## Verification

- `tools/test_spy_ddr.py`: **91 checks**, compiling the actual validator, command
  completion/advancement, per-player validation, resource allocation and guide
  placement methods. Cases include short response windows with skipped steps,
  quarter turns and jumps for each player in 2-4-player groups, last-frame jump
  input, forgiving completed steps, smaller step configurations and unchanged
  solo advancement. Guide allocation is tested with player count still zero;
  forward and inverse transforms are distinct in the test fixture.
- Existing factory party/mode/objective checks: **149**; guide checks: **13**.
- Final installed binary: `spy-ddr-challenge-tested-release-20261005.log`,
  **41 native PASS, zero FAIL**, no assertions, crashes, allocation, audio,
  script or schema errors. The fixture executes an actual retail six-foot step
  with the short response window: idle players fail, correct P1 cannot approve
  idle P2, and both imperfect completed steps pass through normal advancement.
  Distinct persistent holograms, death/retry, checkpoint restore and authored
  combat exit are also checked. Deadline fixtures clear live input/velocity so
  controller activity cannot alter the intended test positions.
- Final installed solo opening: `spy-ddr-challenge-tested-solo-20261005.log`,
  **9 native PASS**, no assertions/crashes/errors. Full solo instruction is not
  certified by this opening smoke check.
- Captured game snapshots were inspected: P1's green and P2's cyan holograms are
  visible at distinct destinations. Physical 3/4-controller play and every full
  random instruction sequence remain unverified.
- Intermediate logs containing failures are diagnostic runs, not passing proof.
  They exposed absent guide allocation, inverse guide placement and stale fixture
  movement. The final native and offline checks above supersede them.

## Installation

Live and staged EXE/PDB hashes match. Install record:
`build/logs/spy-ddr-challenge-install-20261005.json`.

- EXE: `8FDE393CD8E0A6D6F7DEFD232CFB6F5EFD781D4F206168AA6A5539C270189895`
- PDB: `E88053AEAEDCD19E0D33280A8D73FB6726850867BF3E58B9B30B46E6B48BB938`
- Previous tested build backup: `build/backups/pre-spy-ddr-challenge-20261005-143024`

Bink is unchanged. Tests use isolated logs/saves and only their own processes.
