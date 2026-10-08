Night Sneak fabricator release (2026-10-06): campaign bots complete their short
exit GOTO before reacting to combat; alarm orders cannot strand them inside the
machine. Solo and local co-op share the correction, while PvP rules remain unchanged.
See docs/fabricator-release-20261006.md for focused native verification and limits.

## 2026-10-06: Secret Rendezvous countdown and ending

PC DET-pack HUD cleanup now follows countdown ownership across possession changes.
Finite 2D cinematic player dialogue finishes naturally (GL_22m_020: 3.815 authored,
4.556-second sample). This mission's co-op ending restores the story actor to Glitch
and releases the console before moving him. See docs/rendezvous-possession-timer-20261006.md
and build/logs/rendezvous-install-20261006.json. Vendor chatter, intermittent chamber
blockage and the unidentified end assertion remain unconfirmed.

## 2026-10-06: Sniper's Lair completion

The three Agent Goff quest pickups now send the missing retail `goffpickup`
notification; collecting the third completes level 28 in solo and co-op. Native
solo and two-player authored-pickup runs passed through the mission-complete
screen. Four-player testing required an unavailable fourth controller.
See `docs/goff-part-completion-20261006.md` and the install manifest in
`build/logs/goff-parts-install-20261006.json` for deployment state.

Factory co-op crosshair recovery (2026-10-06): safe box exit clears empty-weapon reticle overrides for every player. 25 native checks pass, including both reticles. Solo/PvP unchanged. Details: `docs/crosshair-profile-unlock-20261006.md`.

Co-op encounter update (2026-10-06): 37 audited one-shot enemy/map events accept
one actual crossing when the living group is nearby, without enlarging volumes.
Elevators/transport/exits and unlisted events retain their rules; solo/PvP handling
is unchanged. 1,871 offline checks and 9 native checks pass. See
[scope and installation](docs/coop-encounter-groups-20261006.md).


Factory inspection co-op failure recovery (2026-10-05): multiple/repeated
open lockers now trigger the authored closing warning and return P1 control,
with chip search retained. Solo rules remain original. Verified 17/0 native;
installed EXE `F078E3D4F070570A540734A0C98D60B1996AD49D94201CB6193A90C11E5B8A47`. Details: `docs/spy-inspection-warning-20261005.md`.

# Metal Arms: Glitch in the System - PC source port

Local controller ownership (2026-10-05): the controller opening co-op/PvP
selection stays with P1; keyboard entry still supports one-controller P2.
Devices bind when players join. Unclaimed hotplug can go to P1, while partner
controllers retain their ownership through disconnect/reconnect. Normal solo
allows P1 to switch keyboard/controller without assigning another player.
Input tests and 480 production routing checks pass; physical pad selection and
hotplug remain manual checks. See `docs/p1-controller-ownership-20261005.md`.

Factory packing correction (2026-10-05): retail box animation now advances;
P1 stays carried through delivery, with movement/jump blocked until breakout.
Co-op partners watch the shared sequence, then recover beside grounded P1 with
controls, HUD and independent views restored. Solo receives the animation and
containment fixes too. Per-slingshot animation config ownership fixes the
observed multiplayer teardown leak. Native packing: solo 17/0, co-op 23/0;
production input/camera/party/instructor checks pass. Physical 3/4-player packing
remains unverified. See `docs/spy-packing-recovery-20261005.md`.

Factory instructor correction (2026-10-05): local co-op commands now require
each player to reach their own target, face correctly and perform required jumps
before advancing. Forgiving placement remains; doing nothing no longer passes.
Partner hologram allocation/forward transforms are fixed. Guides now flash and
fade on the original timing (117 production / 42 native co-op checks). See
`docs/spy-hologram-flash-20261005.md`.
Final native checks: co-op 41, solo opening 9; production validator/resource checks
91. Physical 3/4-player instruction remains unverified. Details:
`docs/spy-instructor-challenge-20261005.md`.

Factory co-op follow-up (2026-10-05): post-assembly inspection starts with P1's
neutral owning camera; partners walk the authored escort with independent
views and regain controls automatically. Instruction uses spaced lanes, distinct
markers, wider co-op tolerance and dialogue-safe validation. Solo limits remain
retail. Final native checks: instructor 34, escort/camera 11, solo 9; production
validator 23 mode/lane/tolerance checks. Physical 3/4-player instruction remains
unverified. See `docs/spy-factory-escort-leeway-20261005.md`.

Work in progress: a native 32-bit Windows build of Swingin' Ape's engine (Fang2) and
game, targeting the retail **GameCube** data set.

The original source lives under `ma/` and is kept as close to untouched as possible.
`main` is the verbatim source drop; all porting work is on the `x86-port` branch, so
`git diff main..x86-port` is exactly what was changed.

Spy Factory's instructor now supports the co-op team with separate training
lanes, each player's own command/jump checks, matching unarmed/HUD state and
destination markers. Instructor failure or an actual death retries the team;
real checkpoint restore keeps partners at training rather than the opening room.
The authored exit regroups and equips everyone for combat. Later scripted stage
transitions recover partner state; partners can search lockers and supply the
objective chip. Factory camera transitions use the owning controller's view,
avoiding stale P2 spectator angles. Solo failure rules and the NPC demonstration
remain intact. The tested Release EXE/PDB are installed with a verified backup.
31 targeted native co-op checks, 9 solo opening checks and 149 production
solo/2-4-player mode/objective checks pass. Full factory and physical 3/4-controller
playthrough verification remains. See `docs/spy-factory-party-20261005.md`.

Spy Factory now broadcasts Agent Shhh's native transmissions to every co-op
HUD/antenna while playing the audio once. Its shared factory actor/body-part
rig belongs to P1, leaving partner models independent. The claw camera audio
callback accepts every active co-op listener, fixing the `!uPlayerIndex`
assertion when the wrench disassembles P1 and the head claw picks him up.
Native Debug checks now cover all three claw pickups, conveyor assembly,
post-assembly Shhh transmission and partner recovery with no assertions or
crashes. The authored headless body rebuild lets P1 move to each next grate.
After Shhh finishes, active co-op partners join rebuilt P1 at checked positions;
dead partners revive with their own body and controls before the guide walks.
The guide's lethal inspection punishment is disabled in local co-op because
it assumes a solo mission restart. Partners cannot block/push his scripted
movement; world obstacle avoidance remains enabled. Solo inspection rules
remain intact. Final native verification passes 37 checks through partner
revival and deliberately disturbed inspection, with no assertions or crashes.
The Release EXE is installed with a backup and verified hashes.

Factory assembly animation loading now supports compatible shared retail key
ranges, converting each scalar once. Endpoint/duplicate animation timestamps
hold a key safely instead of asserting or dividing by zero. These animation
fixes apply globally; incompatible overlaps and bounds errors still reject.
Production conversion checks cover all supported retail animations and malformed
aliases. The factory's complete remaining mission is still being playtested.

Co-op controller sensitivity now persists per saved campaign player across
sessions, including players sharing one personal profile. Settings saves
update only sensitivity, preserving checkpoint progress and inventory and
leaving personal solo/PvP profiles intact. Native disk-reload checks pass.
These fixes are installed in the Release EXE with a verified backup.

Spy Factory's control handoff now restores P1's human controls to the same
body the stage gave to AI in local co-op. Previously it restored the last
processed player, leaving P1 unresponsive after the intro. Fresh-start and
checkpoint-restart native checks pass, with P2 grounded and independently
controlled afterward. Solo checks also pass. Release update installed with
a backup and verified hashes; the authored opening transmission still plays.

Mil City Hub's Cap'n Peanuts escape conversation now accepts either player's
interaction in local co-op. Retail checks only its cached story-player handle;
the adapter is restricted to that NPC/world, preserving solo/PvP and other
actions. Native P2 interaction starts the authored scene (3 checks pass).
The performance profiler's gameplay button chord now requires `-debug-info`
on PC. The reported building-corner floor hole remains unlocated/unfixed.

Co-op vendor audio now follows the occupied shop location, releases its music
pause on departure, preserves transmissions that replace its shared stream,
and resumes mission music after shopping. Solo/PvP behavior is preserved.
Production-method checks and a native departure test pass; update installed
with backup and verified hashes. You Know the Drill's exit pipe lift works in
a fresh co-op test; the previous run never logged entry to the upper exit.
Its post-explosion checkpoint state remains unverified. See
[vendor audio and Drill exit](docs/vendor-audio-drill-exit-20261004.md).

Co-op scripts now refresh confirmed cached `Bot_GetPlayer` globals to the living
story actor before queued events run, fixing You Know the Drill's elevator when
P1 dies and P2 triggers the encounter. Strict campaign co-op scope; solo/PvP
unchanged. Retail VM checks and live P1-dead/P2 elevator ride pass; Debug/Release
pass and update is installed with backup/verified hashes.
See [cached-player recovery](docs/coop-cached-player-recovery-20261004.md).

Co-op cinematic camera sharing now excludes a player shopping at Shady/Slim.
The shopper keeps the vendor view while partners retain independent views and
movement. The existing camera fixture passes 1,141 checks; Debug/Release pass.
Installed with the elevator fix. Native gameplay checks and snapshots confirm
private shop views, partner visibility/controls, separate purchase funds and
normal exit. See [vendor camera isolation](docs/vendor-camera-isolation-20261004.md).

At the user's request, Krunk's asylum exit now opens after ten enemy deaths while
all fourteen authored bots/spawns remain. This PC runtime script override applies
to solo and co-op. The real retail script passes an offline AMX VM regression;
Debug/Release pass and update is installed with backup/verified hashes. No game
was launched. The user confirms the ten-kill exit works in co-op gameplay.
See [the asylum kill-goal change](docs/asylum-kill-goal-20261004.md).

Search for Krunk's booth-door cutscene now sends its last elite guard farther inside
the existing arrival trigger. The retail script waits for all four guards, but its
original destination permits normal AI stopping outside the trigger; the user's
log records only three arrivals. The fix preserves normal movement and scripted
door/camera/checkpoint completion, applying only to this campaign scene.
Debug/Release and offline checks pass. Installed with backup and verified hashes
after the game closed; no game was launched. Actual scene replay remains unverified.
See [the booth arrival diagnosis and checks](docs/booth-arrival-20261004.md).

Mil Comm Centre's rising Titan loot now resets its collision exclusion list per query,
fixing the reported 100-entry overflow in chaingun tracker append. Hold Your Ground's
added co-op turrets use Fang allocation/deletion, removing the loading assertion.
Those runtime guns also register for normal world-owned deletion so their brains
and entity entries are cleaned before resource teardown; this addresses the later
mission-end assertion and level-complete crash through released memory.
The user confirmed both turrets work with the mission after continuing past that assertion.
Debug/Release and offline regressions pass, including 6,008 teardown checks.
Installed the combined update in normal `build/Release` after the game crashed/closed,
with backup and verified staged hashes. No game was launched. Actual mission-end
transition with the new build remains unverified.
See [the assertion diagnosis and checks](docs/mil-assertions-20261004.md).

Mil Comm Centre's prison chip now loads from the retail six-field loot entry. The old parser
rejected `prison2`'s `chip 1 1 1 X safe`, leaving the guard without its required drop in both
solo and co-op. Normal loot quantities and pickup/story requirements are retained. See
[the diagnosis and checks](docs/retail-prison-chip-20261003.md). Reload the mission for the fix.
Debug/Release and offline regressions pass; installed in normal `build/Release` with verified
hashes and backup. No game was launched. Actual pickup/panel activation awaits gameplay testing.

The two freed cage zombies in They Live now enter the normal ally-follow system in solo and
campaign co-op. A narrow exception repairs their otherwise-rejected scripted recruitment;
follow thoughts reacquire after checkpoint restore. P1 leads, with a surviving partner while
P1 is dead. See [the cage-ally report](docs/caged-zombie-follow-20261003.md). The user also
confirmed fence transparency in gameplay.
Debug/Release and offline checks pass. Installed together with the previously staged RAT
runtime follow-up in normal `build/Release` after the game closed; verified hashes and backup.
No game was launched. Route navigation and the new RAT runtime behavior await gameplay checks.

A follow-up guards pause confirmations against stale input and logs checkpoint restore callers.
It also corrects the RAT controller camera's vertical input, preserves the human gunner model,
initializes jet loops immediately, refreshes positional sound gain, and stabilizes PC sphere/edge
collision arithmetic. Debug/Release and offline regressions pass; this update is staged in
`build/pending-update/Release` while the user's existing game stayed open, then installed with
the cage-ally update above after it closed. The user confirmed
the earlier RAT texture fixes. See [the follow-up report](docs/checkpoint-rat-runtime-20261003.md)
for evidence and remaining gameplay checks. No game was launched.

The RAT gunner's vertical aim is reversed, and the ZombieBot King intro now places co-op partners
inside after the story character's scripted relocation. PC slingshot release no longer silently
fails a frame-sensitive release-speed threshold; transient stance gates retain the held trigger.
Cutout surface passes repeat alpha rejection to keep fence holes transparent despite the port's
relaxed multipass depth test. Reflection shaders survive vertex-buffer changes, fullbright uses
baked instance colors, and CMPR/RGB5A3 decoding is corrected. See
[the implementation and validation report](docs/rat-spawn-sling-render-20261003.md).
Debug/Release, production-method regressions and the two-pass GPU fence fixture pass. Installed
in normal `build/Release` with verified hashes and backup; gameplay confirmation, including full
RAT black-surface coverage, remains pending. No game was launched.

Weapon swapping now synchronizes the equipped weapon's visibility with Glitch and refreshes mesh
flags, preventing an outgoing weapon's temporary hidden state from propagating through swaps.
611 offline checks pass. This guards a possible cause of the reported post-valve invisible weapons;
the exact trigger and the separate valve position correction still need confirmation. No weapon
firing-rate or audio changes were made after the user's clarifications.
Debug/Release builds pass. This visibility patch is now included in the installed update above.

Co-op join panels now keep consistent screen proportions and thin borders at widescreen and
high resolutions. Partners render the active cinematic camera in their own split viewport,
including its live motion, shake and lens, while retaining their normal camera controllers for
the return to play. See [the cinematic/UI report](docs/coop-cinema-ui-20261003.md) for design and
851 offline checks. Debug/Release builds and existing regressions passed; the combined update is
installed in normal `build/Release` with verified hashes. Previous binaries are backed up in
`build/backups/pre-coop-cinema-ui-20261003-173526`. In-game visual confirmation is pending;
no game was launched.

The campaign playthrough update restores shared radio visuals and vendor history/recovery,
adds checked initial partner placement and whole-RAT targeting exclusions, and plays authored
ending movies at completion with effects-volume control and additional headroom. Wasteland
Journey's invisible/harmless liquid flags, parent movement and liquid checkpoint state now work;
baked external lighting avoids incompatible texture-lightmap shaders. The communications-centre
Titan-only entrance also accepts player weapons in co-op. See
[`docs/coop-playthrough-20261003.md`](docs/coop-playthrough-20261003.md) for evidence, offline checks
and unresolved visual/audio reports. Debug/Release builds pass and the update is installed in
normal `build/Release` with verified hashes and a backup. Gameplay confirmation is pending;
no game was launched.

PvP now starts independent local controller routing on its join screen, including after returning
from co-op. Keyboard/mouse and the first controller no longer merge into P1's slot. Routing stays
through match setup/gameplay; Back restores the configured main-menu layout, while co-op retains
its campaign path. Offline production menu/input checks pass 145 cases; gameplay confirmation
is pending. The combined update is installed in normal `build/Release`; previous binaries are
backed up, matching staged hashes verified, and no game launched. It includes the Scatter Blaster
infinite-ammo fix below.

Scatter Blaster firing with infinite ammo now avoids indexing past the visible shell array when
its clip stays full. Both firing paths retain shells when no round is consumed and remove the
proper shell for finite ammo. Production-method offline regression passes 8,347 checks and the
existing cheat fixture passes 413. Gameplay confirmation remains pending; no game was launched.

Hold Your Ground now has an experimental co-op defense setup: P1 keeps the original AA gun,
P2-P4 receive extra guns beside it and board automatically after P1's intro. Waves wait for every
player's gun/camera, and failure/retry is shared. Aim limits, mortar support and durability match
the original; each player has independent controls, camera and reticle. Placement uses provisional
16-unit lateral spacing and still needs platform/sightline verification in-game. No extra static
foundations or wave balancing changes were added. Production-helper fixtures pass 189 checks,
existing checkpoint/gate/scene fixtures pass, and Debug/Release builds pass. No game was launched.

The latest co-op finishing update fixes a safe-checkpoint placement query admitting Glitch's mesh
through a mask intended to exclude bots. Safe floor/body checks now explicitly filter bot meshes
and attached equipment; the earlier pending Clean Up checkpoint was not disabled by Nuts of Steel.
Local co-op racing RATs reserve the friendly scripted NPC gunner's seat for a human: P2 boards
automatically beside P1 once ready, with manual boarding available. Second-occupant entry preserves
vehicle health and driver input ownership. Mission timers display on every co-op HUD, including
vehicle occupants. The new fixture passes 178 offline checkpoint/RAT/timer checks; existing gate
and scene checks and Debug/Release builds pass. Actual revive, RAT driving/gunning and checkpoint
restart still need gameplay confirmation. Additional vehicles for P3/P4 and other single-seat
vehicle missions remain open; Hold Your Ground now has the experimental setup above. With the game closed, normal Release EXE/PDB/Bink
were synchronized with the staged build and hashes checked; no game was launched.

The latest waterfall screenshot exposed a separate near-particle camera handoff defect. Windows
emulated particles draw view-space quads with an identity graphics camera but did not restore it;
sorted transparent world waterfalls could then inherit that camera and shift toward the screen.
That path now restores the active view, including mirrors. World-geometry setup also establishes
the current camera and resets both world-transform paths. The offline regression failed before
the fix and now passes 2,592 projection/particle checks across P1-P4. Debug/Release builds and the
existing waterfall and scene checks pass. Normal Release and pending-update copies are synchronized;
no game launched. The user confirmed the nearby cave waterfall fix in gameplay afterward.

The previous finishing update fixes repeated weapon-selector attempts from movement and closing-list
reopens. A single tap keeps the retail reload/secondary-target-clear behavior; double-tapping either
selection button within 0.3 seconds cycles that player's weapons or throwables. Hold for 0.3 seconds
to open the list. Empty and unavailable weapons are skipped by quick cycling. Pause Quit reached
the main menu but crashed on a freed label atlas; its cached texture is now cleared with the wrapper
resource frame. Debug/Release builds and 6,147 offline selection/menu checks pass. Normal Release
and the pending-update copy contain this update; gameplay confirmation is pending. No game launched.

Finishing checks (2026-09-30): the user confirmed co-op save/reload, checkpoint revives with the
vanilla animation, partner cutscene cameras without letterbox bars, and alt-tab stability. Play is
smooth with no specific audio issues reported. Controller hot-swapping, a complete campaign
progression playthrough and additional liquid surfaces remain to be checked. Experimental Nuke and
Water Grenades have been removed; ordinary Coring Charges and EMP Grenades use their original paths.
Existing saved experimental grenades restore as their ordinary counterparts, retaining ammo.
Water in Seal the Deal's cave and either player's Seal the Mines possession objective now work and
were confirmed by the user. Co-op world weapons can be collected once by each player, with visibility
and checkpoint claims tracked per player. Shops serve the activating player and use that player's
separate washer wallet. Unsupported legacy upgrade-kit catalog entries are omitted; see Known issues.
The later waterfall follow-up repairs invalid fractional-power animation vertices, aligns every
waterfall vertex declaration with its three UV pairs, explicitly establishes depth/alpha-test state,
and updates waterfall shape/scroll once per frame across split-screen views. Debug and staged Release
builds pass, with 6,901 offline waterfall checks covering 544 complete draw passes. Visual confirmation
is pending. This update was initially staged while the game ran; it is now included in normal Release
and `build/pending-update/Release` with the selection/menu fixes above.

The October 2 follow-up lets cutscene spectators land before holding them stationary and preserves
moving-platform carry. Door/lift checkpoints now retain mesh animation state and rebuild the saved
physical pose, preserving pickup/open timers. This repairs a restore defect but the reported Mines
elevator failure still needs a gameplay check. Waiting prompts have a 0.35-second display delay.
The recurring moving square prompted an additional liquid fix: FVF passes now reset their separate
fixed-function world matrix, alongside shader constants. Visual/physical recurrence is unverified.
Unfocused client areas use an explicit arrow cursor instead of inheriting the startup spinner.
Debug/Release builds pass, with 400 gate checks and 434 scene/lift/liquid fixture checks offline.
Normal Release and pending-update copies are synchronized; the game was not launched.

The intermittent blue circle still needs a recurrence check, and Discord's visible client activity
has not been inspected.

A later Mines 1 death exposed a checkpoint firing while the surviving partner was airborne: the
downed player revived at the death location below the route. Revival now checks ground and body
clearance before resurrecting, and defers saving until a safe partner position is available. Dead
bodies cannot trigger timed fall recovery; a team restore overrides a pending save. Both builds
pass, but gameplay verification of this fix is pending. The user requested no further game launches
at this time. The opt-in `revive` regression is ready for the next authorized testing session.

The October 2 progression fixes select a living Glitch for co-op story scenes and keep that actor
through the cutscene, including AI movement and scripted fall actions. Spectators animate in place;
scene actors can cross later triggers without waiting for their disabled partners. Mines 1's terminal
exit parks early arrivals, and checkpoint restores clear prior arrival records. Fresh profiles now
load the difficulty screen's correct retail assets instead of displaying the Reset layout. Existing
profiles passed CRC checks and were backed up; the missing old test profile was not recovered.
Debug/Release builds and offline actor/menu checks pass. The user confirmed a cutscene progressed
with P1 dead and P2 temporarily serving as the story actor; remaining progression checks are pending.
The agent has not launched the game under the user's no-launch instruction.

The PC pause menu's `Quit to Main Menu` returns to the retail front end after the existing
confirmation. This also applies to direct `-mission`/`-level` launches. Main-menu Quit exits the app.

`Pause > Cheats` applies to the player who paused: infinite ammo, invulnerability, refill ammo,
give 1,000 washers, give available weapons, max weapon upgrades and heal. Weapon grants use assets
already prepared for the current level and report partial availability; grants/upgrades require
controlling the original live Glitch. Heal restores the current live body without reviving a
downed player. Toggles last until returning to the main menu and are not saved; inventory grants
follow normal checkpoint/profile saving. Co-op inventories and wallets remain per player.
The offline fixture `python tools/test_pc_cheats.py` passes 413 checks across P1-P4; gameplay/UI
verification is pending under the user's no-launch instruction.

## Rescue dialogue and flamethrower audio (2026-10-04, installed)

Natural forced-2D cutscene dialogue now lets its finite voice clip finish when
the retail animation schedule is shorter than the decoded audio. Glitch's late
Slosh rescue line was scheduled for 2.25 seconds versus 2.71 seconds of speech.
Explicit cancellation/skip still stops immediately; ambient talk, infinite loops
and damaged voice behavior are preserved. This is a PC campaign audio repair,
including solo, rather than a co-op camera change.

The flamethrower's six retail sound groups now load from the columns after its
inserted particle fields and resolve against the loaded level banks. This
restores fire, stop and reload/empty sounds globally in the PC port. The Windows
virtual sound-instance budget is now 512 instead of 160, which the live Reactor
run exhausted; audible DirectSound mixing limits are unchanged.

The two Reactor defaults are retail **ambience** streams (`ser_reactor` and
`ser_genint`), not continuous music themes. Native logs and read-only inspection
of the user's live process confirmed playback and nonzero, unpaused gains.

Validation: native rescue-clip/talk-system checks pass in co-op and solo (7 each),
256-effect allocation/completion/reuse stress passes (4), and Slosh fire/stop
stress passes (2), with `FlameLoop/FlameOut` playback in the log. Final runs have
no assertions or emitter-creation failures; offline cinematic/UI checks: 1178.
Debug/Release compile. Full authored scene replay remains a playthrough check.

Installed with the Mil City captain/profiler fixes after the user closed the
game. EXE `build/Release/ma_port.exe` SHA256:
`B819D9E91C272CBD5DEB17FAFFE6FFBE6C70418F820C56C0FF4967F898271BA8`.
Previous binaries: `build/backups/pre-city-audio-20261004-194243`.
Installed Release captain interaction check passes all 3 checks without script
errors or assertions; file hashes match the staged build.

## Reactor floor pads and P1 camera restoration (2026-10-04)

- Verified Reactor door, bridge and electrical floor pads now accept one living
  original player in co-op. Both sides of the timed Reactor Core door can
  reopen it independently, preventing separated players from waiting for each
  other. Exact world/name allowlists preserve other team gathers, retail script
  timing, solo and PvP. Production gate checks: 834 pass; native four-player
  reported door-pair test: both sides pass with P2 alone.
- Manual cutscene deactivation restores P1's camera to P1's current body rather
  than the surviving story actor. Production camera regression reproduced the
  previous wrong-target behavior; 1,178 camera/menu checks pass after the fix.
  Native visible manual scene with P1 dead, scene exit and subsequent P1 revival
  passes six checks with no assertions/script/audio errors. Snapshots confirm
  shared cinematic view followed by separate owner views. The exact authored
  Slosh sequence still needs a playthrough retest.
- Debug/Release compile. Installed normal Release including the previously
  staged I, Predator beam change. EXE SHA-256
  `0EAE902A0AC4A7A878AB4426588005EC81D940D8680E5F535DF702DCABE4E510`;
  backup `build/backups/pre-reactor-pads-camera-20261004-175210`.

## Morbot floor pads accept one co-op Glitch (2026-10-04)

- Active retail `WERMmorbot1` pads `circtrig1`, `circtrig2`, and `fliptrig`
  now release their authored enter event for one living original player, using
  that actual actor. This is an exact mission/name exception inside the co-op
  gate; solo/PvP, possession/named-objective filters, kill volumes, checkpoints,
  elevators, mission exits, and every other team gate keep their existing rules.
  No geometry/activation-radius change. Script `xemo_mbot.sma` confirms these
  three events only operate `leftdoor`, `rightdoor`, and `flipdoor` structures.
- Offline production gate/crossing/occupant fixture: 460 checks, covering every
  P1-P4 operator with 2-4 players, wrong mission, downed actors, solo/PvP and
  existing team gates. Debug/Release compile. Native four-player run verifies
  P2 alone moves both circle structures to 0.93 and flipdoor to 1.00 while
  partners remain at spawn, no wait HUD and no assertions/script/audio errors:
  `build/logs/coop-floor-pads1-enter-20261004.log`.
- Initial native fixtures did not exercise the switch: long intro movie, then
  incorrect box-only assumption (pads are spheres), and relocation disabled
  tripping. Corrected to skip the intro through native test keys and enter with
  the real tripwire sphere center and tripping enabled. Test-only warps seen by
  user are not spawn changes. Ordinary spawn code is untouched.
- Initial candidate names from unused `xemorbot2.sma` are absent in the active
  `WERMmorbot2` world/scripts. Removed those candidates; do not broaden a gate
  exception from unused assets. Native floor-pads2 candidate run failed to find
  them and is not a pass. No second-mission floor override remains.
- User's screenshots at 13:58/14:13 and old Release log ending 14:23 predate the
  render-color fix installed at 14:50. Recorded assertion stack is decal vertex
  color conversion (`fdx8math.inl:221`), matching the previous byte-clamp fix.
  Airship correlation is plausible, not established as its sole source.
  Production aim/color checks 660 pass. Native two-player `WERMmorbot2` test
  assigns attack goals to the three retail airships; repeated firing is recorded
  (`SWDMl2lfire`) with no assertion/script/audio errors, two fixture checks pass:
  `build/logs/coop-airship-combat-20261004.log`. Not a full mission playthrough.

- Installed normal Release EXE/PDB/Bink after two closed-game checks; all three
  copies hash-verified. Backup `build/backups/pre-floor-pads-20261004-153356`;
  EXE SHA-256 `8C974F6C696F39BFE7B25C00FF83EFE6CB47429681C5E4109DCE6869C468AA59`.
  Default CMake Release output paths restored.

## Shared Chase turrets and seated co-op gunners (2026-10-04)

- Wasteland Chase (`WEWHchase01`, `player_rat`) attaches up to three extra RAT
  turret pods to the original story vehicle. Each partner controls a separate
  gun/camera; the existing NPC driver/path and story passengers stay intact.
  Pods use vehicle-owned `fnew` lifetime, manual work after vehicle movement,
  and normal remove/destroy cleanup. No extra vehicles or competing AI routes.
- Whole-RAT aiming/projectile skip lists include extra turret and occupant
  meshes, so teammates' pods do not absorb shots. Co-op human RAT gunners
  (including the primary seat and other missions) attach with glue, zero entry
  velocity, and disable body collision while seated. Exit restores the previous
  collision flag and detaches normally, including removal from the world.
  NPC/solo/PvP seating paths retain their prior behavior; the RAT stays damageable.
- PC RAT pitch no longer applies a second inversion after player controls already
  apply the preference. Pitch uses profile sensitivity and is capped to the
  authored yaw speed; gunner camera follows actual gun elevation for both input
  types rather than independently adding height. Native feel still needs user
  confirmation with their controller/preferences.
- Repeated `fdx8math.inl:221` assertions came from decal float-color conversion,
  not square-root/collision code. PC render color bytes now clamp negative/NaN
  to zero and overbright/infinite values to 255 before unsigned conversion.
- Empty co-op shop offers give the shopper a brief "No items available for you"
  prompt; no inventory/stock change. User confirms this feedback works. User
  also confirms Level 1 still waits for partners after cached-player changes.
- Debug/Release build; production runtime guards 44,535 and aim/color 660 checks.
  Native four-player boarding/movement/body-collision/exit checks pass, followed
  by normal return-to-menu teardown without assertions. Native two-player
  checkpoint save/reload and restored exit pass (nine checks total), log
  `build/logs/coop-rat-seat-checkpoint2-final-20261004.log`.
  A first checkpoint fixture checked asynchronous saving too early; corrected
  to check on the following frame. Full mission completion not exercised here.

- Installed normal `build/Release/ma_port.exe` after two closed-game process
  checks; EXE/PDB/Bink copies verified by SHA-256. Backup
  `build/backups/pre-rat-seat-20261004-145006`; EXE hash
  `BEFB2B150955C97CB71893D90691591A44CA3BFE60E8DAB50340930886739201`.
  Restored default CMake Release output paths after staging.
- Empty-shop native fixture did not reproduce an empty inventory offer state:
  the shop opened, so its full-inventory assertion failed. Log
  `build/logs/coop-shop-empty1-final-20261004.log`; do not count it as a pass.
  User's actual no-offer prompt is confirmed; stock/offer rules remain unchanged.

## Building

Requires Visual Studio 2022 (C++ workload, x86 tools), CMake 3.20+, Python 3.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Release --target ma_port

Output: `build/Release/ma_port.exe` (+ `binkw32.dll`). It must be 32-bit (see below). Release is the one
to play (about a third of Debug's CPU time per frame); it keeps symbols and frame pointers, so crash logs
and the stall sampler below work in it too. `--config Debug` builds `build/Debug/ma_port.exe` with the
engine's asserts.

## Running

Retail data is **not** in this repo. Put the extracted disc files in `gamedata/files`
(the `.mst` master file and the `Movies` folder), or point at them:

You can double-click `build/Release/ma_port.exe` or run it with no arguments. The launcher resolves
the default `gamedata/files` relative to its executable when the working directory is elsewhere. If
the data is stored separately, pass its directory with `-data`.

    ma_port -data <dir> [-mst <file>] [-res WxH] [-fullscreen] [-level <world> | -mission <world> [-coop 2-4] | -world-only <world>] [-log <file>] [-asset-log <file>] [-shots <dir>] [-shot-every <frames>] [-mouse-sensitivity <n>] [-aim-assist auto|on|off] [-input-layout shared|separate] [-button-prompts auto|keyboard|xbox|playstation] [-no-audio] [-debug-info] [-save-dir <dir>] [-console] [-port-diag] [-discord-app-id <id>|off] [-test-keys <s:vk,...>]

Display: Advanced Settings has Display Mode (Windowed / Fullscreen / Borderless), Resolution (the
adapter's fullscreen modes) and Field of View (-10 to +40 degrees on the third-person camera). They are
stored in `settings.ini` `[Display]` (`Mode`, `Resolution`, `FieldOfView`). Mode and resolution apply at the
next launch; the field of view applies at once. `-res`, `-fullscreen`, `-borderless` and `-windowed`
override the saved choice for one run. Fullscreen without a saved resolution, and borderless always, use
the desktop resolution (the desktop's size is offered as an extra windowed mode for this). On screens wider than 4:3 the view keeps the 4:3 vertical framing and shows more
at the sides (Hor+), and the process is DPI-aware so scaled desktops get real pixels.

`ma_port.exe` is a windowed app with no console window; `-console` opens one showing the log (engine
output and the level scripts' own print messages, such as "NONETRIPWIRE ENTER EVENT"). The log
file (`-log`, default `ma_port.log` in the working directory) always has everything. Fang's asset
loading log defaults to `ma_port_asset_log.txt`; use `-asset-log <file>` to give each concurrent run
its own asset log. `-port-diag`
(or `MA_PORT_DIAG=1`) adds the port's diagnostics: `PORT-HITCH` frames over 40 ms, `PORT-SND` the
first plays of each sound, `PORT-MIX` a snapshot of every playing sound's level every 2 seconds,
`PORT-TALK` bot dialog and `PORT-DUCK` audio ducking. They are off by default because writing them
every few seconds caused visible hitches.

Measuring: `-no-vsync` presents immediately. Under `-port-diag` a `PORT-PERF` line every 10 s gives the
frame rate, the worst frame and the time spent before Present, and a watchdog logs the game thread's call
stack (`PORT-STALL`) whenever a frame runs past 100 ms (`MA_PORT_STALL_MS` changes that). It found the
hitches fixed so far: the log being written on the game thread (it is written by a background thread
now), music/speech streams being opened and their buffers made on the game thread (a worker does the
whole load now), and movies waiting on Bink's file reads (Bink gets a 16 MB read-ahead now).

`-test-keys "62:0x1B,70:0x51"` presses those virtual keys (Escape, Q) that many seconds after start;
`g8:0x1B` presses Escape eight seconds after the first gameplay frame. These work without the window
having focus, so a test never takes the keyboard from the desktop. (`MA_PORT_TEST_KEYS` is the same.)
`-no-audio` skips game audio setup and mutes Bink movie tracks while keeping the video clock running.
The test window title includes `[TEST RUN - NO AUDIO]`; `-mute` keeps the audio path active but silences
game and Bink output. `port_run.py` and `mission_parallel.py` now leave audio on by default; pass `--mute`
only when a silent run is intentional.

For concurrent mission checks, use `python tools/mission_parallel.py --config Debug --seconds 75 --jobs 4 wewchold_01 wedttown_01 WEWHchase01 WEWJjourn01`.
Each instance gets separate engine, asset, and save paths. It reports load completion, script events
and errors, data warnings, allocations, asserts, crashes, frame timing, and stalls. `--jobs` caps the
number of simultaneous windows; audio is enabled by default and Discord is off.

Discord Rich Presence is on by default: "In the menus", or the level ("Level 4: Clean Up") with
"Campaign" / "CO-OP Campaign (N players)" / "Multiplayer: <game type> (N players)" and the elapsed time, under the port's own Discord
application (its ID is `_szDefaultDiscordAppId` in `port/main_win.cpp`; the application's name and
icon are what Discord shows as "Playing ..."). `-discord-app-id <id>` (or `MA_PORT_DISCORD_APP_ID`)
uses another application, and `-discord-app-id off` turns presence off. A Rich Presence image is
optional: upload one on the application's Rich Presence assets page and launch with
`-discord-large-image <asset-key>` (or `MA_PORT_DISCORD_LARGE_IMAGE`);
`-discord-large-text <tooltip>` / `MA_PORT_DISCORD_LARGE_TEXT` sets its tooltip. The port talks to the
local Discord client's pipe directly, so nothing else is installed, and it does nothing when Discord
isn't running. The log says when it connects, and quotes Discord's answer when Discord refuses the
application ID or an activity (for example an unknown asset key).

`-level <world>` starts the generic debug level path using `Level01` configuration.
`-mission <world>` resolves a registered single-player mission and uses its own configuration,
material table and normal level-loading path. Unknown/unregistered worlds fail explicitly.
`-world-only <world>` loads and converts the WLD resource, then exits before localized setup
and gameplay entity creation. These three launch modes are mutually exclusive.

`-coop 2` through `-coop 4` is a basic campaign player-slot prototype and requires `-mission`.
It keeps campaign rules active, assigns keyboard and XInput to separate ports by default, and creates
the normal split-screen player slots without persistent profiles. It does not add a character/bot
selector or implement campaign co-op behavior. Current smoke runs reached end-of-loading. A captured
frame showed the main view, but the lower split-screen view was malformed; treat this as an
initialization experiment, not a playable mode.

For the mission path with engine captures:

    ma_port -data gamedata/files -mission wecdsneak01 -log build/logs/mission.log -shots build/shots-mission -shot-every 1800

`-mission wecdsneak01` loads Night Sneak's configuration, attaches and initializes its eight
retail scripts (no unresolved natives, no script errors), and reaches gameplay. Objectives,
level transitions and saves have not been verified; this is not yet a complete campaign launch.

`tools/mst_list.py` lists/extracts a `.mst` master file (GameCube byte order).
`tools/gamedata_dump.py` inspects extracted binary `.csv` game-data tables as indexed JSON;
write reports under ignored `build/` because they contain retail asset values.
`tools/dol_vocab.py` recovers the retail game-data vocabularies (field type, conversion,
size and clamp range) from `gamedata/sys/main.dol` by following its `FGameDataMap_t` tables,
e.g. `python tools/dol_vocab.py gamedata/sys/main.dol LaserL1`. `tools/dol_xref.py` finds and
lists retail PowerPC code that references a data address (a minimal decoder, enough to see
which structure offsets feed which runtime fields). Use them to resolve schema drift from
retail evidence instead of guessing; keep their output under `build/`.

`tools/syntax_check.py` syntax-checks the C/C++ sources without MSVC, for sessions that cannot
build (Linux/cloud): clang against MinGW-w64 headers with flags that mimic the MSVC build. It
catches type errors and wrong API use, not everything MSVC would; it never links or runs.
`python3 tools/syntax_check.py --changed HEAD` checks the C/C++ files changed since HEAD.

`tools/mathdiff/mathdiff.py` runs the GC-layout math (`dx/fdx8gcmath_*.inl`, the scalar code this
build uses) and the shipped SSE math (`dx/fdx8math_*.inl`) on the same inputs as 32-bit Linux programs
and reports methods whose results differ. Known differences are documented in the tool; each was
checked against the retail GameCube code (`gc/fGCmath_*.inl`). Run it after changing the GC-layout math.

## Saves

Player profiles are saved in `%APPDATA%\Metal Arms PC Port\Saves` (override with `-save-dir <dir>`
or `MA_PORT_SAVE_DIR`); the log names the directory at startup. Each profile is one file,
`profile-<hex>.sav`, whose name is the profile name's UTF-16 code units in hex (profile names may
contain characters Windows file names can't, or differ only by case). Saves are written to a
`.tmp` copy that replaces the profile once complete. Profiles that earlier builds saved as
`profile-<name>` in the working directory are copied into the save directory at startup; the
originals are left in place. In-level checkpoints are memory-only, as on the consoles.

## Desktop controls

| Control | Action |
|---|---|
| WASD | Move (diagonal speed is normalized) |
| Mouse movement | Captures the mouse for raw mouse look (during gameplay, while the game has focus) |
| F1 | Turn automatic mouse look off (free cursor) / back on |
| Mouse / arrow keys | Look / turn |
| Space | Jump |
| E | Action / interact |
| F | Melee |
| Left / right mouse button | Primary / secondary fire (requires a supported weapon) |
| Q | Hold 0.3 s for the throwables (secondary) list |
| R | Tap to reload; hold 0.3 s for the weapons (primary) list |
| 1 / 2 / 3 / 4 | Quick-select up / right / down / left |
| Escape / Enter | Pause (gameplay); Escape also releases the mouse |
| Enter / Space | Menus: accept (in the pause menu Space selects; Enter resumes) |
| Escape | Menus: back (also leaves the pause menu and skips movies) |
| Q / E, Tab / Shift+Tab | Pause menu: previous / next page |
| Alt-Tab | Releases the mouse; moving it over the game again recaptures it |
| Alt-F4 | Close the game |

The user confirmed responsive mouse look and reported that some weapons appear to work.
Traced in source (keysmelee. Q and R were swapped
from the adapter's first mapping at the user's request. On the desktop a weapon list opens only after
its button is held for 0.3 s (`_WEAPONSELECT_HOLD_SECS` in `game.cpp`, pads included); a shorter tap
uses the retail callback directly, so R reloads without entering the list. Double-tap R/Q (or the
corresponding controller buttons) within 0.3 s to cycle weapons/throwables, wrapping past empty or
unavailable slots. Pending gestures clear when gameplay controls are interrupted. With a throwable equipped
and ammo available, right mouse maps to secondary fire and starts the throw. The HUD selection code
accepts W/S to scroll while a selection menu is held open; releasing the menu button equips the
selection. Throwable behavior has not yet been confirmed interactively.

Mouse look uses raw relative motion, applied as angular displacement without the controller's
acceleration curve or turn-speed cap. `-mouse-sensitivity 0.1` is the default, in degrees per
mouse count, before the game's look-sensitivity multiplier. Use `0.05` for half that speed.
Advanced Settings now includes **Mouse Sensitivity**, shown as a multiplier of the default raw
mouse speed (1.00x = 0.1 degrees per count). Arrow keys, the mouse wheel, and the value's left/right
click areas adjust it in 0.10x steps from 0.10x to 10.00x. Back or Accept saves it to the PC settings
file under Local AppData, independently of profiles. The existing Look Sensitivity setting still
multiplies the resulting look speed. A valid `-mouse-sensitivity` / `MA_PORT_MOUSE_SENSITIVITY`
override is shown as locked and never overwrites the saved preference.
If a PC preference cannot be written, Advanced Settings displays a warning. Dismiss it and
choose Back again to leave with the current session values; each exit retries the write.
Launch overrides do not trigger this warning.

Moving or clicking the mouse over the game during gameplay captures it; menus, Escape and losing
focus (Alt-Tab) release it, and the next movement over the game recaptures it. F1 switches
automatic capture off for a free cursor, and on again. All inputs return to neutral when the game
loses focus. Desktop defaults to non-inverted look;
loaded profiles retain their own setting.

Target assistance (reticle snapping, aim biasing, shot focusing) is tuned for sticks. By
default (`-aim-assist auto`) it is suspended while you aim with captured mouse look and returns
when the right stick aims; `-aim-assist on|off` (or `MA_PORT_AIM_ASSIST`) forces it.

### Front-end menus with the mouse

The front end (title menu through level select, multiplayer setup and settings) draws its own pointer,
the HUD's triangular reticle (`tfh_cross01`) used apex-up as an arrow, and hides the Windows cursor over
the window. Moving the pointer over an item selects it; a left click on an item picks it (A); a right
click is Back (B); clicking a button prompt ("Accept", "Back", ...) presses that button; the wheel steps
through lists (and pages sideways lists); over a selected setting (sound volumes, advanced controller
options, multiplayer rules) the wheel changes its value. The keyboard and pads keep working; using them
hides the pointer until the mouse moves again. Clicks are queued with their own positions, so fast clicks
on the name keyboard all land. While a menu draws its own pointer the mouse buttons are not also fed to
the triggers (a held right trigger starts the launch screen's level-unlock code and blocks input).
`MA_PORT_POINTER_DEBUG=1` outlines every hit box and logs each click's target.

The in-game pause menu uses the same in-engine pointer, and so do the settings screens opened from it.
Hovering a row selects it; left click selects it, the bottom prompts accept or resume, and right click
resumes. Click the upper left or right tab regions to change pause pages; the wheel moves the current
selection. Q/E and Tab/Shift+Tab also change pages. The pause menu reads its buttons through the menu
control map only for the moment it samples them, so the input layer treats a paused game as a menu
(`FLoop_bGamePaused` in `gamepad_Sample()`): the mouse is released for the pointer, Escape is Back
(which resumes), and Q is not also Back there. An Escape press that began before such a switch is
ignored until released, so pausing with Escape never immediately backs out again.

`-button-prompts auto` (the default) switches between keyboard key caps and retail Xbox art with the
most recently used input device. `keyboard` and `xbox` lock that choice. `playstation` uses generated
Cross, Circle, Triangle, and Square glyphs (a dark round button with the colored symbol) in front-end
and pause prompts and PlayStation wording in menu and in-world instructions; it needs no additional
assets. `MA_PORT_BUTTON_PROMPTS` accepts the same values. Ports without the keyboard (other players'
pads) always get pad wording. The pause menu draws key caps too (Space, Esc, and Q/E on its page tabs).

Prompt layout (`wpr_drawutils_DrawButtonOverlay()`): the retail layout floats each button icon above
and to the left of its text. On the PC every style's icon (pad art, PlayStation glyph, key cap) is
sized to the prompt's text line, centered on it and sits flush left of the text; prompts sharing a row
flow left to right without overlapping. The font's line metrics are measured from the prompts as they
print (`_fPromptLineHeightPerScale`); `_PROMPT_ICON_SIZE`, `_PROMPT_ICON_CENTER` and `_PROMPT_ART_FILL`
tune the icon.

Settings screens take clicks: the arrows beside the selected setting, On/Off (and 2-way/4-way) values
and the level bars (a click sets the level at that tick). Leaving Audio Levels or Advanced Settings with
Back (Escape, right click) keeps the changes, as on PC; on the consoles Back cancelled them. The
Controller Map shows a chart of the keys and mouse buttons (or the chosen pad style's buttons) beside
each action instead of the retail controller picture.

Profile-name entry accepts typed characters that the on-screen keyboard has (either case), spaces
(not first) and Backspace while the name screen is open; Enter is Done. Those keystrokes do not
trigger gameplay bindings, and Enter is not also START there. Storage text talks about the save folder
instead of memory cards (`wpr_datatypes_PcText()`; format becomes reset, blocks become space).

XInput controllers can connect after launch. `-input-layout shared` (default) puts the keyboard/mouse
and controller 1 on port 1 and controllers 2-4 on ports 2-4. `-input-layout separate` keeps the
keyboard/mouse alone on port 1 and puts controllers 1-3 on ports 2-4, so a keyboard player and pad
players are separate players (local co-op). `MA_PORT_INPUT_LAYOUT` is the environment equivalent.
The adapter intends A for jump, Y for action, B/X for weapon selection, triggers for fire,
and RB/right-stick click for melee (the same control map as the keys above). Controller mapping has automated coverage; physical controller
behavior and rumble still need verification. Legacy DirectInput-only pads are not supported by
the new desktop adapter.

Capture through the engine with `-shots build/shots -shot-every 1800`. This saves numbered BMPs
from the D3D back buffer. The console, `-log` output, CRT diagnostics, and shader/texture probe
environment variables remain available.

## What changed and why

| Area | Change |
|---|---|
| Build | New CMake project. Source lists are generated from the original `.vcproj` files (`tools/gen_sources.py`). 32-bit only. |
| Renderer | The engine targets Direct3D 8, which modern Windows SDKs no longer ship. `port/compat/d3d8.h` + `d3d8_compat.cpp` wrap D3D9 behind the D3D8 interface the engine expects (vertex-declaration token translation, sampler-state split, shader handle tables, BaseVertexIndex, ZBIAS->DEPTHBIAS). `d3dx8.h` reimplements the few D3DX math/texture helpers used. |
| Shaders | `tools/build_shaders.py` assembles the `.nvv`/`.nvp` sources with the Windows D3D assembler into the `CompiledVShader*.h`/`PShader*.h` headers the original build produced with the DX8 SDK. |
| Threading | `fdx8loop.cpp`: the game-loop thread was an MFC `CWinThread`; replaced with a small Win32 thread class with the same lifecycle. |
| Entry point | `port/main_win.cpp` replaces the MFC "mawin" dialog. Same boot sequence as the Xbox `main.cpp`. |
| Struct layout | Built with `_FANGDEF_WINGC` (GameCube alignment, no SSE) so structures read from GC data line up in memory. This mode was previously tools-only, so `fdx8gcmath_*.inl` gained 12 math functions the runtime needs (scalar code from the GC reference). |
| GameCube assets | `port/gcdata.cpp` converts big-endian CSV, particle, animation, world header, visibility, and world-init records, and decodes GX tiled TGA images. `port/gcmesh.cpp` expands supported GX display lists into D3D vertex and index buffers. |
| AI graphs | `AIGraph.cpp` converts retail GameCube `.gt` graph headers, vertices, edge slots, and POIs before pointerization; the GC build uses the five edge slots present in the retail format. |
| Language | C++ rule changes since 2003: anonymous-union members made implicit copies deleted (`CFSphere`, `CFRect2D`), implicit-int declarations, `Lock(void**)`, modern MASM operand sizes. |

## Status

- [x] Engine (Fang2), scripting VM, game logic and AI compile and link.
- [x] Executable starts; engine boots; D3D device/mode enumeration works on a real GPU.
- [x] Load the retail GameCube master file. The runtime swaps its big-endian header and
      directory, accepts the GC platform version, and warns about newer asset compiler versions.
- [x] Convert GameCube CSV tables to host byte order, including pointer offsets and UTF-16 strings.
- [x] Decode GameCube TGA textures from GX tiled formats, including CMPR and split S3TCx2,
      into linear ARGB pixels for the D3D texture path.
- [x] Convert static and skinned GameCube meshes, including streamed NBT3 display lists, to
      D3D vertex/index buffers. Convert embedded kDOP collision trees and their triangle data.
- [x] Adapt the retail version 8 particle layout to the source version 7 runtime layout;
      particle textures now load during startup.
- [x] Convert WLD headers, visibility trees, shape-init records, and world mesh tables.
      `-world-only we01multi01` loads 22 meshes, 22 portals, 22 volumes, 22 cells, and 101
      world-init shapes through the resource loader. Streamed GX display-list data is retained
      for mesh conversion.
- [x] Convert retail `.mtx` animation headers, bone records, key-time arrays, and compressed or
      floating-point tracks. Startup loads character animations during world entity creation.
- [x] Convert retail GameCube `.gt` AI graphs. The converter validates graph counts, array ranges,
      edge counts and edge targets before swapping the header, vertices, edges, and POIs. The
      full `we01multi01` level path converted a graph with 153 vertices and reached boot completion.
- [x] Convert GameCube `.fnt` fonts. `ftext.cpp`'s font loader (`ftext_Load()`) is a bespoke
      reader that never went through the generic `fresload`/`gcdata_Convert` hook and never
      byte-swapped anything; `gcdata_ConvertFont()` validates and swaps the header and its
      three variable-length arrays (letter buckets, bucket-letters, letters) in place before
      the loader's own offset-to-pointer fixup runs. This was crashing every launch (a wild
      pointer read while drawing the first piece of HUD text); see the CRT diagnostics entry
      below for how this was actually diagnosed instead of just hanging.
- [x] Fixed a real, pre-existing engine bug (not GC-specific) in `fresload.cpp`'s mesh-portion
      loader: its failure path called `fang_Free()` on memory that was actually allocated via
      `fmem_Alloc()`/`fres_AlignedAlloc()` (both `CFHeap`-backed, a completely different
      allocator with no `fang_Malloc`-style tracking header). `fang_Free()` read whatever bytes
      happened to precede the block as if they were that header and corrupted them trying to
      unlink it, crashing in `flinklist_Remove()`. This was presumably always latent - retail
      PASM output apparently never failed a mesh conversion, so this path was never exercised -
      until this port's own (still incomplete) GameCube mesh converter genuinely rejected an
      unsupported display list and hit it. Removed the redundant, wrong free (the
      `fres_ReleaseFrame()` immediately above it already reclaims the same memory, since its
      frame marker was captured before the allocation). This turned a hard crash into a clean
      "this mesh/level can't fully load yet" outcome on real single-player levels.
- [x] Render the single-player `wecdsneak01` scene with textures, Glitch, and the HUD after
      correcting D3D shader input declarations and affine bone/instance transforms.
- [ ] Extend mesh coverage beyond the currently supported data. Collision conversion failures
      still drop collision for that mesh; display lists with more than four bones use an
      approximation that needs review.
- [x] Load retail scripts (`.sma`). All 393 are compact little-endian AMX (file version 5,
      AMX version 4) that the bundled interpreter runs without byte swapping. The loader
      validates headers and symbol tables before allocating. Across the corpus, 10 natives
      are missing from this source (`Checkpoint_Save2`, `Audio_Play2DSoundEx`,
      `Bot_IsPosessed`, `Bot_IsRecruited`, `Bot_Recruit`, `Console_Enable`, `FX_StompRing`,
      `Misc_GetDifficulty`, `Misc_GetValue`, `Misc_SetValue`); none is used by Night Sneak.
      Scripts importing them still load, the missing names are logged, and a call to one
      aborts that script callback with `AMX_ERR_NOTFOUND` instead of calling a NULL pointer.
- [x] Implement the 10 missing natives from their retail code in main.dol. All 393 retail
      scripts now bind every native. `Bot_LoadTalk` accepts the retail 5-argument form.
- [ ] Verify mission objectives, level transitions and saves.
- [x] Bink cutscenes play full screen (4:3, stretched with linear filtering) with their own
      audio. The retail movies use the `GC_` prefix; `-no-audio` mutes these tracks for test runs.
- [x] Game audio: GameCube MusyX banks convert to PCM, and DSP-ADPCM music/speech streams use the
      retail GameCube volume chain. User confirmed effects, droid speech, and music in a mission;
      final mix and individual weapon/UI levels still need listening checks.
- [ ] Vehicle controls: the RAT in `WEWHchase01` does not respond to WASD for driving or the
      turret, and mouse motion appears to steer it (user report).
- [x] Keyboard controls and direct raw mouse look, confirmed interactively in `wecdsneak01`.
      XInput mapping includes deadzones, separate triggers, focus handling, and hotplug support.
- [ ] Verify physical XInput controllers, vehicle-specific mouse aiming, and rumble.
- [ ] Save games. The front end uses the Xbox flow with the PC's single "hard disk" device
      (`dx/fdx8storage.cpp`). That backend was rewritten: per-user save directory, safe file names,
      atomic writes, and `ValidateProfile` now reports a missing profile (it always said "valid",
      so `CPlayerProfile::IsOnCard()` could never fail). Syntax-checked only; creating, loading,
      renaming, deleting and in-game saving still need a run.
- [x] Engine back-buffer BMP capture through `-shots` / `-shot-every`. The original
      `port/screenshot_port.cpp` keyboard shortcut implementation is still a stub.

## Known issues / things worth a reviewer's eye

- `SetRenderState(D3DRS_ZBIAS)` is mapped to `D3DRS_DEPTHBIAS` with a guessed scale
  (`d3d8_compat.cpp`); decals/coplanar geometry may z-fight until tuned.
- `D3DXLoadSurfaceFromMemory` supports only same-format (and 32-bit interchange) copies.
- D3D8-only render states with no D3D9 equivalent are silently ignored.
- The launcher writes Fang's resource load log to `ma_port_asset_log.txt`; the `-log` option
  remains the engine/debug log.
- Retail data was built with newer tool versions than this source snapshot (e.g. mesh
  compiler 0x39 vs 0x37 in `fdata.h`); the runtime doesn't enforce these, but layouts
  may differ slightly.
- Retail `Difficulty.csv` has 20 fields in its `Diff` table. The PC build now reads all
  five fields for each of the four difficulty levels, so it no longer falls back to
  defaults. The three retail additions are retained in `CDifficulty::CInfo` but their
  gameplay uses have not yet been mapped. The retail flamer table also has newer tail
  columns, which are ignored by the older source vocabulary.
- Retail `w_laser.csv` (73 fields; a charge/burst redesign) is mapped onto the source's
  primary-fire laser from retail evidence: the `LaserL1` vocabulary in `main.dol` and the retail
  `CWeaponLaser` InitSystem/ClassHierarchyBuild/fire code. Mesh, muzzle bone, tracer texture
  and RGBA color, clip (reserve forced infinite, as retail does), fire rate, tracer
  speed/length/width/range, target-assist range, cull distance, recoil, damage profile and
  decal come from the retail fields. The recharge rate is inferred from the retail refill step
  (fields 39/40). Retail L3 fires tracers, so the WINGC build uses the tracer path instead of
  the source's continuous L3 beam. Not implemented: the charged burst (fields 19-28, 60-64,
  67-72), particle muzzle/smoke effects (47-58) and sound groups (65-70).
- Retail `afDataTable` (the float table vocabulary min/max indices point into) inserts `4.0`
  after `3.0`, so retail indices from 12 up are one higher than this source's
  `F32_DATATABLE_*`. Data files are unaffected; compare retail vocabularies with
  `tools/dol_vocab.py`, which uses the DOL's own table.
- Retail GameCube AI graphs have 5 edge slots per vertex. `InitEdgeToPoiLookup` skipped free
  vertices with a hard-coded 6, misaligning (and overrunning) the edge-to-POI table and
  asserting thousands of times during mission AI setup; it now uses the slot constant.
- Grunts carrying lasers previously failed to build, so mission scripts could not find them
  (`E_Find() : Could not find entity named 'added_grunt3c'`). Other retail damage profiles
  (`Debris`, `SentinelCannon`, `SpewSmoke`, `ScoutSiren`) are still missing from the loaded
  damage tables.
- The blaster
  loader now reads its 45-field layout and handles the three available player variants without
  initializing absent military variants. Its two extra numeric fields are retained but their
  behavior is not implemented. Blaster resource creation, firing and upgrades need runtime
  confirmation. Other item tables still report unsupported retail collectable names.
- Retail inventory startup slots now follow recognized items when unsupported names are skipped;
  array/count validation prevents invalid table indices and empty-inventory underflow.
- Retail data layouts recovered from main.dol and now read: goodies.csv (collectables; every
  pickup was unknown before), materials.csv (surface class field), deb_group.csv (debris; the
  random-orientation flag is gone and priority widened), sw_wall.csv (wall sentry; smoke fields
  not implemented). `fgamedata_GetTableDataRemapped` reads a source vocabulary from mapped retail
  fields. Retail barter EUK kits and battery upgrades have no source collectable types yet.
  Those unsupported shop entries are omitted so they cannot become unrelated quest-item displays.
- The retail particle keyframe's extra 8 bytes sit before `NumPerBurst`, not at the end.
  Removing the wrong bytes had corrupted counts, colors and timings in 375 of 377 effects and
  produced out-of-range bubble chances that asserted every frame (a frozen-looking mines level).
- Bone-mask index tables must end with 255. Five did not (rat gun x2, Glitch fire-1 lower,
  Predator left arm, Corrosive summer) and the reader ran into adjacent data; the rat gun
  crashed the chase level.
- The retail elite guard has seven dismemberable limbs (the source had only the head).
- The retail zombie boss level looks up only `ZombieBotBoss`; the cage/Mozer lose rule and the
  `mozer01` lookup are skipped in the GC build.
- Assert and `/RTC` reports are rate-limited (first 10, then 100, 1000, ...) with a stack on
  the first occurrence, and world entities that fail to build are logged by name.
- `CFQuatTang3::Calculate` now falls back to +Z when the path tangent has no usable XZ
  component. `WEDTtown_01` loads `xedt_carts.sma`, whose swinging spline actor passes the
  point-path tangent into this math; a vertical/degenerate tangent previously reached
  `Unitize()` as zero and produced NaNs. The fallback leaves acceleration unrotated and keeps
  the existing normalized path for valid XZ tangents. The town mission reached end-of-loading
  without asserts or crashes; cart motion still needs a visual check.
- Weapon selection rejects unavailable runtime weapon objects and retains the previous equipped
  item. Starting with Empty Secondary now preserves the inventory count so throwables remain
  selectable. These changes build successfully; runtime confirmation is pending.
- The wrapper system accepts trailing retail phrases and reads its six-field screen-name stride,
  selecting the Xbox UI entries and ignoring the two screens this source does not define.
- A regular `-level we01multi01` launch now passes wrapper and world setup and reaches
  `END OF BOOTUP`. Use `-mission` for registered campaign content; the generic debug-level path
  does not establish campaign audio/content behavior.
- Older multiplayer `we01multi01` logs showed a repeated startup cycle. The later skinned-mesh,
  collision, shader, and matrix fixes supersede the earlier blocker descriptions in the handoff.
  Use `wecdsneak01` as the current interactive baseline; multiplayer behavior needs a fresh check.
- Current captures still show some scenery surfaces as solid black. Rendering completeness
  remains a separate task from the now-working movement and mouse look.
- The animation adapter bounds-checks offsets, counts, and track ranges. It rejects animations
  with overlapping track ranges and files with more bones than the source runtime's 127-bone
  limit; those assets still need a format-specific review.
- The `.gt` converter matches the analyzed retail corpus: its GameCube graph header is 40 bytes,
  each vertex is 128 bytes with five edge slots, and each POI is 8 bytes. Recheck those layout
  assumptions if support is extended to other platforms or asset versions.
- `-world-only` deliberately skips world-shape entity creation. Its diagnostic exits successfully
  but prints debug allocator warnings for game-system objects during shutdown; it does not
  represent normal gameplay teardown.
- The particle adapter is based on the retail corpus layout: each version 8 keyframe has an
  additional 8-byte tail field, removed before the version 7 structures are byte-swapped.
  It validates that both removed fields are zero, plus the shifted texture name, sampling
  interval, and zeroed list state before converting a file.
- `main_win.cpp` now redirects every Debug-CRT diagnostic (asserts, `/RTC` stack/uninitialized-
  variable failures, pure-virtual calls, invalid-parameter aborts) to the log instead of a
  blocking "Microsoft Visual C++ Runtime Library" dialog. Without this, a hit anywhere in the
  Debug build looks exactly like a hang: the process is still alive, sitting in a modal message
  box nobody is there to click. This is how the font crash below was actually found, instead of
  just timing out. Non-fatal asserts now log and continue (matching "Ignore"); this can let a
  real bug run further than it would on the original platforms before something else notices -
  treat a firing assert as a real bug to fix, not background noise. The first occurrence of
  each distinct assert also logs a symbolized call stack, so a repeating assert names its caller.
- Found via the above: `CBotGlitch::_InitInventory()` was passing a retail inventory's current-
  weapon slot straight to `_ChangeWeaponIndex()` with no bounds check against this source's
  reduced weapon set (see the laser/blaster schema note above) - a real out-of-bounds array read
  that the original `FASSERT` alone did not prevent (`FASSERT` compiles out entirely in
  Release/Production builds, so this was always a live bug, not just a debug-build nuisance).
  Fixed at both the call site (fall back to slot 0 when the retail slot is out of range) and in
  `_ChangeWeaponIndex()` itself (bounds-checked no-op instead of undefined behavior).
- Script event masks are `u64`, but events were set and tested with a 32-bit `1 << n`. For events
  32-63 the GameCube's PowerPC gives 0 (never delivered) while x86 wraps the count (event 40 fired as
  event 8; event 63 matched events 31-63). `fevent_Bit()` (`FEventListener.h`) now gives the GameCube
  result everywhere (`FScriptSystem.cpp`, `FEventListener.h`, `GeneralCorrosiveGame.cpp`,
  `SpaceDock.cpp`, `ColiseumMiniGame.cpp`). The remaining C4334 warnings (`fcoll_kDOP.cpp`) shift by a
  kDOP vertex index, at most 23, so they are harmless.
- 32-bit only: 150+ inline-asm blocks, MASM collision code, x86 Bink import lib.

## Legal

The source and the game data are proprietary. This repository is kept private and does
not contain any retail game data (`gamedata/` and disc images are git-ignored).

### Co-op menu prototype (2026-09-27)

The PC main menu has an experimental Co-op entry with Players (2-4) and Controls.
Controllers is the default: pads 1-4 control P1-P4, with keyboard optional for P1.
Keyboard + controllers reserves P1 for keyboard/mouse and maps pads to P2-P4.
The CLI also accepts `-input-layout shared` or `separate` with `-coop`.
The prototype starts a new campaign with virtual profiles and no progress saves.
Network play is unavailable. Release and Debug builds succeeded; two-player menu
launch rendered split screen. Physical multi-pad gameplay and four-player menu
launch remain unverified. See [the network direction](docs/coop-network-plan.md).

### Desktop main-menu exit

The PC main menu includes Quit to Desktop, using the game's normal shutdown path.
The four menu entries fit below the original logo with mouse hit targets and
keyboard/controller navigation. Release and Debug builds succeeded; a muted
scripted-key run selected Quit and exited with code 0 before its test timeout.

The generated desktop labels use the game's angular display font with italic
lettering, blue edging and layered gold shading; selection reuses the original
green highlight mesh. `pc_menu_style` visually confirmed this and a clean Quit.
Automated co-op routing checks cover four pad slots, separate keyboard routing,
and restoring the configured layout; physical multi-pad play remains pending.

Keyboard/controller navigation now clears stale mouse actions cached earlier in
the same menu frame, preventing those actions from overwriting the new selection.
The direct Campaign-to-profile path and save-location label spacing were visually
confirmed in `pc_profile_direct`; simultaneous physical input remains untested.

An unavailable save folder falls back to the location screen. Its no-save choice
is labeled Play Without Saving and explains the problem without fake zero-space
statistics. Release/Debug builds and an isolated Debug failure-path capture passed;
checkpoint write failures remain a separate verification item.

Dialog prompt polish: generated keyboard and PlayStation prompts now align to their
action labels; dialog text uses the fixed PC text coordinate range at widescreen
resolutions. Verified Accept with PlayStation prompts at 1920x1080 and keyboard
prompts at 1280x960. Physical controller and other dialog buttons remain pending.

On PC, the pause Information page sizes item descriptions and mission objectives
to their panels. Verified the Washer and Mines text at 1920x1080 and 1280x960;
longer descriptions and the weapon pages remain to be checked.

The active pause layout has Options and Information pages. PC item counts now
scale with their slots; 1920x1080 and 1280x960 captures show them readable.
