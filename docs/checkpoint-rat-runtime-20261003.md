# Unexpected checkpoint restore and RAT runtime follow-up — 2026-10-03

Update staged in `build/pending-update/Release`; live game PID 34964 remains running.
No game launch, process termination, live-memory modification or save/profile change.
Normal `build/Release` executable remains the installed rendering/slingshot update.

## Evidence and changes

- User reported checkpoint rollback in level 10 with both players alive during uninterrupted
  movement. Preserved log `build/logs/checkpoint-unexpected-level10-20261003.log` records slot 1,
  notify 0 after `trig_alloy` releases. The ordinary player death/falling paths request notify 1;
  the production pause Respawn path requests slot 1 / notify 0. There is no caller information
  in that version's log, so the exact trigger is not established.
- Confirmed pause control-flow defect: accepting restart/respawn calls ExitPause but Work then
  continues into menu and mouse handling with the same input. Confirmation ownership also
  depended on shared CMsgBox context/result and was not reset across pause/level lifecycle.
  Track the specific pending action, consume it once, reject inactive/non-options callbacks,
  clear ownership on start/exit/level init/uninit, and end menu processing after dialog results
  or an action closes it. Cancel/quit/restart/respawn are covered offline. This repairs an unsafe
  path but is not proof of the reported spontaneous rollback's cause.
- Every production restore caller now names its reason: pause, death continue, outside-world,
  stuck-in-air, scripted restart, boss/defense/coliseum restart, developer shortcut. Log current
  and pausing state plus each player's entity, in-world/death/air state and position.
- User **confirmed RAT black textures now look fixed** in the installed build.
- RAT controller camera still used raw vertical input to integrate camera height independently
  of the turret, which the previous patch reversed. Reverse the PC gunner camera input too;
  mouse still follows gun pitch. Other vehicle cameras unchanged.
- P2 looked like the NPC in the turret. Generic vehicle DrawEnable propagated its draw flag
  to the decorative Zobby mesh, exposing it after co-op entry/restore. Human entry explicitly
  hides that mesh and shows the selected player body; generic redraw preserves the NPC hide
  for a human co-op gunner. Solo/NPC rendering retains its behavior.
- Loud constant hum at RAT start persisted through some restarts, stopped when approaching
  the jet and did not return on a subsequent restart. Positional gain formerly refreshed only
  on an emitter-position or selected-listener-orientation event; listener switches, radius
  changes and stationary voice reactivation could retain stale gain. A gain not yet computed
  used a full 3D-scale fallback. Recompute distance gain from the nearest active listener each
  audio update independently of pan/orientation; no listener or unknown gain is silent for 3D.
  2D sound mixing stays intact. New jet hover/wind voices receive their intended gain, pitch
  and position in the allocation frame; formerly initialization waited for the next bot tick.
  New allocation logs identify jet entity, 3D/2D state and possession index. The exact reported
  jet-hum trigger still requires confirmation in a new run.
- Actual RAT assertion: `fcoll_sphere.cpp:1147`, invalid fImpactDistInfo in sphere/triangle-edge
  sweep. Preserved stack in `build/logs/rat-assert-and-jet-20261003.log`: _CollideSphere →
  _RatCollisionDetect → CFPhysicsObject::Simulate → CVehicleRat::MoveVehicle.
  Game survived the assertion. PC edge sweeps now project into relative perpendicular space,
  calculate in double precision and use a stable entry-root solution. Retain edge/vertex
  contacts, earliest hit selection and legacy non-PC math; no blanket collision disable or
  assertion suppression. Production arithmetic stress tests reproduce five invalid legacy
  push distances in 120,000 sweeps and zero with the revised math.

## Validation

- Debug and staged Release builds pass; output overrides reset afterward.
- `tools/test_checkpoint_confirmation.py`: 136 production confirmation checks.
- `tools/test_rat_runtime_guards.py`: 44,535 checks, including 120,000 old/new collision sweeps,
  1–4 listener gains, no-listener startup, radius changes, camera input, decorative model and
  immediate jet-loop initialization. Legacy invalid pushes 5; revised invalid pushes 0.
- Checkpoint/RAT fixture 296, scene/lift/liquid 434, slingshot/texture/aim 9,144 and selector/menu
  6,147 checks pass. Source whitespace and Python syntax checks pass.
- No game launched. Confirmation of checkpoint reset, hum, camera/model and actual collision
  location remains pending. Staging protects the user's current session.

Final staged SHA256 values:

- `ma_port.exe`: `0D2CB54F8B54B8952AD9F6FFB4CF026B50119B5473398D464D3AD298276402A4`
- `ma_port.pdb`: `3DA99CB7EC89BA6414DEF93AACD3481B3BB45D31D2783BA2B4D4CAE75551D9B8`
- `binkw32.dll`: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`

Live EXE remained unchanged (`E24BE14A8FADD3E8B419B0A03F7A4EFEA9B1EB62A32A2A53659270741534F6A6`),
with PID 34964 still running at final verification. Release output overrides are cleared.
