# Level 15 first arcade possession crash — 25 October 2026

## Report and reproduction

The report describes intermittent process closure when the first arcade takes
control of a Rivet Mil in Level 15, `WEWRresrch2`. The reporter's crash log and
exact build were not supplied. Four native defects were reproduced locally in
this path; these are confirmed defects, not proof that the reporter's exception
has the identical stack.

The first console is the unnamed entity near `(-478.72,-100.06,-363.07)`, using
fabricator `dspns1`, alarm net `consolegrunt` and `xa_autobot_grunt5`.

1. `console15-baseline-unspawned-handoff.log` records the real console calling
   `CBot::Possess` on an off-world Mil, violating `IsCreated() && IsInWorld()`.
   The old transition checks `IsUnderConstruction()` only. A queued fabrication
   that has not started has that flag clear too.
2. The crowded startup fabrications can exhaust the 256-element vector pool.
   A failed `CFXMeshBuildPartMgr::Init` cleans index `i` repeatedly inside its
   loop over `j`, leaving the earlier parts' vectors allocated. Even after that
   cleanup is corrected, pending destinations can occupy 254/256 elements while
   starting a part needs four more. Six unfinished bots then remain stuck.
   `console15-baseline-fabrication-starvation.log` preserves the early diagnosis;
   later opt-in logs include pool occupancy and concurrent build counts.
3. `console15-baseline-cancel-crash.log` records a real `0xc0000005` access
   violation in `CFCamera::SetFOV`, called by
   `CEConsole::_Work_FadeInBehindBot`. Ending possession immediately after the
   console handoff resets the borrowed Mil's owner to -1; fade-in still indexes
   that player's camera. This probe deliberately ends possession at that point
   to make the otherwise timing-dependent transition failure deterministic.
4. `console15-baseline-restore-offworld.log` records a checkpoint reload calling
   `CBot::Possess` on the same off-world Mil. Alarm cleanup removes pooled bots
   after entity restoration, immediately before saved possession is restored.
   This failure is separate from the initial console transition.

The guard fallback was inactive during these reproductions (`cutscene=0`). Its
code, guard destination repair and original booth ending are unchanged. The
13,500 production-AMX booth checks passed again during this repair.

## Repairs

`ma/App/ma/FXMeshBuild.cpp` (PC path only):

- Roll back each successfully initialized part by index `j` on allocation
  failure, restoring its matrix and returning its vectors.
- Temporarily reserve four existing vector slots while admitting a new build.
  Return that reservation on success and every failure. This preserves enough
  working space for at least one pending part to start. Other fabrications retry
  through the existing dispenser logic. Pool size, effect limits, animation
  timing, particle settings and NPC combat/release rules are unchanged.

`ma/App/ma/econsole.cpp/.h` (PC path only):

- Transfer control only to a living, in-world target that is not pending removal,
  and retain the existing construction-completion check. A queued fabrication
  waits; if its net stops without a usable target, release the operator cleanly.
- Validate the borrowed bot's owner and current-player relationship before
  looking up fade-in HUD/camera state.
- Recover an interrupted transition using Glitch's permanent owner: clear this
  console's logo/fade, restore appropriate owner presentation/control, then run
  the existing shield/collision/joystick/operator-release behavior. Another
  player's borrowed bot is not unplugged. An active story camera is not reset.

`ma/App/ma/gamesave.cpp` (PC path only): reinsert the saved controlled bot after
alarm cleanup, before restoring possession. Other pooled bots stay removed.

No save/checkpoint fields or entity data layout changed. No new player immunity,
weapon changes or broader cutscene timeout was introduced. Earlier other-chat
changes were hash-checked against the pre-repair source snapshot.

## Validation

`tools/test_console_possession_native.py` uses the real Level 15 console,
fabricator, scripted possession event, rendering and isolated saves. The user
profile is never used. Tests are opt-in via `MA_PORT_TEST_COOP_POLISH` and never
run on an ordinary game launch.

Native solo runs passed ordinary handoff/release, forced disconnect during
fade, Mil death during fade, and restoration of the authored checkpoint followed
by release. Co-op runs with P2 operating the console passed ordinary handoff,
interrupted fade and checkpoint restore/release; P1 retained independent control.
All seven probes completed without a Fang assertion, exception or failed check.
Final build identity is recorded in the deployment receipt alongside this note.

The normal/restore probes also deliberately leave only 20 vector slots free,
attempt construction of the actual Mil mesh, verify complete failed-allocation
cleanup, release the pressure, retry successfully and force-finish. This tests
the real resource pool and construction methods, not a counter-only mock.

Manual reporter follow-up: retry the first arcade several times, including
checkpoint reload and a Mil dying shortly after acquisition. If closure recurs,
retain `ma_port.log`/`ma_port.prev.log` from that build so the actual exception
can be compared. No full manual Level 15 playthrough is claimed by these tests.

Pre-repair source: `build/backups/pre-console-crash-20261025-072719/`.
