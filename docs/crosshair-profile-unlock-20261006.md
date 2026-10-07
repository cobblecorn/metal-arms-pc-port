## Crosshair restoration / current-profile mission unlock (2026-10-06)

Installed normal Release/staged/Desktop EXE `87AA6003246850A79D924064C4853E4F49BF3AE02089AC43C7362AF2FC7C7560`;
PDB `1315C7CCA2D1B4BF234D02739644F326A4360B88889EB747DB50387EB005DDDA`. Backup `D:\Documents\metal arms source port\build\backups\pre-crosshair-unlock-20261006-004343`.
Record `build/logs/crosshair-unlock-install-20261006.json`.

Safe co-op factory box exit now calls TurnOffHUD(FALSE) after CoopSyncStage,
clearing the forced reticle override inherited from Empty Primary. Previously
HUD/control returned but newly equipped weapons could not draw crosshairs.
Solo and PvP paths unchanged. Native two-player packing: **25 PASS / zero FAIL**,
including both crosshairs at actual box destruction/safe exit; no assertion/crash.
Offline packing66 / party149 pass. Native fixture intentionally destroys box
and returns menus, isolated saves; never use that fixture for the user's play.

User explicitly chose **unlock current profile**. Backed up and unlocked all42
campaign missions in AppData MAGITS blizzard solo profile and both blizzard
co-op records. Preserve settings, real stats, existing inventories through prior
current mission, unrelated profiles and PvP unlock fields. Populate only later
inventories using actual retail per-mission CSV/default logic, exactly as the
retail unlock code does. Temporary native inventory exporter was removed before
final build; no new menu/cheat feature. Personal CRC and outer co-op CRC verified;
co-op inner zero headers preserved. Save manifest `build/logs/blizzard-unlock-20261006.json`.
Finished campaign flag disables Continue; all missions accessible through Replay.

Launched **26 Ruins / WEMCcity_05**, two players, Desktop PID 27692.
Log `D:\Desktop\MetalArmsPC\ruins-next-test.log`. Normal CLI quick launch: no save profiles, no fixture env.
Leave this user play session running. Their unlocked profile is available from
normal menus on subsequent runs. Native snapshot evidence
`build/shots/ruins-next-current-20261006/latest.png` (if captured).
No computer-use skill or subagents.


Original save files and hashes are recorded in the manifest. Every per-level stat block and setting was verified unchanged before atomic replacement. No fabricated completion times or 100-percent achievement flag.
