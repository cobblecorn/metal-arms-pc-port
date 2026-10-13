# Reported bugs audit, 12 October 2026

Five reports have source-backed repairs. The other seven were investigated but
are not established as port defects. Native shop tests and offline production
method tests are listed separately below. Campaign playtesting remains needed
for servos, the arena, and recruited vehicles.

## Repairs

| Report | Cause and resulting behavior | Verification |
| --- | --- | --- |
| C1: max-level EUK crash | Weapon tables include tiers used by NPCs beyond the player's inventory upgrade chain. The old eligibility check accepted those tiers, then inventory notification traversed beyond its last upgrade. Eligibility now requires both a valid weapon tier and a valid next inventory tier. Maxed weapons refuse an upgrade safely. | Offline coverage of all 23 retail EUKs and four player slots; native solo/P2 grants and world pickups at max level. |
| Level 2 arm servo keeps returning | A new Glitch body reset its runtime servo level without reading the saved Arm Servo item. Servo timing also pointed at shared class data. New bodies restore the existing one-based inventory level, and each Glitch owns its weapon timing data. Missing-item insertion stores the actual collected level. | Extracted production methods verify persistence, rollback, invalid-level clamping, and independent player timing. Mission transitions and disk reload need playtesting. |
| Unhandled Exception: cannot switch/reload in the combat arena | The battle stage explicitly disabled weapon selection; the reload handler uses the same enable flag. The PC battle stage now enables normal selection/reload. Instruction stages retain their restrictions. | Source control-flow review and successful Release build. Needs arena playtest. |
| Unhandled Exception: no direction after the fight | The authored victory sequence opens the exit but hid the HUD without an exit message. It now shows each player a six-second message: "Training complete. Go to the open arena door." The authored door, unarmed exit stage, and completion trigger remain intact. | Offline production battle method verifies no premature exit and messages for both co-op players. Needs visual/playthrough check. |
| Recruited hovercraft pilot still attacks | Recruitment changed the pilot's allegiance while the vehicle's independent firing brain retained its hostile race/target. Recruitment and reversal now synchronize the current NPC driver's vehicle team/race and reset its active AI job. Human-controlled vehicles, unrelated passengers, destroyed vehicles, and inactive-world AI jobs are protected. | Extracted production helper verifies recruitment, reversal, and exclusions. Actual hovercraft retargeting needs playtesting. |

These are PC runtime fixes, including single-player. Co-op-specific iteration is
used only where multiple HUDs need the exit message. Existing inventory IDs and
the save-file format are unchanged. The new servo member is runtime-only.

## Reports still requiring gameplay evidence

| Report | Source/data findings | Focused test |
| --- | --- | --- |
| Quiet command-giving Mil | Command sound groups have full authored volume. The audio dialogue classifier already treats the BR voice bank as dialogue rather than applying the SFX trim. No definite attenuation or gain defect found. | Record a command beside normal dialogue, with music/SFX/voice volume settings and whether solo or co-op. |
| Door lights alternate red/green | Door lock state and texture-flip animation are separate systems. The engine provides explicit animation enable/disable controls; some scripts correctly stop the animation before selecting a light page. No particular faulty door is identified. | Supply mission and exact door. Check whether only the light alternates or the actual lock also changes. |
| Washers cluster when dropped | Goodie bags launch pickups from the owner's position with randomized directions and speed. Some clustering is possible under the authored behavior. No identified failure of the current scatter or owner-collision logic. | Record a repeatable drop, including whether the washers remain collectible or cause a performance problem. |
| Mega Energy fully green | The retail pickup references the `gf_life` mesh. No specific incorrect color assignment has been isolated in code or data. | Compare the same pickup with replacement textures enabled and disabled; capture both views. |
| Turret/Pincer audio clipping or fast playback | The chaingun firing state uses one looping emitter rather than creating a new sound for each bullet. The PCM loader uses the sample-directory rate. No proven emitter duplication or sample-rate defect found. Loop-region handling remains a possible audio investigation, not a confirmed cause. | Hold fire for several seconds on one turret, then compare simultaneous co-op firing. Record pitch/clipping, mission, and audio settings. |
| Slosh refuses washers | `_IsPickupAllowed` explicitly allows energy, Mega Energy, and Magma Bombs for the ChemBot/Slosh and excludes washers. This is an authored source restriction, retained here. Console behavior has not been independently compared. | Only revisit if a vanilla comparison establishes that Slosh should collect them. |
| Severe residual Grunt flamethrower burn | The authored Mil burn profile ticks every 0.25 seconds; flame burners last roughly 2-4 seconds and repeated flame contact can produce multiple burners. No port-specific timing error established. Damage was not reduced speculatively. | Compare one brief flame contact followed by moving clear against sustained contact. Record health loss and frame rate. |

## User playtest order

1. Collect the level-2 arm servo, change mission, reload the profile, and confirm
   the upgrade remains applied. In co-op, verify one player's upgrade does not
   change the other player's reload/switch timing. Check rollback to an earlier
   checkpoint separately.
2. In Unhandled Exception's combat arena, switch weapons and reload. Finish the
   fight and follow the exit message through the open arena door.
3. Recruit a Mil piloting a hovercraft. Verify its weapon stops attacking the
   player and targets enemies; then verify allegiance returns when recruitment
   ends. Check dismount/re-entry if available.
4. Use the focused visual/audio tests above for the remaining reports.

## Validation performed

- `tools/test_checkpoint_euk.py`: 2,042 offline production-method checks pass.
- `tools/test_reported_systems.py`: 963 servo, recruited-vehicle, and arena guidance
  production-method checks pass; these do not launch the game.
- `tools/test_slosh_pickups.py`: 118 PC checks and 7 legacy checks pass.
- Native `tools/test_vendor_upgrades_native.py --players 1 --owner 1` and
  `--players 2 --owner 2`: each displayed all 23 real retail EUK catalog entries,
  purchased Rivet Gun 2/3 and Ripper 2/3 through the normal purchase handler,
  rejected maxed direct grants, exited the shop, checked normal pickup eligibility,
  and processed maxed world EUKs placed at the buyer during gameplay.
  Weapon/inventory tiers and wallet checks pass, with no logged crash,
  assertion, or failed check. Co-op purchases leave the partner unchanged.
- Native runs use isolated test saves and opt-in fixture setup; normal runs do
  not receive its supplied weapons/washers. These were shop/pickup tests, not
  full campaign playthroughs. Both runs use the final built source, including
  arena guidance, servo restoration, and inactive-world vehicle protection.
- Release build and whitespace diff check pass.

Native logs are in `build/logs/vendor-upgrades-1p-owner1-20261012.log` and
`build/logs/vendor-upgrades-2p-owner2-20261012.log`. Compressed evidence frames
are retained in the matching `build/shots` directories.

## Packaging and deployment

The current 1.0.6 resource version and the other chat's existing changes are
preserved. The package input is `build/Release/ma_port.exe`. Deployment replaces
only the live EXE and matching PDB after backing up the originals; user saves,
settings, and texture packs are untouched. The deployment manifest records
original and installed hashes under `build/backups/reported-bugs-20261012-*`.
No patcher is built by this task. Remaining playtests above are not an all-clear
for every reported issue.

Final built and installed EXE SHA256:
`10F96423C2AE646549D022C685F56B3066A6E9302C9CC2E394DF0F8D1E01F461`.
Original EXE/PDB backup and deployment record:
`build/backups/reported-bugs-20261012-165318/manifest.json`.
