# Factory instructor hologram timing

## Instructor hologram flash restored (2026-10-05)

The prior patch held holograms for the entire command response window. Removed
that co-op override at the user's request: the original alpha fade now runs in
all modes. Each command flashes its guides and they disappear on the authored
timer, before the response deadline. Partner guides keep distinct lane positions
and colors, mirror the primary fade, and disappear together. Delayed fake-command
previews retain their scheduled insertion. Per-player command checks are unchanged.

Verified: **117 production checks** including original solo/co-op fade, partner
expiry and command validation; **42 native co-op PASS, zero FAIL**, including
guides visible shortly after the command, all hidden while its response window
is still active, missed-command rejection and correct-command advancement.
Native log: `build/logs/spy-hologram-flash-native-20261005.log`.
No assertions, crashes, allocation, audio, script or schema errors were reported.
Physical 3/4-player play and full solo instruction remain unverified.

Installed live and staged EXE: `211B8FCA1D93594DB4D1EE222C0E8911C1900C2E5A1C4724223F14F8F6581257`.
PDB: `2811B8254A75429052E43C233EEC238C223CAA8D4C7D884BA547BD5DABBCF858`.
Record: `build/logs/spy-hologram-flash-install-20261005.json`.
Previous build backup: `D:\Documents\metal arms source port\build\backups\pre-spy-hologram-flash-20261005-180049`. Bink unchanged.

