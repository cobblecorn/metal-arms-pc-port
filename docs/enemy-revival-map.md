# Enemy and actor implementation map

> **Update 2026-09-29 (implementation):** the Mil Sniper now spawns in the campaign and fights. The float/fling,
> crash and no-damage problems below were bugs in the recovered actor (a stale `AppendTrackerSkipList()`
> override, a laser-sight array overflow, an empty tracer kill callback, invincible retail armor), all fixed.
> See HANDOFF.md, "Cut-content Mil Sniper in the campaign", for what changed and what was verified.

Status: source and static-world audit for handoff. This maps what the current checkout can construct, what its code contains, and which serialized types occur in the local US GameCube `.wld` worlds. It does **not** prove runtime behavior in this DirectX port or account for actors spawned dynamically by scripts after load. Retail `b_*.csv` tables are packed in the game data (see `gamedata/mst_index.csv`), not checked in as ordinary source CSV files.

**Static placement audit:** `.wvs` is streamed audio, not world data (`ma/Lib/Fang2/dx/fdx8data.h`, `fdx8audio.cpp`). The local GameCube MST index contains 59 `.wld` worlds. All 59 static world init blocks were decoded using the layout in `ma/Lib/Fang2/fdata.h` and conversion/validation path in `port/gcdata.cpp`: 16,396 shape records, 11,556 with embedded GameData, with no header or shape-count errors. The records include 57 serialized `BotBlink` entities, but no `BotSniper`, `BotSwarmer`, SwarmerBoss/Queen, Berserker, or cut-weapon entity types. `BotBlink` currently maps to `CBotMiner`; it is not evidence that the early Glitch model was placed. Some records named `snipergrunt1` are typed as BotGrunt or BotJumper, not BotSniper. This only covers static world-init entities; scripts and spawn systems can create actors after load.

For current runtime status of the recovered Sniper and the disabled Swarmer Queen, this document supersedes the earlier optimistic wording in [`cut-content-revival-audit.md`](cut-content-revival-audit.md).

## How an enemy becomes functional

The relevant path is:

1. A world entity type string is dispatched to a class in `ma/App/ma/entity.cpp` (the factory around `_CreateWorldShapeMesh`).
2. Its builder interprets the serialized entity configuration and loads the bot-specific retail table. The class table supplies mesh/animation, sounds, damage profiles, weapon attachments, and other tuning.
3. `ma/App/ma/Ai/AIBuilder.cpp` chooses faction/race, weapon count, attack rules, awareness, initial job and, for some bots, a custom brain/mover. `AIBrainman.cpp` provides the fallback brain and mover when the builder does not override them.
4. The AI brain chooses jobs/thoughts (wait, wander, patrol, combat, search, etc.). The concrete `CBot…` class runs its own animation, weapon, attack and damage routines, while `CBot` supplies shared target, movement, health, damage and death behavior.

So a class compiling is only one link in the chain. For a recovered enemy, verify the world type string, builder leaf bit, AI defaults, retail table shape, model/animation resources, attack/damage path, and world placement together.

### AI defaults worth knowing

`CAIBrainman` currently maps Pred/Probe to generic hovercraft brains with the 3D mover; pillboxes/rat guns to the Pill brain with XZ movement; Rat vehicles to generic biped brain with car movement; Scout to the Scout brain with XZ; Mortar to its Mortar brain with XZ; Jumper to generic biped with XZ-and-hover; and most other mobile bots to generic biped with XZ. Site weapons and Swarmers are excluded from that ordinary fallback. `AIBuilder` can override these: Snarq sets a 3D mover, Corrosive and ZombieBoss set custom brains, and a level's serialized builder data supplies per-actor rules and loadouts.

The generic biped brain connects wait/wander/patrol/follow/combat/goto/face/talk thoughts. The Scout brain adds ground search and a scout-alert combat thought. The Corrosive and ZombieBoss brains have their own combat thoughts. The Mortar brain's default thought table is empty, so its actual control/behavior must be checked in the serialized builder or mission scripts instead of inferred from the enum name.

## Source-backed mobile bot roster

“Attack evidence” below means the source has a corresponding routine/profile path; it is not a claim that it has been confirmed in a live port session. Table names are the class's expected retail data keys. The MST index has entries for most of them; case varies between source strings and the index.

| Actor | Source / retail table | AI and source-backed behavior | Handoff notes |
|---|---|---|---|
| **Grunt (`BotGrunt`)** | `CBotGrunt` — `botgrunt.cpp`; `b_grunt` | Generic biped. Builder defaults allow mech use, start with grenade resources, and configure AI weapon behavior after the selected NPC weapon is known. Class has weapon switching/firing, shields and grenade/throwable handling. | Best baseline for validating the common hostile-bot path. Actual gun depends on serialized builder/loadout; don't hard-code a single weapon for all Grunts. |
| **Miner / legacy Blink (`BotBlink`, alias `BotMiner`)** | `CBotMiner` — `botminer.cpp`; source requests `b_Miner`, which resolves to indexed `b_miner.csv` | Generic biped and Droid faction by default; miners can buddy with same-race players. Builder defaults to Laser + Hand, with code paths for stored Hand/Laser/Spew/Flamer/Rivet/Rocket variants; CableHook and throwable secondary behavior are present. | Both accepted world strings construct `CBotMiner`. `b_blink.csv` is separately indexed and has a Blink table, but the current source does not reference it; resolve the intended old Blink actor/table through world data before wiring it. |
| **Jumper (`BotJumper`)** | `CBotJumper` — `botjumper.cpp`; `b_jumper` | Generic biped with XZ-and-hover mover; jump-capable combat defaults, close melee and a pulse-cannon shot with splash damage. | Attack rule set and jump range are type-specific in `AIBuilder`; check both mover and attack rule wiring when diagnosing an inert actor. |
| **Mortar (`BotMortar`)** | `CBotMortar` — `botmortar.cpp`; `b_mortar` | Dedicated mortar bot and projectile/muzzle code. Defaults describe a stationary/long-range role and no ordinary carried weapons. | Brain enum's default thought row is empty. Confirm map script/builder control and its firing target input before assuming it should chase or acquire targets like a Grunt. |
| **Mozer (`BotMozer`)** | `CBotMozer` — `botmozer.cpp`; `b_mozer` | Generic biped. Has NPC weapon work plus a dedicated girder melee/impact attack path. | Large/slow encounter behavior is still builder/data driven; inspect its animation and damage-table setup before reusing Grunt assumptions. |
| **Scout (`BotScout`)** | `CBotScout` — `botscout.cpp`; `b_scout` | Scout brain + XZ mover. Alert/search thoughts are available; AI defaults set zero weapons and avoid mech use. | Primarily a detection/alarm actor, not a conventional shooter. Check its alert callbacks and the map's alarm links. |
| **Scientist (`BotScientist`)** | `CBotScientist` — `botscientist.cpp`; `b_scientist` | Generic biped framework; AI defaults use Mil race, wait as initial job, a dedicated Scientist attack rule set, low movement speed, and no mech use. | Can be neutral/friendly or hostile depending on race/serialized configuration and mission context. Don't classify every Scientist as an enemy. |
| **Slosh (`BotSlosh`)** | `CBotSlosh` — `botslosh.cpp`; `b_slosh` | Generic bot framework with its own weapon handling; source defaults identify Flamer as NPC weapon 0 and MagmaBomb as slot 1. Droid race default. | `ENTITY_TYPE_BOTCHEMBOT` is the legacy macro name, but its world string is `BotSlosh`; the factory does not accept a separate `BotChemBot` string. Verify weapon resources and both slots. |
| **Snarq (`BotSnarq`)** | `CBotSnarq` — `botsnarq.cpp`; `b_snarq` | Hover-capable bot with 3D mover set by AIBuilder; pulse/tracer shot has an impact damage submission path and armor-bone break handling. | Do not rely solely on `AIBrainman` fallback: the builder's 3D override matters. Its code has some legacy/commented damage setup, so verify profile lookup and tracer group in the running port. |
| **Corrosive (`BotCorrosive`)** | `CBotCorrosive` — `botcorrosive.cpp`; `b_corrosive` | Custom Corrosive brain/rule set. Source has rocket/projectile, fist/swipe and foot-stomp damage paths; damage is location/weak-point sensitive and has damage milestones. | A high-value special enemy, but requires its specific mesh bones, damage/vulnerable spots, effects and profiles. A generic bot spawn can render while attacks or weak points remain broken. |
| **Pred (`BotPred`)** | `CBotPred` — `botpred.cpp`; `b_pred` | Generic hovercraft brain + 3D mover. AI defaults include primary quad-laser bursts and a secondary volley. | Confirm bot table, two weapon slots, muzzle bones and projectile/tracer resources as a set. |
| **Probe (`BotProbe`)** | `CBotProbe` — `botprobe.cpp`; `b_probe` | Generic hovercraft brain + 3D mover; has explicit attack selection/animation and pinch, ground-hit and spin damage profile paths. | Its attacks are not a normal CWeapon inventory flow; inspect the attack state and collision callbacks. |
| **Titan (`BotTitan`)** | `CBotTitan` — `bottitan.cpp`; `b_titan` | Generic biped ground mover. AI defaults call for chain-gun and rocket slots, plus melee/stomp; source has shield, footstep/stomp radial damage and rocket blast handling. | Large actor with shield, weapon bones and multi-profile damage. The data table and mesh palette must initialize correctly before diagnosing combat. |
| **Zombie (`BotZombie`)** | `CBotZom` — `botzom.cpp`, `botzom_part.cpp`; `b_zom` | Generic biped brain but Zombie race and attack rules. Close-range claw/slash damage uses a swept sphere; multipart damage/death and limb-part handling are implemented. | Separate its multipart subsystem and zombie damage rules from the single-mesh bot assumptions. |
| **Swarmer (`BotSwarmer`)** | `CBotSwarmer` — `botswarmer.cpp`; code requests `b_swarmer` | Flock-managed movement and bite attack (`SwarmerBite` profile); not the ordinary per-bot mover path. | `b_swarmer` was not present in the extracted MST index list checked for this map, while a Queen table `b_sboss` is indexed. Verify the required table/resource in the actual retail data before testing. |
| **Elite Guard (`BotEliteGuard`)** | `CBotEliteGuard` — `botEliteGuard.cpp`; `b_elitegrd` | Generic biped with an Elite Guard attack rule set. The source builder defaults to Staff in slot 0 and Hand in slot 1; AIBuilder has cadence/melee defaults commented as stun-gun behavior. The class has its own weapon and damage routines. | Verify the serialized loadout and source/data naming: class defaults and the AIBuilder comment describe different levels of the attack setup. Don't assume every variant is ranged. |
| **Mil Sniper (`BotSniper`, recovered source)** | `CBotSniper` — `botsniper.cpp`, `botsniper_data.cpp`, `grapple.cpp`; `b_sniper` | Primary shot creates a damage-bearing tracer. Scope transition and a hook/grapple state machine exist. Rifle tracer width/range/speed, reload time and grapple cling offset are hard-coded in `BotInfo_Sniper_t`; the `Sniper` data-map entry remains commented out. The class creates a scope in weapon slot 1 but has no normal `CWeaponSniper` primary weapon. | **Not yet a verified enemy.** The static `.wld` audit found no BotSniper placement. See the test record below. `ENTITY_BIT_BOTSNIPER` aliases `ENTITY_BIT_UNSPECIFIED`, so type-specific AI defaults/selection need a careful audit. Player grapple input does not prove enemy AI will grapple. |

## Retail table and NPC weapon map

The local packed tables can be decoded with `tools/gamedata_dump.py`; the output is only a view of the local retail data and should stay under ignored `build/` if saved. The unique tables are useful clues about how much of each actor was authored:

| Table | Non-generic tables / notable contents | Interpretation |
|---|---|---|
| `b_jumper` | `idles`, `jumper`, `jumpersounds` | Dedicated attack, jump-jet and sound tuning exists in addition to shared movement/weapon data. |
| `b_corrosive` | `corrosive`, `vspots`, `damagemilestones`, `deathexplosions` | Full weak-point and phase/damage progression support is data-backed. |
| `b_elitegrd` | `dive`, `staff` | The authored special attack is staff/dive focused; do not rely only on generic weapon defaults. |
| `b_grunt` | `grunt` | Grunt-specific data supplements shared bot data; NPC weapon selection is also builder/world-config driven. |
| `b_mozer` | `mozer`, `girder`, `idles` | Girder use and Mozer-specific tuning are represented in data. |
| `b_pred` | `predator` | Predator has its own attack tuning table; shared `weapon` table is absent, so its two attacks are class-specific rather than assumed generic weapons. |
| `b_probe` | `probe` | Probe-specific attack tuning supports the custom pinch/spin/ground-hit routines. |
| `b_scientist` | `scientist`, `idles` | Scientist-specific behavior data is present. |
| `b_scout` | `scout` | Scout-specific detection/alert values are data-backed; the class's weapon count default is zero. |
| `b_snarq` | `snarq`, `idles` | Snarq-specific attack/hover tuning supplements its generic bot data. |
| `b_titan` | `titan`, `sounds`, `idles` | Extensive Titan-specific combat, damage and sound data is present. |
| `b_vrat`, `b_vsentinel`, `b_vloader` | Vehicle, physics, engine, camera and per-vehicle tables | These are vehicle data maps, not standard bot weapon tables. Keep their vehicle physics, driver/gunner and attack data together. |
| `b_sboss` | Only `gen`, `mountaim`, `walk`, `jump`, `weapon` | The Queen's retail table has only shared bot tables, not a boss-specific attack/phase table. This matches the incomplete encounter source. |
| `b_sniper` | Only `gen`, `mountaim`, `walk`, `jump`, `weapon`; `MountAim` has 11 fields (the class expects 12) | The port has an 11→12 field remap with a 20-degree lock-angle fallback. The Sniper-specific data table is not in its game-data map; the class uses hard-coded sniper stats. `b_sniper` shares most `gen`/`walk`/`jump`/`weapon` values with `b_sboss`, including some unrelated-looking shared strings, so treat its table as a thin/inherited cut configuration and verify every model, bone, armor and debris dependency. |
| `b_blink` | `blink` plus a 47-field `gen` table | Indexed retail content, but the current source `CBotMiner` loader requests `b_miner` and `CBotGlitch` requests `b_glitch`. No current source string references `b_blink`; identify its intended actor through decoded entity placements before wiring it to either class. |
| `b_swarmer` | Not found in the local MST index/extracted table set | `CBotSwarmer` source asks for this table. Its absence from this image is a direct data dependency to resolve, even though the Swarmers' bite class code exists. |

The `CBotBuilder::NPCWeapon_e` allocator in `bot.cpp` can instantiate Hand, Laser, Spew, Flamer, Grenade, Blaster, Rivet, MagmaBomb, Tether, EMP, Staff, Rocket and Recruiter weapon classes. That is broader than the world-table parser: `NPCWeapon0` accepts Hand/Laser/Spew/Flamer/Blaster/Rocket/Rivet, while `NPCWeapon1` accepts only Hand/Grenade. Some bot classes set additional weapon types internally. The AI builder separately controls weapon count, attack rules and firing cadence, so a created weapon object alone does not guarantee the bot will select or fire it.

There is no Sniper-rifle or Water-Bomb entry in `NPCWeapon_e`. Sniper primary fire is a custom actor tracer path; the source-backed MagmaBomb is not evidence that the cut Water Bomb/Nuke Grenade works. The Ripper is a normal active weapon class, separate from these enemy-specific paths.

## Special enemies, emplacements and AI vehicles

| Actor | Source / data | Functionality and classification |
|---|---|---|
| **ZombieBoss (`BotZombieBoss`)** | `CBotZombieBoss` — `botzombieboss.cpp`; `b_zombiebos` | Active source class and dedicated ZombieBoss combat brain/rule set. Has custom boss attack/damage profiles and work code. This is distinct from Swarmer Queen. Still verify the boss table/animations and current port runtime. |
| **Swarmer Queen (no factory type in active path)** | `CBotSwarmerBoss` / `CSwarmerBossGame` — `botswarmerboss.cpp`, `botswarmerboss_data.cpp`, `swarmerbossgame.cpp`; `b_sboss` | A sphere-arena movement/animation prototype exists, but the three implementation files are compiled out with whole-file `#if 0`. The manager assumes a specific arena and named objects; the bot's active work loop does not wire a complete attack/damage/phase encounter. Not spawn-ready. |
| **SiteWeapon (`SiteWeapon`) / Sentry** | `CBotSiteWeapon` — `site_botWeapon.cpp`; `b_sentry` | A family of stationary mountable defenses, including wall sentry, pillbox and Rat Gun variants. Pillbox/Rat Gun use the Pill brain; these are emplacements, not mobile bots. AI config covers how a bot can mount/use a site weapon. |
| **AA Gun (`BotAAGun`)** | `CBotAAGun` — `botAAgun.cpp`; `b_aagun` in MST index (source spells it `b_AAGun`) | Dedicated emplacement with primary/secondary projectile, collision and damage code. Keep separate from the generic SiteWeapon variants when creating or selecting entity types. |
| **Rat vehicle (`VehicleRat`)** | `CVehicleRat` — `vehiclerat.cpp`; `b_vrat` | AI-driven vehicle with car mover and vehicle-specific attack rules. AI race is ambient until occupied; driver/faction controls hostility. Not a biped bot. |
| **Sentinel tank (`VehicleTank`)** | `CVehicleSentinel` — `vehiclesentinel.cpp`; `b_vsentinel` | AI vehicle; data/defaults describe a cannon and a second machine-gun/rocket-style slot, with car movement. Check actual level loadout. |
| **Vehicle Loader (`VehicleLoader`)** | `CVehicleLoader` — `vehicleloader.cpp`; `b_vloader` | AI vehicle actor with claw and machine-gun style attacks. Ambient race defaults; has its own vehicle control/attack path. |
| **Krunk (`BotKrunk`)** | `CBotKrunk` — `botkrunk.cpp`; `b_krunk` | A source-backed bot/NPC with its own builder/table and weapon routines. The class does not by itself imply hostility; mission/faction configuration determines how the encounter treats it. |

## Present actors that should not be mistaken for enemies

- **Glitch** (`CBotGlitch`, `BotGlitch`, `b_glitch`) is the protagonist/player actor with player inventory, weapon, HUD and possession logic.
- **Scientist, Krunk, Miner/Blink, Rat and Loader actors** can be friendly, neutral, possessed, or hostile depending on mission and serialized builder configuration. Their class names alone do not establish their team.
- **Projectiles, destructible objects, recruited bots and mountable weapons** have separate entity types; don't add them as ordinary bot subclasses just because they appear during a fight.

## Sniper test record and next-step checklist

The recovered Sniper sources are now in the port's entity factory and CMake source list, but the opt-in `-spawn-sniper-test` path is **not a reliable combat test yet**. A previous run asserted in `CBot::HandleCollision` and then flooded invalid collision/impact work; a generic collision fallback was added, but the latest user-observed run spawned the actor floating in front of the player/in the sky and then flung it away. That does not establish whether its target selection or weapon AI works.

There was also an `aibrainman_UninitSys` active-brain assertion on teardown when the test actor was created outside the normal level parser ownership path. Treat explicit test-actor cleanup as a required part of a repeatable test; don't leave a dynamically allocated bot/brain across level shutdown. The latest gameplay observations came from this non-grounded test placement, not from a campaign-authored Sniper encounter.

For the implementation chat, a low-risk order is:

1. **Make placement deterministic:** raycast against loaded world collision from a point above the intended test location, place the feet on the hit surface with a small clearance, and log ray hit/miss plus final position. If there is no surface hit, skip spawn rather than guessing a vertical offset.
2. **Give the test entity normal ownership:** tie creation/destruction to the current level/test harness and tear it down before `aibrainman_UninitSys`. Confirm bot, brain and mover counts return to baseline after leaving the level.
3. **Audit the Sniper AI mapping:** add/select a noncolliding type bit or an explicit supported default without assuming `ENTITY_BIT_UNSPECIFIED` triggers Sniper-specific defaults. Verify race, targetability, mover, weapon slot and attack rule set in logs.
4. **Only then test combat:** confirm model palette/world transform, target acquisition, line of sight, damage-bearing tracer impact, received damage and death. Test the grapple as its own AI tactic after rifle behavior works; existing Fire2/player-control code alone does not create an AI grapple decision.

## Source and retail data lookup

- Factory/type names and bit assignments: `ma/App/ma/entity.h`, `ma/App/ma/entity.cpp`.
- Shared bot model and builder interfaces: `ma/App/ma/bot.h`.
- AI brain/mover fallback and thought tables: `ma/App/ma/Ai/AIBrainman.h`, `ma/App/ma/Ai/AIBrainman.cpp`.
- Per-actor faction/attack defaults: `ma/App/ma/Ai/AIBuilder.cpp`.
- Retail table-name index: `gamedata/mst_index.csv`. The index currently includes `b_aagun`, `b_blink`, `b_corrosive`, `b_elitegrd`, `b_glitch`, `b_grunt`, `b_jumper`, `b_krunk`, `b_miner`, `b_mortar`, `b_mozer`, `b_pred`, `b_probe`, `b_sboss`, `b_scientist`, `b_scout`, `b_sentry`, `b_slosh`, `b_snarq`, `b_sniper`, `b_titan`, `b_vloader`, `b_vrat`, `b_vsentinel`, `b_zom` and `b_zombiebos`. A name being indexed does not guarantee every associated code path is integrated or every field maps correctly.
- Source inclusion: `cmake/sources_ma.cmake` is an explicit source list; `CMakeLists.txt` adds the recovered Sniper/grapple sources. The Queen files appear in the list but are disabled internally by `#if 0`.
- The companion [`cut-content-revival-audit.md`](cut-content-revival-audit.md) cross-checks all 18 archive leads, weapon/item registration, retail assets/tables, and static placement results. Dynamic script spawns and other platform/region discs remain outside that audit.

## Static entity placement by world

These are counts of serialized `Bot*`, `Vehicle*`, and `SiteWeapon` shape records in the local US GameCube `.wld` initializations. They are authored static placements, not the runtime number of actors; entries may be disabled, replaced, or spawned again by mission scripts. Fifty-six of the 59 worlds have at least one record of these types; `we_1victory.wld`, `wfhu_droid.wld`, and `wldparticl1.wld` have none.

| Serialized type | Static records | Serialized type | Static records |
|---|---:|---|---:|
| BotGrunt | 853 | BotJumper | 103 |
| BotTitan | 100 | SiteWeapon | 83 |
| BotZombie | 72 | VehicleRat | 67 |
| BotBlink (`CBotMiner`) | 57 | BotPred | 38 |
| BotEliteGuard | 31 | BotScientist | 28 |
| BotSnarq | 25 | VehicleLoader | 19 |
| VehicleTank | 19 | BotProbe | 15 |
| BotScout | 14 | BotCorrosive | 5 |
| BotAAGun | 3 | BotMortar | 2 |
| BotSlosh | 1 | BotZombieBoss | 1 |

The exhaustive per-world breakdown follows. It includes multiplayer worlds as well as campaign worlds; the archive/cut-content candidates are absent from the static types in both groups.

```text
we01multi01.wld    botjumper=1, bottitan=2, SiteWeapon=1
we02multi02.wld    botgrunt=4
we03multi03.wld    bottitan=3, vehicleloader=1, vehicletank=3
we04multi04.wld    boteliteguard=1, botjumper=1
we05multi05.wld    botjumper=2, botpred=2, vehicleloader=2
we06multi06.wld    botcorrosive=1, botjumper=2, bottitan=1, vehicletank=1
we07multi07.wld    botpred=2, bottitan=2, SiteWeapon=2
we08multi08.wld    bottitan=2, SiteWeapon=3, vehicleloader=1
we09multi09.wld    botjumper=1
we10multi10.wld    botjumper=2
we11multi11.wld    boteliteguard=2
we12multi12.wld    botAAgun=2, bottitan=2, vehiclerat=2, vehicletank=2
we14multi14.wld    botjumper=1, bottitan=1, vehicleloader=2, vehiclerat=1, vehicletank=1
we15multi15.wld    boteliteguard=1, botjumper=2, bottitan=2, vehicletank=2
webccolis01.wld    botblink=2, botgrunt=8, botjumper=2, bottitan=2
webccolis02.wld    botblink=2, boteliteguard=1, botgrunt=11, bottitan=1, botzombie=7
webccolis03.wld    botcorrosive=1, botgrunt=11, botpred=1
webccolis04.wld    botcorrosive=1, botgrunt=3
wecdsneak01.wld    botgrunt=23, botpred=1, botscout=8, bottitan=4, SiteWeapon=5
wecdsneak02.wld    botgrunt=14, botjumper=2, botpred=2, SiteWeapon=2, vehicletank=4
wecffacty01.wld    botblink=9, botgrunt=7, botscientist=7, SiteWeapon=4
wecrruins01.wld    botgrunt=23, botpred=1, bottitan=1, botzombie=14, SiteWeapon=4
wecrruins02.wld    botgrunt=25, botmortar=2, bottitan=2, botzombie=4, SiteWeapon=3
wediinvas01.wld    botblink=9, boteliteguard=8, botgrunt=11, botjumper=11, SiteWeapon=3
wedmmines01.wld    botblink=13, botgrunt=15
wedmmines02.wld    botblink=8, botgrunt=46, botprobe=3
wedmmines03.wld    botgrunt=35, botsnarq=5, vehicleloader=3
wedttown_01.wld    botblink=8, boteliteguard=1, botgrunt=13, vehicleloader=2
wemccity_01.wld    botblink=2, botgrunt=17, botpred=3, bottitan=6, SiteWeapon=3
wemccity_02.wld    botgrunt=19, botpred=1, bottitan=4, SiteWeapon=3
wemccity_03.wld    botblink=2, botgrunt=18, bottitan=6, SiteWeapon=2, vehicletank=2
wemccity_05.wld    botgrunt=32, bottitan=6, SiteWeapon=6, vehiclerat=1
wermmorbot1.wld    botgrunt=19, botjumper=16, botpred=2, botprobe=8, vehicleloader=1
wermmorbot2.wld    botgrunt=25, botjumper=3, botpred=11, botprobe=2, bottitan=5
werrreactr1.wld    botgrunt=31, botjumper=7, botslosh=1, vehicleloader=3
werrreactr2.wld    botgrunt=23, botprobe=2, botsnarq=9
wesccorros1.wld    botcorrosive=1, botjumper=3
weshhangr01.wld    botgrunt=12, botjumper=6, botscientist=4, botsnarq=4, bottitan=5, vehicleloader=1
wesrrepair1.wld    botcorrosive=1, botjumper=9, botpred=2, botsnarq=3, bottitan=4
wessstatn01.wld    botgrunt=15, botjumper=14, botscientist=6, SiteWeapon=8
wessstatn02.wld    botgrunt=27, botjumper=18, bottitan=4, SiteWeapon=7
wewccomm_01.wld   botgrunt=57, botscout=3, bottitan=5, SiteWeapon=9
wewccomm_02.wld   botblink=2, botgrunt=30, botscout=3, bottitan=9, SiteWeapon=3, vehicletank=2
wewccomm_03.wld   botgrunt=26, botsnarq=4, bottitan=5, SiteWeapon=2, vehicletank=2
wewchold_01.wld   botAAgun=1, botgrunt=8, botpred=2, vehiclerat=8
wewhchase01.wld   botgrunt=52, botpred=7, vehiclerat=25
wewjjourn01.wld   botgrunt=26, botzombie=22
wewjjourn02.wld   botgrunt=6, botzombie=15
wewjjourn03.wld   botgrunt=13, botzombie=7, SiteWeapon=4, vehiclerat=1
wewkrockt01.wld   botgrunt=32, vehiclerat=2
wewrresrch1.wld   botgrunt=18, botscientist=7, bottitan=3, SiteWeapon=3, vehiclerat=4
wewrresrch2.wld   boteliteguard=11, botgrunt=26, SiteWeapon=4
wewrresrch3.wld   boteliteguard=6, botgrunt=5, botscientist=4, bottitan=4
wewrresrch4.wld   botgrunt=28, botpred=1, bottitan=9, SiteWeapon=2, vehicleloader=3
wewtrace_01.wld   botgrunt=39, vehiclerat=23
wewzzombi01.wld   botzombie=3, botzombieboss=1
```

Rare placements are useful candidates when checking how fully a class works in its authored context: `BotAAGun` has 3 records (`we12multi12`, `wewchold_01`); `BotCorrosive` has 5 (`we06multi06`, `webccolis03`, `webccolis04`, `wesccorros1`, `wesrrepair1`); `BotMortar` has 2 (both in `wecrruins02`); `BotSlosh` has 1 (`werrreactr1`); and `BotZombieBoss` has 1 (`wewzzombi01`). The 57 `BotBlink` records are spread across 10 worlds; source maps that serialized type to `CBotMiner`, so these are not evidence for an Early Glitch encounter. There are no static `BotSniper`, `BotSwarmer`, Queen, or Berserker records in these worlds.
