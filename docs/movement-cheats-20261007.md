# Speed Boost and High Jump - 2026-10-07

## Persistent toggle revision (staged only)

Pause > Cheats now offers Speed Boost: On/Off and High Jump: On/Off for the player who paused. Speed uses the retail 2x multiplier; high jump uses 2x jump velocity and 1.5x gravity. Enabled cheats refresh their existing timers without restarting visuals or sound each frame. Off immediately cancels only that player's selected effect and restores movement/camera parameters.

Available in solo campaign and local co-op for living original Glitch. New enables reject PvP, possessed/special characters and removed actors. Turning Off works while dead or possessing a bot. Enabled effects resume after actor cleanup when original Glitch becomes playable. Leaving the game session resets toggles; no profile/save fields were added. Ordinary pickups remain timed when the cheat is Off.

The earlier KillAll shared-count correction remains in place. The other chat's newer PC muzzle-flash capacity increase and antenna stability guards are preserved in source and this fresh build. No old source snapshot was restored.

## Verification and remaining work

- Release build passed; 547 offline campaign-cheat checks passed, including P1-P4 isolation, independent Off, death, possession and PvP rejection.
- The first four-player native run mixed manual menu toggles with automatic fixture grants. Selected-player isolation and persistence past 25 seconds passed; simultaneous-player checks failed. It is inconclusive for the user's P1 report. The fixture now pauses while the menu is active and explicitly checks P1 before partners.
- Fresh native runs did not advance past mission startup. The older installed movement-cheat build also failed to advance with the comparison launch. Cause unresolved; current live menu ownership and startup remain unverified. No further launches this pass, per user request.
- Current EXE/PDB/DLL staged in build/movement-toggle-staged/Release. Hashes and source preservation recorded in build/logs/movement-toggle-staged-20261007.json. Root Release, Desktop and both existing pending copies were not replaced. Owned test games closed; the user's root Release game preserved.
- Runner: tools/test_movement_toggles_native.py. Logs: build/logs/movement-toggle-native/, build/logs/movement-toggle-native-20261007/, and build/logs/movement-toggle-compare/. All saves isolated.

## Earlier installed timed version

The earlier 25-second grant version's backup, hashes and tested pair are recorded in build/logs/movement-cheats-install-20261007.json. It passed 529 offline checks and eight native checks in each solo/four-player run (movement-cheats1-final-20261007.log and movement-cheats4-final-20261007.log). Those results apply to the earlier version, not the staged toggles.

Chip, DET, bot spawning, flight, noclip and freecam remain design-only.
