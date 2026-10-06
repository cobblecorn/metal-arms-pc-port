# Surviving co-op players and cached story handles (2026-10-04)

The user's live `WEWRresrch4` (You Know the Drill) log showed P2 entering
`add_titan1` with P1 dead. The co-op tripwire released normally, but the script
never printed `ADDING TITIANS` or created the working elevator.

Retail `xewr4adtitn.sma` stores `Bot_GetPlayer()` in global byte offset 52 during
OnInit. Its encounter trigger compares the entering bot against that saved
handle. Although the native already chooses a living story player, the stored
P1 handle remained unchanged. Consequently the eight NPCs, checkpoint, two
setup timers, real `g_lift` insertion, and `g_fakelift` removal never ran. The
separate `elv_down_bug` trigger cannot restore a lift that was never activated.

The retail audit found 147 script files with the same direct return-to-global
pattern, including five in this mission. This is a pattern count, not evidence
that all those scripts have broken triggers. The PC campaign co-op native now
records only confirmed `Bot_GetPlayer` global assignments. Immediately before
script work, those bindings follow the selected living story player. The
existing story selection preserves the actor of an active cutscene. Restored
checkpoint player handles are recognized; null or repurposed entity cells are
left alone. Binding records reset at level initialization and teardown.

Both registration and refresh require `IsLocalCoop()`. Solo and competitive
multiplayer scripts are unchanged, and retail assets are not edited. This
addresses this class of stale-player gate, not every possible mission softlock.

Validation:

- Real production AMX interpreter and retail elevator script: 39 checks,
  reproducing legacy rejection before refresh, normal NPC/checkpoint/timer/lift
  completion afterward, P1 revival, stable cinematic actor, 2–4 player survival,
  restored checkpoint handles, repurposed cells, and solo/PvP exclusion.
- Existing cinematic/menu, tripwire, scene/lift, and asylum fixtures passed
  (1,141, 400, 434, and 68 checks respectively).
- Debug live regression with isolated saves and engine snapshots: P1 killed,
  P2 enters setup trigger, real lift activated, P1 killed again after checkpoint
  revival, P2 rides to the top. Four checks passed, no asserts or script errors.
  `build/logs/coop-elevator-p1-dead-20261004.log`.
- Private vendor live regression: independent split views, partner entity
  controls and visible model, shopper-only hiding, shopper-only purchase charge,
  normal shop exit. Eight checks passed. An earlier fixture disabled the partner
  before relocation, leaving its mesh at the old position; that fixture step
  was removed. `build/logs/coop-private-vendor-visible-20261004.log` and
  `build/shots/coop-private-vendor-visible-20261004/shot_016.bmp`.

Debug/Release builds passed. The user explicitly authorized launching/closing
the game for these tests and requested native commands/snapshots instead of
computer-use. Tests used `tools/port_run.py` and opt-in `MA_PORT_TEST_COOP_POLISH`;
user profiles/saves were untouched. Release installed after two closed-process
checks with backup `build/backups/pre-coop-cached-player-20261004-033758`.
EXE/PDB/Bink hashes match staging; CMake staging overrides are cleared.
Installed EXE SHA256:
`B4DBF1900F6C121A2B2EFAF1B5CFF250FF47CFB4CDC02131B2F1DB5E75881ABF`.

The installed Release also passed all four isolated elevator checks, with no
asserts or script errors, in `build/logs/coop-elevator-installed-release-20261004.log`.
Test processes are closed at completion; the updated normal executable is ready
for the user's next launch.
