# Booth fallback, vehicle debug hops and tank machine-gun volume

The user replayed level 15, The Search for Krunk, and confirmed the booth scene
completed normally. They requested a narrow fallback because enemy traffic can
still leave a guard short of the arrival tripwire. Existing arrival-goal repair
remains in place.

## Booth safety net

Only campaign `WEWRresrch2` and the recognized original `rsboothdoor.sma` layout
qualify. The script must have started the alarm scene, issued its guard orders,
and entered the wait for fewer than four arrivals. After 15 seconds of that
wait, the fallback also requires the scene's own camera path and streaming speech
to be done. It advances only to the authored closing state and marks the missing
arrivals consumed. The unchanged retail script then closes the door, waits one
second, checks its camera completion, releases the camera, waits another half
second, ends the scene and checkpoints. A late real arrival cannot restart this
ending. There is no direct cutscene-end call, teleport or forced camera stop in
the fallback. Other levels, scripts, scene phases and PvP are excluded. The
per-level timer resets when the scene is inactive and on level initialization.

Validation:
- 13,500 checks execute the real retail bytecode in the production AMX VM:
  normal four arrivals, one/all missing, a late arrival after fallback, longer
  camera/speech, paused timing and invalid layouts. Original ending delays and
  unchanged bytecode are verified.
- Existing arrival-goal fixture: 7,241 checks passed.
- Native normal two-player scene: all four guards entered and no fallback ran.
- Native solo and two-player blocked scenes: four guards deliberately held
  outside the tripwire. The real fallback released the wait at approximately
  15 seconds and both scenes completed at approximately 16.5 seconds of waiting,
  with the normal camera release, checkpoint and each player's control restored.
  These are opt-in isolated-save fixtures, not physical-controller playthroughs.

## Vehicle jump and sound

The Sentinel tank and RAT had a development-only upward-force branch under
`!FANG_PRODUCTION_BUILD`; the PC Release configuration still includes diagnostic
code. The Loader had a similar developer-user hop. All three branches now exclude
`FANG_WINGC`, retaining legacy debug behavior, normal on-foot jumping, scripted
entry jumps and vehicle righting/flip recovery.

Only the tank machine-gun playback gain is reduced, to 0.70710678 (-3 dB), for
both its player 2D and NPC 3D paths. Sound radius, randomized pitch, fire rate,
cannon/engine sound, damage profiles and the existing global mix are unchanged.
Release build and the 118 existing production vehicle floor/wheel checks pass.
Vehicle feel and final listening balance await user playtesting.

The actual extracted tank, RAT and Loader branches were also compiled in PC
and legacy debug configurations: PC applied zero hop forces and legacy debug
retained all three. See `tools/test_vehicle_debug_jump.py`.

The Mil City slope/doorway collision repair and complete 1.10 review boundary
are documented in `1.10-followup-review-handoff-20261023.md`.

## Projectile phasing report remains unconfirmed

The running user test log was copied to
`build/logs/level15-phasing-20261023/paused-playtest.log`. After the level-15 scene
it records impact-decal exhaustion, but not per-shot hit/miss details. The later
possessed-Titan report occurred after the same run loaded `WEMCcity_03` (Mil City
Hub); its script records named deaths but no corresponding collision traces.
No broad collision, invulnerability, friendly-fire or projectile-damage change
was made from these logs. The reported 5-10-second phasing remains an open bug.

## Delivery

The final Release pair and revised 1.0.10 package include these changes together
with the tested L2/L3 mouse/controller controls and global quarter-bar charge
cost. Deployment state and exact hashes are recorded in the release manifest;
an already running user game keeps its existing executable until it closes.
