# Source update through 1.0.11

27 October 2026. This publishes the gameplay changes accumulated since the
previous 1.0.9 repository snapshot, including the 1.0.10 follow-ups and 1.0.11
native runtime. Exavolt PC's application, generated mod ZIPs, texture packs,
retail data, personal saves and local build artifacts are outside this update.

## Charge-shot controls and projectiles

Mining Laser L2/L3 and possessed-MIL lasers share the player charge path in
`weapon_laser.cpp/.h`, `port/pc_laser_charge.h`, `botglitch.cpp`, `botgrunt.cpp`
and `port/pc_input.cpp/.h`.

- Mouse primary repeats ordinary shots. A short right-click uses the selected
  secondary item on release; holding past 0.75 seconds commits to charging.
  A canceled/blocked charge does not become a grenade.
- Controller partial primary pulls charge; an initial full pull fires ordinary
  shots. Once charging starts, squeezing fully keeps that charge until release.
  Controller secondary items retain their immediate behavior.
- The deliberate custom energy rule spends 25/100 energy per charged burst for
  all player/input paths. Retail resets remaining energy to 25% instead. Idle
  recharge remains active; do not describe the custom cost as retail parity.
- Original charge bands, damage profiles, thick-projectile collision, reflections,
  wire pass-through and impact effects are restored. Continuations wait until the
  next tick to avoid recursive same-tick reflection. NPC input is unchanged in
  normal play; Super Mining Lasers is a separately enabled mod.

Rivet L3's mouse right-click hold drives its existing embedded-rivet charge;
release detonates it. A tap still uses the secondary item. Its primary/controller
paths and damage/charge timing remain. Embedded arrows now detach and keep
ticking if a temporary hit parent is removed, avoiding stranded active rivets
and stale-parent notifications. L1/L2 do not gain a new charge mode.

Details: [Mining Laser](mining-laser-retail-parity-20261023.md),
[Rivet follow-up](rivet-mouse-charge-20261027.md).

## Gameplay and co-op repairs

- Shop ammo goes to the buyer's owned weapon without resetting its upgrade.
  Failed/full deliveries do not charge washers or consume stock.
- World batteries use personal co-op pickup claims. Authored battery/weapon
  gifts in The Hand Is Mightier use the same per-player collection system;
  purchases and explicit recipient grants retain their own recipients.
- The full weapon selector stops its user's movement while keeping selector
  scrolling available; partners retain independent movement.
- Slosh targeting accounts for the viewport aspect ratio when the reticle is
  offset. The normal muzzle clamp and NPC targeting remain authored.
- Vehicle primary/secondary damage profiles select the intended player/NPC
  tables. Tank/RAT debug jump forces are excluded on PC; tank MG gain is reduced.
- Destroyed looping audio emitters release their actual DirectSound resources
  even if their logical playback state has already changed.
- Level 15 console handoff validates the target and owner through fabrication,
  interrupted fade and checkpoint restore. Fabrication allocation failure cleans
  up initialized parts and reserves working space without increasing effect pools.
- The scoped Level 15 guard fallback waits for the scene camera and speech to
  finish, then lets the authored door/camera/checkpoint ending run normally.
- Friendly Miner-model NPCs use their existing breakup meshes/effects on death.
  Co-op player-to-player damage protection remains separate from NPC damage.
- The Mil City slope/doorway trap has a geometry-specific collision repair.
- Secret Rendezvous security events recognize the actual co-op operator, while
  tether projection and obstruction rays use the owning player's camera.
- The co-op spinning washer is aligned and sized beside its counter, with its
  own projection/depth handling in split-screen.

Review entry points and evidence:
[1.0.10 follow-ups](1.10-followup-review-handoff-20261023.md),
[console crash](level15-console-crash-20261025.md),
[co-op report batch](coop-bug-batch-20261026.md).

## Native mod support

The game-side source includes `port/pc_mods.cpp/.h`, `pc_randomizer.cpp/.h`,
data pools and their engine/game hooks. CMake includes these dependencies, so a
checkout can build without the separate Exavolt PC app.

API v1-v4 provides resource/CSV overlays, seeded native randomizer options,
additive multiplayer maps, loading-movie/music overlays and named native
compatibility adapters, including optional Super Mining Lasers. A profile is
selected through `MA_PORT_MOD_ROOT`; ordinary launches enable no mods or
randomizer settings. Explicit raw texture mods take priority over general HD
matches; other texture-pack priority rules remain.

Original Exavolt is by dj0wns and contributors; the original Randomizer is by
Avohkii with generator contributions by dj0wns. The separate manager compiles
compatible packages into native profiles. Console PowerPC patches are not
executed by the port. See [existing-install setup](exavolt-installation.md).

## Verification and remaining reports

The accompanying tools cover production charge controls/energy, retail impact
tiers, shop delivery, checkpoint upgrades, Slosh aiming, vehicle jump branches,
slope collision, co-op script identity and the original guard-scene bytecode.
Their data-driven checks require the user's locally extracted retail files.
Opt-in native fixtures for laser/Rivet input, console possession, Miner death,
slope recovery and co-op panel/gifts/HUD are included under `port/` and `tools/`.
No test mission runs on an ordinary launch.

For this publication, the Win32 `ma_port`, `ma_setup` and `ma_input_tests` targets
built successfully, the input mapping executable passed, and 12 focused offline
regression scripts passed. These checks did not launch the game or modify the
installed game/save files.

Earlier native results and user-confirmed L2/L3 controls/Slosh aim are documented
in the linked handoffs. These are distinct from offline tests and are not a claim
of a full manual 42-mission or every-mod-combination campaign playthrough.

The recent Level 2 enemy elevator stall was not reproduced in the authored 2P/4P
checks; existing elevator/checkpoint safeguards remain, with no speculative new
lift fix. Intermittent shots passing through an alarmed MIL also remain an open
report without a confirmed cause. Physical-controller feel and broad randomized
campaign coverage still need manual testing.
