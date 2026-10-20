# Playtest batch, 13 October 2026

Tester report: Coliseum endings and lost upgrades, vendors missing in Get To The Tower, and four
cosmetic issues. Built on 1.0.7 (`7d8a470`).

## Fixed

| Report | Cause | Fix |
| --- | --- | --- |
| Upgrades missing from Race to the Rocket on | Each Coliseum strips the player to unarmed (`SetToUnarmed(TRUE)`: no weapons, no items, 3 batteries) and restored the entry inventory only on its scored-win path. Any other ending saved the stripped inventory as the next level's, and the next Coliseum copied that. | `ColiseumMiniGame.cpp`: the next-level inventory points at the entry copy from the moment of stripping, whatever ends the level. `player.cpp`: the pointer is cleared at every level end (a quit Coliseum can't leak it). Saves already broken are repaired at level start: a later level whose inventory has no items at all and fewer weapons than the Coliseum 1 entry inventory gets that inventory back (log: `Port: level N started with a Coliseum-stripped inventory`). |
| Round Two ends while 3 shielded Red Grunts shoot | Retail's arena list held only bots in the world at start; `xebcbttle02` adds `g1-g3_group1` (and more) once the zombots die, so the fight was won as they arrived. | The late-wave inclusion made for Coliseum 3 now covers Coliseums 1-3 (Round Two's list: 13 bots). |
| 15 Minutes: nothing happens, then the whole sequence fires and the level ends | The port's 60 s "no new bots" rule (for Coliseum 3) could run before the fight began, while arena bots waited out of the world, and call the fight. | The rule only runs once an arena bot has died. Not reproduced in play; inferred from the rule and the symptom. |
| Slim and Shady (Mr. Pockets) missing in Get To The Tower | Possessing a bot hides the vendors; unhiding only cleared the state. `xecdliteon1` places them once (`barter1`) in the opening cutscene, so the first possession lost them for the level. Logged: placed, then hidden. | `BarterSystem.cpp`: unhide puts them back at the last scripted point (also a placement requested while hidden); `Barter_Remove` and checkpoint restores before placement forget it. Placement/hide is logged. |
| Fences and girders lit through their holes | Extra light passes (headlamp, spots) and specular add with the Xbox's exact depth match; PC's tolerant match passes in cutout holes, and those passes don't sample base alpha. | `fdx8sh.cpp`: every first pass writes 1 (cutout) or 0 into the stencil where it draws; a cutout's extra light and specular passes draw only where it is 1. The surface pass is not stencil-tested (it alpha-tests). A per-material value was tried first: world geometry runs all light passes before any surface pass, so surface passes failed and fences showed grey and flickered under the headlamp. |
| Decals and glows over nearer objects | `D3DRS_ZBIAS` mapped to a constant `-1.5e-5` per level (a guess). A constant offset grows with distance squared (GameCube's projection shift grows linearly), pulling distant decals ahead of things in front. | `d3d8_compat.cpp`: `5e-6` per level, the usual D3D8-to-9 mapping (~80 steps of 24-bit depth). Not compared in the Access The Ruins poster scene. |
| Water and goo "warpy" | The ripple grid steps once per rendered frame with per-step damping, so above 60 fps ripples die before spreading. | `fliquid.cpp`: fixed 60 Hz steps (at most two per frame). |

## Not reproduced

Tether reticle gone after LOSING SIGNAL: nothing in the signal-loss path, static drawing or unplug
disables the reticle; the reticle switches are listed in this session's search (`ReticleEnable`,
`m_Reticle.EnableDraw`). Needs the mission and what happened around the warning.

## Verification

Offline suites pass (checkpoint/EUK 2,042, reported systems 963, Slosh 118, vehicle 118, pickups
180,016). Native runs: Coliseum 2 start list; Get To The Tower vendor placement (the hide/unhide
path was not exercised: no possession in that run); You Know The Drill renders the same with the
stencil change; Seal the Mines liquids run. Needs play: a Coliseum chain into Race to the Rocket,
15 Minutes, Round Two's final wave, vendors after possession, fences under the headlamp, posters.
