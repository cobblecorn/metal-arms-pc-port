# Mining Laser charge parity and quarter-bar energy rule

The user approved a custom PC energy rule: each charged burst costs 25% of the
bar (25 of 100 energy), so four bursts can be fired from a full bar. Partial
charged bursts cost the same. Normal primary shots still cost one energy.
Retail instead resets the energy bar to 25% after a burst; this difference is
intentional. Existing recharge behavior otherwise remains in place.

The common player-owned laser path covers L2, L3, and the possessed Mil variant
on every mission and local player slot. Primary repeats normal shots. A quick
secondary tap uses the selected item once on release; secondary holds charge
and cannot accidentally become grenade throws when a charge is blocked.
L1 and NPC weapon input/charging are unchanged.

## Local retail evidence

- `gamedata/sys/main.dol`: trigger routine 0x801f8a78, charged damage selection
  0x801fa310, impact callback 0x801fa9c4, reflection 0x801fafb8, direct damage
  0x801fb18c. The upper damage tier starts at 95%, the middle at 50%.
- Original `w_laser.csv`: delay 0.75 seconds, buildup 3 seconds, charge sound
  pitch 1 to 2; projectile speed 800, length 200, width 4, range 100.
- Original `damage.csv`: L2 charged damage 20/35/50; L3 35/55/75;
  possessed Mil 50/75/100, before armor/resistance. The native damage system
  uses normalized units (for example 20 becomes 0.20 health containers).
- Restored charged impacts reflect from surfaces, pass through wires and use
  retail burst/ricochet groups, decals and variant-specific particles. The shot
  retains the launch upgrade, charge band and damager if the weapon changes.
- A charged continuation begins on the next simulation tick to prevent a
  same-tick recursive reflection cascade. Other tracer flags/pool limits are
  unchanged.

## Validation

- `tools/test_mining_laser_charge.py`: 350,656 checks of the production controls,
  charge method and four-shot energy budget across frame rates and players.
- `tools/test_laser_retail_impact.py`: 2,809 checks of the production impact
  callback, all variants/tiers/reactions and invalid launch metadata, with the
  original local tables/DOL supplying the expected damage and thresholds.
- `tools/test_laser_mouse_native.py --upgrade 2` and `--upgrade 3`: real native
  input bridge and Glitch handling; grenade tap 3 -> 2 at energy 100, charge
  100 -> 75 without another grenade, repeated primary fire, and blocked charge
  release without a grenade. Both passed on the final quarter-bar build.
- Native physical-target probe: all nine combinations of L2/L3/possessed Mil
  and damage tier produced exactly one direct impact with the retail damage
  profile through real collision and damage calculation. Solo passed; final
  co-op P2 also passed all nine damage samples. The fixture disables target
  collision after the first impact so a legitimate reflected return does not
  count as an extra initial hit. Four native bursts each spent 25 energy while
  the original idle recharge continued between shots.

Native fixtures are isolated, muted game instances with opt-in instrumentation,
not headless substitutes or a manual mission playthrough. Particle appearance,
reflection feel near tight geometry and physical-controller play remain manual
validation items. The user already approved the earlier L2 mouse controls and
Slosh flame direction; those successful behaviors have been preserved.

Controller clarification: actual firing-device tracking selects the control path,
independently of prompt style or mouse aiming. Controller partial primary pulls
charge; full pulls start repeated ordinary shots. Once charging has begun, a
full squeeze retains the charge until release, matching the retail state branch.
Controller secondary-item behavior remains immediate and unchanged. Mouse
secondary tap/hold remains separate. The 25% burst cost applies to both paths.
Offline checks include analog input across L2/L3/possessed variants and four
player slots, full/partial transitions, four bursts, empty energy and mouse/pad
source selection. Physical controller feel still requires the user playtest.

## User-approved release candidate

On 2026-10-23 the user tested the final L3 Mining Laser run and reported
"looks good", then requested the updated patcher. The initial 1.10 candidate used
the same executable as that run (SHA256
`aaf95cea4942a02d7074e1541e44243106d61f359fca5e24463c799c19dc3923`).
Physical controller use was not specified in that confirmation. Native analog
controller logic and mapping passed; a manual physical-controller claim is not
implied. The approved quarter-bar rule applies to all charge-capable player
lasers, regardless of input device or mission.

Later booth, vehicle and Mil City collision follow-ups supersede that candidate
hash. See `1.10-followup-review-handoff-20261023.md` and the current release
manifest for final package identity and the remaining validation limits.
