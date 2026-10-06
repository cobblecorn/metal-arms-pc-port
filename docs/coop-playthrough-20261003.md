# Campaign playthrough fixes, 2026-10-03

User reports cover Clean Up through the communications centre. This update was built and
checked offline; the game was not launched and profile/save files were not edited.

## Changes and evidence

| Report | Change | Remaining gameplay verification |
|---|---|---|
| Communications-centre front door cannot be damaged by tank/normal weapons | Retail `WEWCcomm_02` gate `frontie` accepts `SentinelCannonPC` (human Titan cannon) only. Titan scripts spawn bots and assign movement/look-at goals; no explicit gate-breach order was found. In campaign co-op, damage credited to a participating player bypasses this gate's weapon-profile filter. Existing armor, health, destruction events and debris remain. Other doors, NPC damage, solo/PvP, detpack-only and zombieball-only filters retain their rules. | Shoot the entrance with tank or ordinary guns. It still takes damage normally rather than opening instantly. |
| Transmissions visible only on the last player's screen | Script radio starts/stops shared HUD presentation. P1 owns the single audio playback; P2-P4 mirror author text, flashes and antennas on their original bodies. Natural end and immediate stop clean up every HUD without duplicating/stopping the shared stream from mirrors. | Confirm radio text and antennas on both screens, also after a death/cutscene. |
| Shady/Slim disappear after tunnel checkpoint restore | Restore preserves saved hidden state. If the checkpoint predates a vendor encounter that is currently present, co-op can restore that encountered placement instead of returning to “not in world” after one-shot script initialization. | Die near the charge plant after encountering vendors; verify presence and dialogue. |
| Partner repeats the vendors' introduction | Finishing the introduction records it on every participating profile. Initial encounter setup recognizes any participant's recorded introduction. Wallets and inventory remain individual. | Shop as P1, then as P2 in the following mission. |
| Wasteland Journey's giant green sheet / stale lowered liquid | Retail hidden textured volume is attached to `sludge` and explicitly has `EnableRender 0`, `DamageIntensity 0`. The port now reads these fields and the `Texture` alias. Liquid centres follow parent relocation; timed height changes clamp to their remaining duration. Centre, dimensions, height offset and remaining animation save/restore at checkpoints. Render-plane updates address the requested plane rather than one past the array, and update its centre/fog. | Drain goo, descend, then restore a checkpoint taken before/after draining. |
| Invisible volumes might still be authored hazards elsewhere | Render suppression does not suppress collision callbacks. Invisible nonprocedural volumes check collisions once per frame without touching ripple buffers or emitting splash particles. Damage intensity independently determines whether a damage form is submitted. | Preserve normal hazards in other missions. |
| P2 spawns outside Zombiebot King's arena | Initial partner offsets are checked after script initialization/Verlet priming and before checkpoint 0. Invalid offsets search for a safe floor/body placement reachable from P1 without crossing a wall, avoiding prior partners. Valid placements are left alone; no unchecked teleport is used. | Verify boss startup placement and narrow platforms. |
| RAT human gunner targets/hits its own chassis | Both human target acquisition and RAT gun firing add the entire vehicle's existing tracker skip list, including chassis, turret and occupants. | Aim forward/low while the RAT is moving. |
| Loud opening movies | Bink playback now respects the user's effects volume (or pre-fade cache) and applies 0.5 gain, about 6 dB of headroom. | Compare opening volume to speech/game effects at current settings. |
| Missing endings in Clean Up, Wasteland Thunder, Wasteland Journey and Zombiebot King | Level loading now acquires authored `EndMovie` resources. Campaign completion plays the ending once before scheduling the level-complete screen. A missing/failed movie continues normally; PvP does not use this path. | Confirm ending movie followed by normal next-level progression. |
| Possible missing ending in They Live | Retail `WEWJjourn01` has no `EndMovie` entry. No scene was invented. | Compare the retail transition if desired. |
| Black track geometry and opaque-looking fence holes | Corrected a shader mismatch: baked external vertex lighting now excludes texture-lightmap modes, including fast reflections and translucency. Previously those modes could select shaders sampling lightmap textures that were deliberately not bound. | Screenshots cannot confirm this is the only rendering defect. Recheck track/fences; close-range disappearing wires remain unconfirmed. |
| Valve use prompt missing | Switch-parent interaction prompts can display when a trigger child is the nearby target even if the child's own actionable flag is absent. Existing switch busy/used, chip and Mil-only checks remain. | Approach and use the valve with either player. Authored activation positioning/animation remains. |
| Brief “waiting for partners” flashes | Visibility delay increased from 0.35 to 0.85 seconds; gate release/death/cutscene/exit still clear the message immediately. Trigger policy and team synchronization are unchanged. | Brief crossings should stay quiet; a genuinely waiting player still receives the message. |

## Still unresolved

- One checkpoint reload's constant hum: Hunter wind/hover emitters already get destroyed by
  removal via `_RemoveAllAliveEffects`. No verified leaking emitter or duplicate stream was found,
  so no speculative audio-lifetime change was made.
- Delayed Wasteland Thunder music: retail timing comparison remains outstanding.
- Close-range slice-wire disappearance, and whether every black fence/track surface is corrected,
  need another user playthrough. The shader fix has deterministic offline coverage but no live
  visual confirmation.
- Valve activation's small upward body adjustment remains the authored interaction motion.

## Offline verification

Fixtures compile production methods against isolated audio, collision, world and checkpoint data:

- `tools/test_coop_playthrough.py`: 328 checks covering gate/profile restrictions, caged-zombie
  neutrality, shared radio ownership/cleanup, vendor restore/history, liquid relocation/save/restore,
  invisible-volume collision, shader selection and ending-movie acquisition/completion/volume.
- `tools/test_coop_checkpoint_rat.py`: 200 checks, now including initial placement for 2-4 players,
  walls, distinct safe offsets, no-floor failure, missing collision data and solo/PvP exclusion.
- `tools/test_coop_gate_recovery.py`: 400 gate/crossing/occupant checks, with updated prompt timing.
- `tools/test_coop_scene_restore.py`: 434 scene/lift/liquid checks.
- `tools/test_waterfall_render.py`: 6,901 checks / 544 complete waterfall draw passes.
- `tools/test_pvp_join_routing.py`: 145 join-routing checks.

Debug and staged Release builds passed, as did Scatter Blaster's 8,347 checks, cheats' 413 checks
and the PC input mapping executable. EXE/PDB/Bink were installed to normal `build/Release` with
matching staged hashes. Previous binaries are in `build/backups/pre-playthrough-20261003-165656`.
Installed EXE SHA256: `161AC62F2C8216D443839A6533473FBFE30A5338BB1DB92166324C62CAC87907`.
Release output overrides were reset. No game was launched. Extracted proprietary assets,
temporary analysis scripts and fixture executables stay under ignored `build/` directories.
