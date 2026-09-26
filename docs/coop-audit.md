# Co-op audit: single-player assumptions

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
