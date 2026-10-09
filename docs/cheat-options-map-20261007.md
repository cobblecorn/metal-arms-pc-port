# Proposed cheat options — source investigation, 2026-10-07

Design only. No cheat actions, controls, game code, EXE, or saves were changed for this investigation. Existing working-tree changes belong to earlier work and the other chat.

## Menu and scope

The current menu already identifies its player (`CPauseScreen::m_nPlayer`) and calls `pccheats_Apply(player, action)`. The campaign check `MultiplayerMgr.IsSinglePlayer()` includes local co-op; competitive multiplayer is excluded. Add actions to `PcCheatAction_e`, their handlers in `port/pc_cheats.cpp`, and labels in `CPauseScreen::RefreshCheats`.

The existing menu has seven actions plus Back, with fixed vertical spacing. Adding all five options directly would extend it past its present layout. Proposed layout: existing Cheats page with an **Items** page and a **Movement / Camera** page. Each action still targets the player who opened the menu.

| Option | Proposed behavior | Implementation size |
| --- | --- | --- |
| Give 1 Chip | Add one normal control chip to this Glitch's inventory and show the resulting count. | Small |
| Bring DET Pack | Bring one available DET pickup beside this player; offer a grant fallback only where the mission supports DET planting. | Small to moderate |
| Flight: On / Off | Move Glitch in three dimensions while retaining walls and floor collision. | Moderate |
| Noclip: On / Off | Move Glitch through geometry, with a safe return when disabled. | Moderate; needs transition handling |
| Free Camera: On / Off | Move only this player's view; keep their body in place and leave partners' views active. | Moderate; reusable camera math exists |

## Give 1 Chip

Use `CPlayer::GetInventory(player)` / `INVPOS_CHIP`, following the existing washer grant. Verify the item exists and clamp the signed 16-bit count. Prefer `CItemInst::NotifyAmmoMightHaveChanged` so the display follows the new count. Require a living, controlled original Glitch for the first version, matching the existing weapon grant restriction.

This is the **normal Chip**, not Secret Chip. The console consumes the normal count through its existing insertion routine (`econsole.cpp`), so the cheat does not need to bypass a console, force possession, or alter a door. `CCollectable::GiveToPlayer(bot, COLLECTABLE_CHIP)` also exists if a physical pickup is desired, but that route requires a loaded mesh pool and successful pickup on a later frame. Direct inventory credit is the dependable interpretation of Give Chip.

## Bring DET Pack

Two different objects exist:

- `COLLECTABLE_DET_PACK` is the carried item, stored in `INVPOS_DETPACK`.
- `CEDetPackDrop` is the authored planting/countdown site. Its `ActionNearby` checks carried inventory, and the normal planting sequence consumes a pack and emits the mission's `detpack` events.

Proposed action sequence:

1. Require a living original Glitch and a loaded mission that has an authored DET planting site. Inspect actual entities/state rather than maintaining a guessed mission-number list. Loaded but currently inactive sites do not become active because of the cheat.
2. Find the nearest existing, uncollected DET **pickup**. Move it to a safe point beside the requesting player, stop its old motion, refresh its tracker/bobbing origin, and reserve pickup for that player. A small helper inside `CCollectable` is appropriate because motion, recipient, and state fields are private. Keep the actual object's identity and normal pickup notifications.
3. If no pickup remains, allow an explicit +1 grant or a recipient-scoped new pickup only after checking mission compatibility and loaded resources. `GiveToPlayer` already supports recipient-specific spawned pickups. Report unavailable when a physical spawn's resource pool is absent; never report success merely because a spawn was requested.

Do **not** move the planting site/wall marker, clone a countdown, restore an exploded site, or activate a bomb from the cheat menu. The player still plants the pack through the existing interaction. Suggested messages: `DET pack brought to you.`, `DET pack added.`, or `No DET-pack objective is available in this mission.`

## Flight and noclip

There are useful primitives, but neither is already a complete cheat:

- `CBot::SetBotFlag_NoGravity` exists, but `HandleAirTranslation` returns early when it is set. It freezes normal air handling; setting it alone does not produce controllable flight.
- `CBot::HandleCollision` skips collision when the player's world-mesh collision flag is off. This supports noclip, but also affects grounded state and interaction discovery. Turning it off is not a complete movement implementation.
- `CPlayer::Work` recovers players outside world volumes and also monitors unsupported airborne states. Flight/noclip need a narrow per-player exemption or bounds restriction so exploration does not unexpectedly restore the checkpoint. Keep that exemption tied to an active cheat.

Recommended shared movement controller: ordinary horizontal movement and look controls, separate ascend/descend inputs, adjustable speed, and frame-time-based displacement. Flight retains collision; noclip bypasses it. Do not use the existing Glitch “fly” animation name as proof of a flight system—it is part of jumping/falling animation.

Track the player's entity GUID and the exact flags/state owned by the cheat. Suspend or turn off on death, checkpoint reload, mission change, possession, vehicle/turret entry, grabbed/disassembled state, shop, and cinematic control. Avoid restoring cached entity pointers after any of those transitions. Noclip should retain the last valid non-overlapping position and return there if switched off inside geometry. Flight should stop its cheat velocity before gravity resumes.

## Free camera

`CamDebug.cpp` already implements independent position, yaw/pitch, forward/strafe, and vertical movement with no camera collision. It is included in the current build; CMake's Release configuration defines `_FANGDEF_TEST_BUILD`, so the production-only exclusion does not remove it here.

The existing debug wrapper cannot be exposed unchanged as a co-op menu toggle:

- `gamecam.cpp` allocates **one** debug camera and tracks one monitored player globally.
- Entering it freezes the target camera and disables the player's entity work.
- The debug camera's Action input teleports `CPlayer::m_pCurrent` to the camera, which is unsafe to reuse as an ordinary player-specific freecam interaction.

Reuse the transform/control math with a per-player camera state, preferably through the existing per-player simple-camera slots or a dedicated cameraman. Keep body simulation/script work running, intercept only that player's input, and render only their viewport through the detached camera. Do not relocate the body on ordinary Action input. Cinematic/shop camera ownership takes priority and exits the cheat. On exit, restore the camera for the player's **current** actor rather than a saved actor pointer.

Proposed default: world and partners keep running; body remains vulnerable unless the separate invulnerability cheat is enabled. Camera movement alone does not trigger gameplay tripwires.

## Suggested implementation order and focused checks

1. Chip grant: verify P1/P2 counts separately, normal console consumption, no Secret Chip changes, and unavailable while possessing another bot.
2. DET retrieval/grant: verify an authored DET mission, a mission without planting sites, a pack already consumed, requesting-player ownership, normal wall planting, countdown events, and checkpoint restoration.
3. Freecam: verify two independent viewports, restoration after a cinematic, menu exit, death/reload, and unchanged body position.
4. Shared flight/noclip movement: verify collision retained for flight, bypassed for noclip, no forced checkpoint while exploring, safe toggle-off, and cleanup during scripted/vehicle transitions.

The item grants alter ordinary inventory and can consequently be saved in a checkpoint. Movement/camera toggles should remain session-only and should clear on transitions. This investigation is based on source inspection, not newly implemented or playtested features.

## Source entry points

- `port/pc_cheats.h`, `port/pc_cheats.cpp`: actions, campaign/player checks, status messages, per-player toggle bits.
- `ma/App/ma/PauseScreen.cpp`: fixed cheat layout around line 510; labels/callback around line 2311.
- `ma/App/ma/ItemInst.cpp`: default Chip/DET inventory slots around lines 636/656.
- `ma/App/ma/collectable.cpp`: `GiveToPlayer` around 1664, resource checks around 2571, relocation around 2105, inventory credit around 3698, recipient pickup check around 4045.
- `ma/App/ma/econsole.cpp`: chip consumption around 1507–1525.
- `ma/App/ma/edetpackdrop.cpp`: planting eligibility around 205, consumption/events around 462–497.
- `ma/App/ma/bot.cpp`: collision bypass around 3936, no-gravity air handling around 5320.
- `ma/App/ma/player.cpp`: world/air recovery around 1359 onward.
- `ma/App/ma/CamDebug.cpp`, `gamecam.cpp`, `CamSimple.h`: existing camera movement and camera ownership.
