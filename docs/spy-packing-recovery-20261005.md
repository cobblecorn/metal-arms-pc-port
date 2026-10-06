# Factory packing recovery — 2026-10-05

The user's combat encounter had completed: all six authored fighters had zero
health. The HUD/equipment removal was the intended handoff into PROGRAM, not
co-op friendly-fire protection making NPCs invincible. No damage-team rule was
changed.

The packing crate's animation clock was stalled at unit time 0.3 because the
retail mesh did not drive its animation. PC now enables animation driving and
work for both the crate and packing arms before starting the authored sequence.
This also corrects solo.

Only P1 is glued to the moving crate. Partners remain hidden, immobile and without
human control while watching the shared P1 view, including after P1 regains aim,
weapon selection and firing at delivery. P1's movement/jump inputs are filtered
only while actually carried by this crate; aiming, firing and selection remain
available for the authored breakout. This prevents jumping through the walls.
After destruction, P1 is placed on nearby ground checked for body clearance and
wall obstruction before detachment. Once grounded, partners regroup/revive with
their HUD, controls, bodies and independent cameras restored. Other camera
exceptions (private shops and instructor individual views) remain intact.

The user's menu teardown log also exposed an undeleted CFAnimCombinerConfig at
weapon_mortar.cpp. Its pointer was static despite allocation/reset/destruction
per slingshot instance. It is now owned per weapon. Partial-creation cleanup uses
the owned resource array rather than a potentially null active-resource pointer.
This lifetime fix applies globally. Existing prior slingshot firing changes were
preserved.

Validation:

- Release build succeeds.
- Native retail packing: solo **17 PASS / 0 FAIL**, two players **23 PASS / 0 FAIL**.
  Includes authored animation/conveyor, sustained containment, shared delivered
  view, actual dialogue completion, destruction callback, grounded P1 recovery,
  partner HUD/control/model recovery and return to menu for level cleanup.
- Native fixtures deliberately destroy the box after dialogue; ordinary play
  never uses that fixture. Fixture saves are isolated. Completed fixture
  processes were closed. Physical input during earlier tests exposed the jump
  escape, which was corrected. Earlier failing logs are diagnostic, not final.
- Production input filter **66 checks**, cinematic/menu **1202 checks**, factory
  party **149**, instructor **117** all pass. Existing factory individual-view
  stubs were added to the cinematic fixture, plus held-packing view/release cases.
- Final native cleanup logs contain no assertions/crashes or undeleted C++ class
  report. Existing missing BRMG/Level_18DC/door audio assets are not resolved by
  this patch. These runs do not establish complete audio or campaign correctness.
- Native two-player combat resume was inspected with game snapshots and live
  state: stage BATTLE, six living fighters, both players visible/armed with HUDs
  and separate views. Full subsequent progression and physical 3/4-player play
  remain manual checks.

The user requested skipping the completed instructor on the next boot.
`tools/resume_spy_combat.py` starts an explicitly opted-in `spy-resume` process
at the authored combat stage with its normal awarded loadout; it does not enable
fixture invincibility, automated box destruction or menu return. Normal launches
are unchanged. The Desktop `Resume Spy Combat.cmd` invokes it using normal user
profile storage. A normal play process was launched from the updated Desktop EXE.

Installed Release, staged and Desktop EXE SHA256: `83BE4953BEBD4CE8C2ABFF962A802B768ECF9263D6E44877E40F9F388E146077`.
PDB: `945665DFA884B70DEAF6595D61D9717DA5CF78FF6C96A3837C3ABA056F48E930`. Bink unchanged: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`.
Record: `build/logs/spy-packing-install-20261005.json`.
Release backup: `D:\Documents\metal arms source port\build\backups\pre-spy-packing-20261005-204505`.
Desktop backup: `D:\Documents\metal arms source port\build\backups\pre-spy-packing-desktop-20261005-213530`.
Final logs: `build/logs/spy-packing-jump-coop-20261005.log`,
`build/logs/spy-packing-jump-solo-20261005.log`,
`build/logs/spy-combat-resume-check-20261005.log`.
