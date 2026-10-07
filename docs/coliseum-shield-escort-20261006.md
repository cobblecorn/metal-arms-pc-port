# Coliseum escort blocked by Grunt shields (2026-10-06)

**Symptom (user, co-op):** in Coliseum 1, 2 and 4 (levels 33, 34, 36; `webccolis01/02/04`) nobody could
step far enough out of the cell to start the escort; destroying the guards' shield bubbles with cheat
weapons let it progress.

**Cause:** the escort guards are Grunts with the retail `Shield=On` property (parsed since `ab8e7d5`),
so they carry a `CEShield` bubble. `CEShield` pushes any Glitch, Grunt or Miner inside it outward every
frame (`_PushOutBots`, made for the Titan) and its mesh is solid to bot movement. Several shielded
guards at the cell door pushed each other and the players apart.

**Fix (Coliseum levels only):** `CBotGrunt::ClassHierarchyBuild` turns off the shield's push-out when
the loaded level is `LEVEL_COLISEUM_1`-`4`; `CBot::NewTrackerCollisionCallback` lets bots and players
through a Grunt shield whose push-out is off (`CEShield::IsPushOutBotsEnabled`). The bubble still draws
and absorbs shots. Other levels keep the retail push and solid bubble (user's choice).

**Verified:** user walked out of the gate area cleanly in two-player co-op Coliseum 1 (`-mission
webccolis01 -coop 2`). Coliseum 2 and 4 use the same guards and code path; not separately played.

## Per-player weapon choice (2026-10-06)

Retail strips player 1 to unarmed on entry (`LoadLevelN`, `LEVEL_EVENT_PRE_PLAYER_BOT_INIT`), lets them take
one pedestal weapon (`WeaponSelect`) and restores their inventory at the end (`CPlayer::SetSaveInventory`).
In local co-op every partner is stripped and restored the same way (`_PortStripPartners` /
`_PortRestorePartners`, `CPlayer::PortSetSaveInventory`); the door opens once every standing player has
picked; each player's weapon is filled; the arena ammo spots alternate between the chosen weapons. The
Coliseum 1 friendly droids' random pedestal pick gives up after 256 tries instead of looping forever
when players took the pedestals. Built; weapon pick seen in the user's Coliseum 2 run (ripper / rocket).

## Partners left behind the tunnel door (2026-10-06)

The battle scripts (`xebcbttle01-04`) walk the story Glitch onto the field during the arena intro
cutscene (`Bot_GotoE`, no snap) and close the door behind it. `CColiseumMiniGame::Work` now places the
partners beside the story Glitch (`CPlayer::CoopPlaceStartingPartners( TRUE, story )`) when the first
cutscene after the weapon room ends; once per level. User confirmed in two-player Coliseum 2.

## Co-op crash at the end of Coliseum 2 (2026-10-06)

`*** CRASH ... writing address 0xCDCDCDCD` in `CRoboBuddy::Destroy` <- `CBotGlitch::~CBotGlitch` <-
`game_UnloadLevel`, after any co-op Coliseum 2 match. Pre-existing (reproduced with every co-op change of
mine disabled; solo was fine). `InitGame` built `m_pFriendly[]` from every friendly bot except player 1, so
partner Glitches were counted as droid buddies: they got buddy AI settings and, in Coliseum 2
(`m_nLevelID == 1`), `WeaponSelect` cast them to `CBotMiner` for `SetStoredWeapon` / `SetArmorModifier`,
writing into the Glitch. Partner Glitches are now left out of the list. Bisected with temporary test
switches (auto weapon pick plus `-test-win-level 40`, all removed); the final both-players run reached the
results screen with no crash (`build/logs/verify_friendlyfix.log`).

## Coliseum 3 fight start: fixed elsewhere

The stall (gate `outerdoor6` closing before the battle script's open check) was diagnosed and fixed in
`docs/coliseum3-start-diagnosis-20261006.md`: the map omits `behavetype=other` on `outerdoor6` and
`outerdoor3`, so `Door.cpp` restores OTHER behavior for those two gates. The temporary script and door
traces used here were not merged.

Still open: Coliseum 3 lava hurts only mid-jump (likely the damage plane's height).
