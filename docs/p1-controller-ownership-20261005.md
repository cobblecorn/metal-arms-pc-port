# P1 controller ownership and hot swap — 2026-10-05

The local join screen previously started a four-port AUTO deal and gave connected
pads to P2–P4 before a player accepted. A single controller opening Co-op therefore
moved from the main-menu/P1 port into P2's box. Normal SHARED routing also exposed
additional controllers as independent ports outside player selection.

`port/pc_pad_routing.h` now keeps preview routing separate from committed device
ownership. Co-op and PvP join entry preserve the controller used to open selection
as P1's controller, regardless of its XInput index. Keyboard/mouse remain on P1's
port. If keyboard/mouse opened selection, the first controller previews P2 and
becomes owned only when P2 accepts; keyboard P1 plus one-controller P2 remains
supported, as the user explicitly requested. Canceling a joined slot releases
its ownership. Forward navigation freezes the actually joined mask for the run.
Fresh selection resets ownership rather than storing controller indices in profiles.

After launch, an unclaimed connected pad may be given to keyboard P1. Hotplug
never silently claims an unused partner slot and cannot take a joined partner's
controller. Partner disconnects reserve their original XInput index; reconnecting
that index restores the same player. P1 can replace a disconnected pad and continue
using keyboard/mouse at any time. Normal SHARED solo/front-end routing puts the
active connected controller on P1 even when another controller is connected idle.
Explicit SHARED/SEPARATE layouts and direct `-coop N` fixture/CLI startup retain
their prior explicit/automatic assignments; normal menu-created sessions use the
new ownership policy. Device identity is XInput slot identity, not a hardware GUID.

Join preview remapping masks held buttons until release, preventing an opening A
or canceled B from replaying on a different player's row. Input/session transitions
and policy updates use the existing polling lock. Profile sensitivity, mappings,
colors and campaign/PvP rules are untouched.

Validation:

- Normal Release `ma_port` and `ma_input_tests` build successfully; input tests pass.
- **480 production routing checks** pass (`tools/test_pvp_join_routing.py`), including
  all connected masks, all controller-opening slots, keyboard P1/controller P2,
  unclaimed hotplug to P1, disconnect/reconnect isolation, sparse joined ports,
  cancellation, fresh co-op/PvP transitions, idle-first-controller switching,
  and held accept/back suppression.
- Native 27-second startup/profile-menu smoke completes without assertion/crash,
  audio, allocation or script error. Existing wrapper `dropfreq` warnings remain.
  The scripted navigation reached profile settings, not a full physical-controller
  join/gameplay run. Physical co-op/PvP device selection and hotplug still need the
  user's controllers; no synthetic desktop controller events were sent.
- Game snapshots only; no computer-use skill or CUA. Smoke processes are closed.
- An intermediate staged OutDir build was superseded: CMake links hardcoded normal
  Release library paths, so final verification/install uses the normal Release build.

Installed Release, staged and Desktop EXE: `98343EE5AA32EDE9A1D7F17CA7D41632F928691B7860D98191B87E8AE1405BA6`.
PDB: `A863387EDBFE4A15409CBB0573AAA037B23590189C5FB937A679FCE45FDB060D`. Bink unchanged: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`.
Record: `build/logs/explicit-input-install-20261005.json`.
Previous complete Desktop EXE / matching PDB backup: `D:\Documents\metal arms source port\build\backups\pre-explicit-input-20261005-220827`.
Normal Desktop game was opened at the main menu for actual device selection.
The factory combat-resume helper still works but bypasses menu device selection.
