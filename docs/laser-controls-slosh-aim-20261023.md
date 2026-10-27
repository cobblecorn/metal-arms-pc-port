# Mining Laser secondary tap/hold and Slosh aim

## Mining Laser controls

The retail GameCube trigger routine at `801f8a78` distinguishes a full analog
pull (normal repeated fire) from a partial pull (charging). The original PC
hold-primary/release implementation was a digital adaptation, not that retail
input scheme. The user requested normal left-click fire and a right-click
tap/hold split, preserving grenades.

Player-owned charge-capable variants now repeat normal shots on primary fire.
A secondary press shorter than the existing 0.75-second charge delay forwards
one secondary-use pulse on release. A longer press is committed to charging;
release fires a burst only when charged energy is available, and never becomes
a grenade throw if charging is blocked by energy, cooldown, stow, or death.
The gesture is owned by each weapon. Glitch consumes the tap pulse through the
normal secondary-throw path; player-possessed Mil bots use the same tap/hold routing.
L1 and NPC input retain their original behavior.

The user-selected PC energy rule spends 25% of capacity for each charged burst,
allowing four bursts per full bar. This intentionally replaces retail's reset
to 25% remaining energy. Charging requires one burst's energy, rather than a
full bar. The shared player path supports L2, L3 and the possessed Mil laser.

The follow-up parity pass restores the retail damage bands (50% and 95%),
thick-projectile collision, launch-time damage/ownership snapshots, surface
reflection, wire pass-through, burst/end particles and charge pitch ramp.
Retail burst profiles and parameters are read from the original local tables.
Visual equivalence against an emulator has not been established. See
`mining-laser-retail-parity-20261023.md` for the source evidence and validation.

## Slosh flame direction: rejected clamp bypass replaced

The user reported that the first fix made Slosh's flame worse. That first change
bypassed `CWeapon::ComputeFireUnitDir`'s muzzle clamp for player Slosh. It has
been removed completely. Its unit/native direction tests only proved that the
weapon followed the supplied target; they did not prove that the target matched
the displayed reticle.

The target calculation had a screen-coordinate mismatch. Reticle rendering
places its X offset in units of half the viewport height, while
`CBot::ComputeHumanTargetPoint_WS` treated the same number as a fraction of the
viewport half-width. Slosh uses nonzero X (0.18), unlike Glitch's centered X.
At 1280x720 this displaced the target by approximately 50 pixels to the right;
the error is larger relative to a split-screen viewport's height.

The PC human targeting path now converts X by the viewport's height/width ratio
before building the ray and visibility-query center. NPC targeting, weapon
muzzle positions, flame damage and the shared muzzle clamp remain unchanged.
The same conversion is used for subsequent target-assist rays. This also
corrects other human reticles with a horizontal offset; centered X is unchanged.

A four-player Fire It Up probe additionally exposed the original fuel-backpack
pool's two-instance cap. PC capacity now covers MAX_PLAYERS plus one authored
NPC, preserving the legacy build's capacity of two.

## Evidence

- `tools/test_mining_laser_charge.py`: 350,656 production-method checks, covering
  frame rates, player indices, normal repeat fire, tap/hold, low energy, cancel,
  simultaneous buttons and four bursts without a refill.
- `tools/test_laser_mouse_native.py`: real PC input mapping, Glitch weapon handling
  and grenade animations in running L2 and L3 game instances. Short right tap reduced grenades
  from 3 to 2 with laser energy 100. Right hold reduced energy to 75 while grenades
  stayed at 2. Left hold fired multiple normal shots. A low-energy right hold
  ended with grenades still 2 and energy 10, without a free burst.
- `tools/test_slosh_aim.py`: 4,002 PC and 4,002 legacy production-method checks.
  These project the generated camera ray back into pixel coordinates and compare
  it with the displayed reticle, over viewport shapes including ultrawide and
  split-screen. They also verify the original Glitch/NPC/Slosh clamp behavior.
- `tools/test_slosh_aim_native.py`: opt-in real Slosh actor, camera, flamer and
  fuel use in `WERRreactr2`. Native projected target error after correction was
  (-0.005, 0.036) pixels solo and (-0.003, 0.017) pixels for co-op P2.
- Actual scripted primary fire was captured through the game's D3D back-buffer
  path and visually reviewed. The flame passes through the reticle center;
  this is stronger evidence than comparing two direction vectors. Capture:
  `build/logs/slosh-flame-reticle-fixed.png`.
- Four-player boot passed backpack allocation after expanding the pool. The
  4P aim fixture did not reach ready-control state before timeout, so 4P
  aiming/gameplay is not claimed as validated. The native solo 4:3 run also
  passed with projected error (-0.005, 0.035) pixels.

Native probes use isolated saves and scripted input or opt-in engine fixtures.
They are not a manual mission playthrough. Continued user testing is needed for
movement, close walls and steep aim. Existing flame effects can continue beyond
a nearby target; that behavior was not rewritten in this pass.

Controller clarification: actual firing-device tracking selects the control path,
independently of prompt style or mouse aiming. Controller partial primary pulls
charge; full pulls start repeated ordinary shots. Once charging has begun, a
full squeeze retains the charge until release, matching the retail state branch.
Controller secondary-item behavior remains immediate and unchanged. Mouse
secondary tap/hold remains separate. The 25% burst cost applies to both paths.
Offline checks include analog input across L2/L3/possessed variants and four
player slots, full/partial transitions, four bursts, empty energy and mouse/pad
source selection. Physical controller feel still requires the user playtest.
