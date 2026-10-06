# Co-op audit: single-player assumptions

## Cinematic views and join panels (2026-10-03)

Partners now borrow the active cinematic view inside their existing split rectangles, retaining
their own camera controllers for the return to play. Co-op join panels use screen fractions and
thin pixel borders instead of width-scaled retail meshes. See [the report](coop-cinema-ui-20261003.md)
for 851 offline camera/layout checks and remaining visual confirmation.

## Campaign playthrough (2026-10-03)

See [the playthrough report](coop-playthrough-20261003.md) for the communications-centre player
damage fallback, shared transmissions/vendor history, vendor checkpoint recovery, initial-spawn
validation, RAT self-hit filtering, authored ending movies and liquid/rendering fixes. The report
separates offline checks from pending user gameplay verification and unresolved hum/music/wires.

## Current verification (2026-10-02)

Nuts of Steel does not suppress the observed Clean Up checkpoint request: compiled `townzone.sma`
calls Checkpoint_Save before the logged Next Zone 3 message. Safe-placement tracker filtering
admitted Glitch's mesh because Fang accepts any matching type bit, including subclass bits left
in `~ENTITY_BIT_BOT`. An explicit callback now excludes bots and their owned weapons, plus the
vehicle-only props/detpacks ignored by ordinary movement. Floor/hazard/void/wall checks remain.
The new fixture reproduces the old safe-ground failure and verifies pending save completion and
wipe priority; one-shot save/defer/completion logging distinguishes blocked saves from no request.

PvP/co-op join handoff now explicitly initializes independent AUTO input routing for either
local mode. Leaving co-op restored SHARED routing, but PvP had not overridden it, merging the
first controller into keyboard P1. PvP entry now clears campaign flags and deals controllers to
separate join slots; forward keeps mapping, Back resets it. 145 offline production menu/input
checks pass, including switch cycles and hotplug; gameplay confirmation remains pending.

Hold Your Ground (WEWChold_01) now has an experimental PC co-op path: one AA gun per player,
created before checkpoint 0, P1's retail walk/jump intro, and automatic seating for P2-P4 after
letterbox ends. Each gun has separate controls/reticle/camera with original aim limits and mortar
support. Waves wait for every seat and camera, breach disables every gun, and the shared restart
reseats the team. Extra guns use provisional offsets +16/-16/+32 along the original gun's right
axis; real platform fit, terrain support, sightlines and mission completion remain playtest checks.
No extra foundations or difficulty scaling were added. The production-helper fixture passes 189
checks; Debug/Release builds and previous co-op fixtures pass. No game launched.

The friendly scripted racing RAT gunner is reserved for a co-op human. Its NPC object is retained
for references but removed from world/auto-work. P2 normally starts in the existing turret beside
P1 after scene/body readiness, with separate gun controls/camera. Initial boarding uses existing
checkpointed RAT flags, manual first entry consumes the automatic offer, and exit stays voluntary.
Second human boarding preserves vehicle health and human driver ownership. Shared scripted timers
now appear on all co-op HUDs through show/hide/restore and vehicle mode changes. The new offline
checkpoint/RAT/timer fixture passes 178 checks; gate (400), scene (434), Debug and Release pass.
Normal Release was updated after confirming the game closed, with hashes verified. No game launched.
RAT gameplay/restart and Clean Up revival remain user playthrough checks. There
are still only two RAT seats: P3/P4 vehicles and other single-seat vehicle missions remain open
participation issues. Hold Your Ground has the experimental separate-gun path above.

The latest Seal the Mines cave screenshot confirms the fall still shifts toward P2's camera at
close range despite the earlier liquid-mesh fixes. The active world is `WEDMmines03`; its init
tables have splash/top particle emitters and liquid volumes but no LiquidMesh entities. Windows
emulated point sprites left the graphics view at identity after drawing view-space quads; nearby
emitters can select this path, and sorted transparent world meshes then project with the wrong
camera. The Windows path now restores the active/mirrored view, matching its Xbox counterpart.
World-space setup also refreshes the view before computing shader matrices and clears the fixed
world transform. `tools/test_particle_camera.py` reproduced the original failure and now passes
2,592 offline checks across P1-P4, near/far selection, projected world anchors, mirrored views,
wrapped sprite buffers, skipped and empty draws. Debug/Release builds and existing waterfall/scene
fixtures pass. Normal Release and pending-update are synchronized after confirming the game closed.
No game launched; the user subsequently confirmed the nearby waterfall issue was fixed.

PC weapon/grenade selection now uses separate per-player tap/hold gestures: single taps retain the
retail callbacks, double taps within 0.3 seconds cycle the requested hand, and only a fresh 0.3-second
hold opens its selector. HUD closing states cannot reopen or hand off from raw held buttons; released
movement cannot keep queuing scrolls. Pause/barter/scenes, focus loss, body/controller changes, death
and restore/load clear pending gestures. Quick cycling skips empty/unavailable items and uses normal
inventory equip callbacks. `tools/test_weapon_select_menu.py` passes 6,147 offline checks across
P1-P4 and 40 wrapper-frame atlas lifetimes. The latest log shows pause Quit successfully returning
to the main menu, then crashing on the static label texture's freed resource pointer. Wrapper reset
now clears that pointer and the attempted-load flag before releasing resources. Debug/Release builds
and adapter/scene checks pass. Normal Release and pending-update are synchronized, including the
waterfall changes below. No game was launched; actual gameplay/menu verification remains pending.

The newest waterfall report (brief camera-center graphic / disappearing head-on) exposed invalid
fractional-power curvature calculations and a 44-byte four-UV declaration for 36-byte three-UV
waterfall vertices. The curve now uses positive unit distance; reflective and molten waterfall
layouts match the actual vertices. Draws explicitly depth-test with LESSEQUAL and disable inherited
alpha tests. Shared waterfall animation/scroll advances once per frame, preserving the same shape
across P1-P4. `tools/test_waterfall_render.py` passes 6,901 offline checks with 544 complete draw passes;
Debug/staged Release builds passed. Visual confirmation remains pending. That active normal Release
session/executable was left untouched; the later selection/menu build above includes these changes.

The latest user screenshots confirm the large moving square persisted after the previous liquid
patch. Liquid planes/waterfalls now reset the fixed-function world transform as well as shader
constants; FVF material passes previously retained the preceding droid's transform. No collision
code was changed. The square's visual recurrence and reported physical effect remain manual checks.

Door/lift checkpoint saves now include the mesh base state (animation clocks, speed, pause flags
and selected mesh). Restore rebuilds line, translated-bone and animated poses, including moving
snapshots and unchanged endpoint states, preserving pickup/open timers and resuming the appropriate
movement loop. Checkpoint request and lift restore details are logged. The reported failed elevator
return is not proven resolved: the remembered mission/restart method is uncertain and the available
Seal the Mines log only shows the trigger releasing, without lift state diagnostics.

Cutscene spectators now fall under neutral controls until actual grounded contact before holding
stationary. Floor correction in Y and sticky-platform/parent movement are retained. Waiting text
has a per-player 0.35-second delay, cleared on leaving/release/cutscene/load/restore; actual event
gating is unchanged. Startup and unfocused window cursors explicitly select the Windows arrow.
`tools/test_coop_scene_restore.py` passes 434 offline production-method checks for scene grounding,
platform displacement, lift snapshot state/pose/timers/direction and liquid transform setup. Both
build configurations pass; normal Release and its pending copy are updated. No game was launched.

The PC pause Cheats submenu targets the pausing player, with per-player ammo/protection masks,
individual washer grants and weapon grants/upgrades through that player's inventory. Possession
keeps the washer grant in the permanent wallet; heal addresses the current live body and refuses
unsafe resurrection. Toggles reset at the main menu and are not serialized, while granted items
use existing saves. Available-weapon grants reject absent prepared pickup assets before indexing
the retail pool and restore the global pickup recipient context afterward. The production-method
fixture `tools/test_pc_cheats.py` passes 413 offline checks across P1-P4; runtime UI/assets remain
unverified because the user requested no game launches.

Checkpoint enters in the audited retail naming families now run for the first living original
player, without a team wait. The normal script still owns saving, objective requirements and safe
revival. Other gates retain sticky team arrivals, with per-frame movement rechecks for missed
enters; PC box triggers now detect complete crossings between frames. An assignment in tripwire
occupant removal also removed the wrong actor; it now preserves the remaining players. The offline
production-method fixture (`tools/test_coop_gate_recovery.py`) passes 400 checks across 2-4 players.
Custom checkpoint names remain a playthrough check. The true Mines 2 wait at `bradys_attack` /
`trigger_btr01` needs manual verification; the earlier HUD-only fix did not release logical gates.

The user authorized updating normal Release after closing their run; the earlier patches and these
recovery changes are applied there. No game was launched. Liquid mesh drawing also now sets its
own world matrix and molten vertex layout rather than inheriting a preceding bot's draw state.
This is a candidate fix for the large square moving among droids in Mines 2; its visual and reported
physical behavior have not been reproduced or verified after the change.

The user confirmed the Mines 1 zipline waiting gate works in play. A separate P2 HUD report came
from an unreleased `goagain2` side-route arrival even after both players passed `goagain` and later
checkpoints. Waiting text now describes a live unresolved gate containing the player, with the
parked-terminal exception. Arrival history remains sticky for thin gates; this HUD change has not
been verified in gameplay. Odd ally movement around successive bot pairs remains an unconfirmed
AI report.

The latest user session exposed two progression issues. Mines 1's terminal arrival record survived
a checkpoint rollback; restores now clear all held arrivals, and common terminal exits park early
arrivals before they can move into floorless geometry. Mines 2's `xedm_end` attempted to face/move
dead P1 while P2 finished, so the world ran but the script waited indefinitely. Story player lookup
now prefers living P1, otherwise a living partner, and retains that actor through the entire scene.
AI movement, fall actions and spectator cameras agree on that player. Scripted actors cross scene
triggers without waiting for disabled spectators. Living spectators animate under neutral controls
after landing while retaining platform displacement; downed viewers see the scene rather than their distant corpse.
The user has now confirmed a cutscene advanced with P1 dead and P2 temporarily acting as Glitch.
That reported fallback scenario is verified in gameplay; other campaign scenes remain playthrough checks.

The fresh-profile Reset warning was a difficulty-screen asset mapping mismatch, not a save deletion.
The retail front-end screen mapping is corrected; source-order and pause tables are unaffected.
Current profiles passed CRC validation and were backed up. The old `Profile1&&&` test file could
not be found, and the user accepts possible accidental deletion. Debug/Release builds, eight offline
actor-selection checks and all 35 front-end/35 pause asset mappings pass. Further progression and
screen verification is pending. Do not launch the game until the user authorizes it.
Scripts retaining a player handle from level initialization and custom terminal trigger names
remain playthrough checks; the confirmed Mines 2 script refreshes its handle immediately before
starting the scene.

The user initially confirmed co-op saving/reloading, ordinary deaths and checkpoint revives with the vanilla
animation, and partner cameras following player 1 during player 1's cutscenes without letterbox
bars. Alt-tab and using other applications caused no observed issues; play is smooth, with no
specific audio issues reported. Controller hot-swapping and a full campaign progression playthrough
remain to be tested. See `HANDOFF.md` for the current implementation and remaining liquid checks.

A later Mines 1 playthrough exposed an airborne-checkpoint softlock: `save03` fired during P1's
jump, and P2 revived at their below-route death location because no grounded partner was available.
The revival code now waits for checked ground/body clearance before resurrecting; the checkpoint
save stays pending until placement succeeds. Dead/dying bodies no longer enter timed fall recovery,
and a team restore overrides the pending save. Release and Debug compile; gameplay verification
of this fix is pending. The new `revive` fixture is compiled but unrun. The user explicitly asked
for no further game launches while they are running other things; wait for their instruction to resume.

The user also confirmed water in Seal the Deal and both players completing the Seal the Mines
borrowed-bot objective. Possessed actors and named entity filters bypass the shared progression
gathering gate so scripts receive the correct bot. The normal regression uses the console and waits
for the cutscene/body transition and possession handoff before entering the objective.

Shops now track their activating player for input, camera, UI, purchases and spending, with one
shopper at a time. Washer balances were already per inventory; pickup and checkpoint tests confirm
their separation. World weapons retain a per-player claim mask, hide only in the collector's
viewport, and persist for partners. Checkpoints restore partial claims; scripted and paid grants
remain exclusive to their recipient. Unsupported legacy retail upgrade kits remain unavailable.
Discord campaign activity includes `CO-OP` alongside mission details. The intermittent blue circle,
controller hot-swapping and full mission progression remain manual checks.

Run `python tools/test_coop_polish.py` after building, or add `--config Debug`. Four-player weapon
ownership checks use `--players 4 --cases pickups weapons`. Fixtures are opt-in and use isolated
saves. Reports are written under `build/logs`; no test fixture runs during ordinary gameplay.

The sections below retain the original 2026-09-27 source audit and its historical status; the save,
death, menu and cutscene limitations described there are superseded by the current handoff.

## Original source audit (2026-09-27)

Status (2026-09-27): `-mission WORLD -coop 2..4` (PC) starts a campaign level with 2-4 local players
(`launcher.cpp` builds the `GameInitInfo_t`: `bSinglePlayer=TRUE`, `nNumPlayers=N`, no profiles; the
input layout uses normal configuration: `shared` by default, pads 1-4 for players
1-4 with keyboard optional for player 1; `separate` reserves keyboard/mouse P1). Verified by runs: the
split screen, HUDs and radars draw per player; no asserts or crashes in a town run. Done from the list
below:

- Start points (4): with a campaign level's single start point, players 2-4 start 3 units beside or
  4 behind player 1 (`CStartPtMgr::InitLevel`), instead of the multiplayer hack's 10 units, which put
  player 2 inside a wall or pipe.
- Death and checkpoints (2): a player who dies (or falls out of the world, or is stuck in the air)
  while a partner is standing comes back beside that partner with their inventory
  (`_CoopRespawnNearPartner` in `player.cpp`); the level-wide checkpoint restore runs only when nobody is
  standing. Not yet exercised in a run (needs a player death).
- Scripts' player (1): `Bot_GetPlayer`, freeze/unfreeze, `BotGlitch_FallDown` and the camera-animation
  cutscenes use player 1 (`_ScriptPlayer()` in `MAScriptTypes.cpp`); cutscenes disable and re-enable
  control for every player (`_ScriptSetPlayersControl`).
- Pause (5) was already per player: whoever presses Start pauses, with their own inventory.

Menu follow-up: a PC Co-op entry now offers 2-4 players and either input layout with virtual
profiles. The two-player controller-layout menu launch rendered split screen;
physical multi-controller gameplay remains unverified. See `coop-network-plan.md`. CLI co-op now
uses the configured input layout (`shared` by default), with `separate` available for keyboard P1.

Still open: no progress saving (3), barter (6),
collectables (7), AI targeting (8), minigames and bosses (9); other players' cameras keep their own view
during scripted cutscenes (only player 1's view shows the cutscene camera).

Goal: campaign levels with 2-4 local players. This lists what already works per player, and every
place found that assumes a single player, with a suggested change. It was written from source only
(no runs). Line numbers are for `x86-port` at the time of writing; search for the quoted code if they
drift.

## Already per-player (the split-screen multiplayer path)

The engine and game already run up to `MAX_PLAYERS` (4) local players for multiplayer:

- `CPlayer::m_nPlayerCount`, `Player_aPlayer[]`: a bot, controller index, profile, HUD, reticle,
  viewports and camera per player (`game.cpp` `_PostWorldLoadGameInit`, the player loop).
- `splitscreen_SetupViewports()`, `PLAYER_CAM(n)`, per-player `CHud2`/`CReticle`.
- Audio: `faudio_SetActiveListenerCount( CPlayer::m_nPlayerCount )`; the PC backend keeps several
  virtual listeners (`dx/fdx8audio.cpp`).
- Inventory: `CPlayer::GetInventory( nPlayer )`.
- Input (port): `-input-layout separate` gives keyboard/mouse player 1 and pads players 2-4
  (`port/pc_input.cpp`); XInput hotplug.
- Respawn: `MultiplayerMgr.RespawnBot()` for multiplayer deaths.

So co-op is mostly about **campaign rules and content assuming one player**, not rendering or input.

## How a co-op level would be started

`game_LoadLevel( title, bShowLoadingScreen, pGameInit )` with `pGameInit->bSinglePlayer = TRUE`
(campaign rules) and `pGameInit->nNumPlayers > 1`. `CPlayer::InitLevel( pGameInit, ... )` then sets
the player count. Two things decide how much of the game follows campaign rules:
`MultiplayerMgr.IsSinglePlayer()` (set from the game init) and `CPlayer::m_nPlayerCount == 1` checks.
Co-op needs `IsSinglePlayer()` TRUE, so every check below that means "exactly one player" rather
than "campaign rules" must be looked at.

## Must change

### 1. Scripts' "the player" (`MAScriptTypes.cpp`)

- `Bot_GetPlayer()` (line ~1753) returns `CPlayer::m_pCurrent->m_pEntityCurrent`. `m_pCurrent` is set
  as each player is updated/drawn, so with 2+ players a script callback gets whichever player was
  processed last. This is the scripting API's way to get the player's bot (the retail scripts are
  compiled AMX and not in the repo, so how often missions call it is not checked here).
  `BotGlitch_FallDown` (~3245) also uses `CPlayer::m_nCurrent`.
  **Suggest:** in co-op, return player 0 (the host / "story" Glitch) deterministically; consider a
  new native for "nearest player to X" only if scripts need it.
- Cutscene begin/end natives (lines ~3745-3793) stop head-look and deactivate/reconfigure the AI
  brain of player 0 only. Other players stay controllable during cutscenes.
  **Suggest:** loop over `CPlayer::m_nPlayerCount`.
- Glitch-specific natives (lines ~6260-6302: goodies, inventory) act on player 0's Glitch. Retail
  scripts give items "to Glitch"; decide per native (give to all players, or to player 0).
- Line ~1646: a `m_nPlayerCount == 1` check on the secondary-fire button (skip-style input); review.

### 2. Death and checkpoints (`player.cpp` ~812-830)

In single player, a player death restores checkpoint 1 (or 0) for **the whole level**, because
`MultiplayerMgr.RespawnBot()` returns FALSE outside multiplayer. With two players, one death would
roll both back. Checkpoints (`FCheckPoint`, `gamesave.cpp` `checkpoint_*`) save the whole world
state, not per player.
**Suggest a policy:** respawn the dead player next to a living partner (reuse the multiplayer
respawn path with a partner-relative start matrix), and restore the checkpoint only when all players
are dead. `PauseScreen.cpp` ~1190 ("restart from checkpoint") stays level-wide.

### 3. Progress saving (`player.cpp` ~404, `wpr_levelcomplete.cpp`)

`CPlayer::UninitLevel()` updates profile stats (level completion, next-level inventory) only when
`m_nPlayerCount == 1`, so a co-op completion would save nothing. `wpr_levelcomplete.cpp`
(~173-182, ~649) shows player 0's stats and uses player 0's profile and controller.
**Suggest:** save progress to player 0's profile (the campaign owner); decide whether other players
get a profile (the front end supports per-player profiles for multiplayer) or a virtual one.
**Done (2026-09-30):** a co-op save per player 1's profile in the save root's `Co-op` folder, with a
record per partner found by profile name; every player's progress is recorded (see HANDOFF.md,
"Co-op saves").

### 4. Start points (`game.cpp` `_CreatePlayerBot`, ~1530)

Player bots are placed at `MultiplayerMgr.NextStartPoint()`. Campaign levels have one start point,
so all players would spawn inside each other. **Suggest:** offset players 2-4 around player 0's
start (checking collision), or add start points per level.

### 5. Pause screen (`game.cpp` ~938, ~1015)

`CPauseScreen::Work/Draw` use player 0's inventory, and pause input is read from
`Player_aPlayer[0].m_nControllerIndex` (~908). **Suggest:** track which player paused and show that
player's inventory.

### 6. Barter droids (`BarterSystem.cpp` ~996-1025)

The barter UI draws into player 0's viewports. Who receives purchases was not traced. **Suggest:**
the player who activated the barter droid owns the session (store its index when the interaction
starts) and gets the view and the goods.

### 7. Collectables (`collectable.cpp` ~1453, ~1591, ~3013)

EUK (weapon upgrade kit) display rules check whether **player 0's** Glitch owns a weapon, to decide
whether a pickup shows as the weapon or as an EUK. The pickup functions themselves take the
collecting bot (e.g. ~1656), so credit already goes to whoever touched it. **Suggest:** "any player lacks it -> show the weapon", or per-viewport meshes (hard);
the first is simple.

### 8. Enemies and AI targeting

- `botswarmer.cpp` ~220: swarmer flock sound range and behaviour use player 0's position (asserts
  it is a bot). **Suggest:** nearest player.
- `botgrunt.cpp` ~2192 (`Die()`): weapon drop decisions look at player 0's inventory. **Suggest:**
  the killer if it is a player, else player 0.
- `Ai/AIBrainman.cpp` ~1521, ~1533: keeps **player 0's** brain working while its bot is out of the
  world. Other players' brains would be skipped in that case. **Suggest:** any player's.
  (~556 is debug drawing only.)
- `Damage.cpp`, `Ai/AIThoughtsGeneric.cpp`, `Ai/AIBuilder.cpp`, `BotDispenser.cpp`, `bot.cpp`,
  `botglitch.cpp`, `site_*`, `vehicleloader.cpp`, `weapon_recruiter.cpp`, `botpart.cpp`,
  `botcorrosive.cpp` branch on `MultiplayerMgr.IsSinglePlayer()` (37 uses in 21 files). Most mean
  "campaign rules" (friendly fire, recruiting, AI aggression) and should stay; each should be read
  to confirm it doesn't also mean "one player".

### 9. Minigames and boss levels

`ColiseumMiniGame.cpp` (25 uses of player 0), `SpyVsSpy.cpp` (9), `swarmerbossgame.cpp`,
`zombiebossGame.cpp`, `MG_HoldYourGround.cpp`, `SpaceDock.cpp` and `Ai/AIZombieBoss.cpp` are written
for one Glitch (win/lose rules, restores, cameras). **Suggest:** disable co-op for these levels at
first (fall back to player 0 only), then adapt one at a time.

## Single-player-only features (degrade, don't break)

These already have a multi-player branch (the split-screen one), so co-op gets reduced behaviour:

- `weapon_scope.cpp` ~436/528 and `reticle.cpp` ~1807: the full-screen scope render target is only
  used with one player.
- `Hud2.cpp` ~3372: "pause to switch weapons" only with one player.
- `game.cpp` ~1457: AI/entity debug drawing only with one player (debug).

## Port layer

- One keyboard/mouse player: `pcinput_KeyboardPort()` is port 0 (`port/pc_input.cpp`). Mouse look,
  mouse aiming and aim-assist suspension follow that port. Co-op with one keyboard player plus pads
  works with `-input-layout separate`; a second keyboard/mouse player would need raw-input device
  separation (not planned).
- `gamepad.cpp` gates mouse look on the keyboard port's control map (`pcinput_BeginFrame( ... )`).
- Saves: `CPlayerProfile` is per player; the PC storage backend has one device (see
  `CLOUD_SESSION_LOG.md` entry 1), which is fine for several profiles.
- Debug: `gamecam_InitLevelCameras( 0, ... )` makes player 0 the debuggable player; `entity.cpp`
  ~5574/5658 debug toggles use player 0, and `botglitch.cpp` ~1288 (an Xbox-only debug headlight
  toggle) only runs with one player. None of these matter for release builds.

## Suggested order

1. A launch option for co-op (`-coop N` building a `GameInitInfo_t` with `bSinglePlayer` and
   `nNumPlayers`), plus start-point offsets (4). This gets 2 Glitches into a level.
2. Scripts' player (1) and death/checkpoint policy (2): without these, missions misbehave.
3. Progress saving (3), pause (5), barter (6).
4. AI/enemy targeting (8) and collectables (7).
5. Minigames/bosses (9), level by level.
