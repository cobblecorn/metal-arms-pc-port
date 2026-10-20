# Complete report coverage — 19 October 2026

This consolidates the original screenshot, the later Reddit report, and its
attached level/cosmetic list. The 26 rows below include overlapping reports;
they are not 26 independently reproduced defects. The October 12 audit is
historical. The October 18 audit describes the first installation; this document
also covers the continuation requested afterward.

Confirmed source defects have repairs. Some reported behavior is vanilla or
already works in native tests. Listening, final visual comparisons, and complete
mission playthroughs remain distinct from these targeted tests.

## Every reported item

| ID | Report | Result | Evidence / remaining limit |
| --- | --- | --- | --- |
| A1 | Max-level EUK crashes | Repaired: both the player's weapon tier and inventory upgrade chain must have a valid next tier. | All 23 retail EUK entries displayed in native solo/P2 shops; four real purchases each; maxed direct grants and world pickups rejected safely after leaving the shop. |
| A2 | Level 2 arm servo repeats / does not apply | Repaired: restore the saved servo on a new body and use per-bot weapon timings. | Native L2 pickup applied the upgrade and recorded tier 2 in inventory. Extracted production methods cover body restoration, checkpoint rollback, invalid values, and separate co-op timings. A full disk-save/mission round trip is still a campaign check. |
| A3 | Unhandled Exception cannot switch or reload in combat | Repaired: combat enables weapon selection and the reload input that depends on it. Instruction restrictions remain. | Native solo and P2 factory tests actually switched to the Rivet Gun and transferred reserve into its clip through the normal HUD tap-reload callback. Both co-op players retained control. |
| A4 | Instructor orders too quiet | Repaired: real and decoy movement orders play as 2D speech and duck competing audio while active. | Native true/decoy lines both reached playing 2D emitters; 1,624 production door/command checks include authored command-name parity and unrelated speech exclusions. Tests were muted; final listening balance remains for the user. |
| A5 | No direction out of combat arena | Repaired: six-second exit guidance for each player. A native test additionally found the victory-opened door immediately reclosing under default proximity behavior; the exact factory `battledoor` now uses script-controlled behavior. | Native solo/P2 tests killed the real six arena fighters through their death handlers, observed the actual exit remain fully open, and retained visible HUDs. The solo guidance capture was inspected. This tests victory processing, not defeating the fighters with physical controls. |
| A6 | Door lock lights cycle red/green | Repaired: retail layer 20 is a two-half atlas, not an automatic scrolling animation. Stop scrolling and select red S=0 / green S=0.5 when lock state changes and after mesh/checkpoint initialization. Script-controlled OTHER doors retain retail's red indicator. | Retail code at `80184bfc` / `801862c4`; native `free_door` red/green checks stayed stable across animated frames. Missing geometry/layers, repeated transitions, and all four behaviors also pass compiled production-method checks. Door movement/group rules are unchanged except the exact factory exit above. |
| A7 | Dropped washers sometimes cluster | Random drop scattering works; no forced spacing rewrite. Restoring attraction fixes the need to individually touch clustered washers. | Native goodie bag released sixteen washers: about 18 units maximum radius and 25 units between the furthest pair after settling. Close random pairs remain possible, as in the authored scatter. |
| A8 | Mega Energy is plain green | Repaired: restore missing per-instance retail tint (68/255, 138/255, 1). The same omitted block restores Mega Washer gold and servo gray. | Retail pool setup at `80177d38`; native real mesh-pool instances matched all three tints. Existing textures were untouched. |
| A9 | AA turret / Pincer audio clips or seems fast | Effects headroom repaired; no unsupported pitch/cadence change. Retail AA rate is 16 firing sounds/sec and the Predator laser group explicitly uses 1.20–1.25 pitch. Samples resolve at 22,050 Hz. | Deterministic sample-overlap analysis found clipping in the old mix. At the new −6 dB effects trim, modeled Predator laser/SPEW clipping fell to zero; AA fell from 7.622% to 0.011% of modeled samples. This is numerical evidence, not a console/DirectSound recording. Residual clipping and perceived speed need a listening comparison. |
| A10 | Slosh does not collect washers | Verified vanilla; retained. | Retail `_CanPickup` at `8017f8e0` permits Slosh energy, Mega Energy and Magma Bombs, and excludes regular/Mega Washers. No speculative bypass. |
| A11 | Excessive residual Grunt flamethrower damage | Repaired: choose the Mil's authored damage profiles instead of retaining Glitch L1. Persistent zero-period forms integrate their authored 60-Hz amount over actual active time. | Compiled production dispatch/profile checks at 30–240 FPS. Mil residual burn uses the authored 0.25-second tick; it no longer inherits Glitch's zero-period burn. Actual brief-contact/sustained-fire balance still needs playtesting. |
| A12 | Recruited manned vehicle keeps attacking | Repaired: synchronize the NPC driver's vehicle team/race and reset its old firing job on recruitment and reversal. | Native actual tank/Mil driver: normal recruitment completed, driver remained aboard, vehicle AI recognized Glitch as friendly, and unrecruit restored its original race. The fixture supplied the required recruiter resources and opened an eligible pilot's data port. This was not a recording of the reporter's exact hovercraft. |
| B1 | Fall to Pieces wrench does not finish level | Repaired genuine co-op dome gating and persistent boss-command faults. Normal timing still matters: remain disassembled when Corrosive destroys the occupied dome. | Native solo and P2 tests entered the real dome, used the wrench during a committed attack, and reached the real destruction callback/victory state. No forced victory callback. Cell/escort-to-ending route remains a full mission playtest. |
| B2 | Washers are not magnetized | Repaired missing retail hop/attraction routine: 20-unit unobstructed acquisition, four-unit rise over 0.2 seconds, then 0.25-second flight to the recipient's head. | Native P2 wallet credited a washer twelve units away without walking. Production checks cover obstruction, death, disassembly, vehicles, pool reuse, frame rate and co-op ownership. PvP/Slosh/Mega Washer rules retain their exclusions. |
| B3 | Shop SPEW/Rivet ammo not delivered | Verified working; no speculative delivery change. Purchases supply reserve; reload loads the clip. | Real purchase-handler tests in solo and as the P2 shopper delivered 75 SPEW / 50 Rivet reserve to live and saved inventory, preserved tiers, charged the right wallet and left the partner unchanged. Retain the report if an affected save/sequence recurs. |
| B4 | Possessed Mil shows three health containers | Repaired: health HUD container count follows the controlled bot instead of Glitch's inventory. | Native P2 possessed a real Mil while Glitch's inventory held three batteries. The inspected capture shows one Mil bar. Production checks also cover Predator two, Glitch inventory count, racing RAT selection, and invalid counts. |
| B5 | Weapon swap during tether rebuilding | Repaired: queue switches until the complete assembly flag clears, not just the earlier public `IsInPieces()` condition. | Native body disassembly retained the equipped weapon, rebuilt, then applied the queued change. Production tests separately cover the late assembly/blend interval for primary and secondary queues. |
| B6 | Player/Mil flame damage and reticle wrong | Damage repair is A11. Also synchronize profiles when a builder creates a non-L1 flamer directly, as well as on later variant changes. Restore retail `tfh_cross02`, scale/radius 0.15 and targeting radius 0.2. | Six profile variants and damage dispatch paths pass production-method tests; reticle values match supplied retail data. Native Slosh firing is logged separately. Final reticle appearance and combat balance remain playtests. |
| B7 | Swarmers absent in both Slosh missions | Repaired: manually controlled NO_AUTO_DEFCON nets keep their script-requested alert instead of comparing it against an uncomputed zero and shutting down. | Both real mission nets spawned living, moving swarmers and stayed activated after seventeen seconds in native tests. |
| B8 | Weapon description text tiny | Repaired: remove the PC one-texture-pixel override from the description and upgrade caption; use HUD panel scaling. | Real held-R weapon menu captured at 1280×960: text fits and is readable. Four-way split and other languages remain visual checks. |
| B9 | Enemy shield bubbles too large | Repaired Grunts from scale 1.0 to retail 0.4; retain Titan 1.0. | Exact retail Grunt caller and constant recovered; no blanket change to all shields. Final appearance remains a visual comparison. |
| C1 | Do Ore Die battery looks like energy | Repaired fresh-profile starting capacity from three to retail one, so numbered battery 2 is an upgrade instead of an already-owned duplicate. | Actual native authored pickup increased capacity 1→2. Existing three-battery profiles are preserved because that capacity may have been earned; already-owned numbered repeats still legitimately become energy. |
| C2 | Seal the Mines planted DET is inside-out | Repaired negative-determinant instance culling in six PC mesh draw paths. | All three actual ore-processor DETDrop transforms have determinant −1. Production renderer helper checks cover winding and odd/even reflections independently of view-reflection culling. Planted appearance still needs a visual playtest. |
| C3 | Clean Up battery missing / energy instead | Same fresh-inventory repair as C1. | Actual native authored pickup increased capacity 2→3 with the previous battery carried forward. Existing legitimate capacity is preserved. |
| C4 | They Live first scope appears as an upgrade | Repaired unnumbered EUK Scope to grant Scope L1 when absent, and prepare its attachment resources. Numbered EUKs retain upgrade-only rules. | Fresh native inventory collected the actual bridge-area item and acquired a real Scope L1. Maxed and unrelated unowned-gun EUKs remain protected. |
| C5 | Default mix too loud, guns mask speech | Repaired default non-dialogue effects trim to −6 dB. Speech/music and user sliders retain their values; `-sfx-db 0` restores untrimmed effects. A4 improves critical instruction clarity separately. | Muted native runs exercise the sound engine. Deterministic overlap analysis supports the extra effects headroom. Final listening balance is not claimed as verified. |

## Native evidence from this continuation

All test processes used `-mute`, independent save directories and resource/log
paths. Only their own PIDs were closed. Normal launches do not activate these
fixtures or skip missions. BMP intermediates were converted to PNG and removed.

Run labels retain the October 18 suffix because this pass continued that work:

- `reddit-systems-wedmmines02-1p-owner1-20261018.log`: actual door atlas, three pickup tints, servo pickup/save fields, sixteen-washer drop.
- `reddit-arena-wecffacty01-1p-owner1-20261018.log`: solo selection/reload, true/decoy speech, death-to-exit processing and guidance.
- `reddit-arena-wecffacty01-2p-owner2-20261018.log`: P2 selection/reload, both players' control/HUD and real exit.
- `reddit-rebuild-wedmmines02-1p-owner1-20261018.log`: physical disassembly/rebuild and deferred selection.
- `reddit-health-wedmmines02-2p-owner2-20261018.log`: real Mil possession and rendered one-bar HUD with a preserved three-battery Glitch inventory.
- `reddit-recruit-wewccomm_02-1p-owner1-20261018.log`: actual NPC driver/tank recruitment, friendly vehicle AI, reversal.
- `reddit-flamer-werrreactr2-1p-owner1-20261018.log`: actual playable Slosh weapon fires and consumes fuel; sound system active with output muted.

These logs are under `build/logs/`; compressed captures use matching folders in
`build/shots/`. Earlier fixture setup failures are not passing evidence: they
included requesting a servo through the weapon-only grant API, requesting ammo
for an already-full gun, and trying to recruit a pilot without the required
data-port/effect setup. The arena's premature door closing was a real production
failure found by the test and repaired before the final solo/P2 runs.

Reproducible drivers: `tools/test_reddit_native.py`,
`tools/test_vendor_upgrades_native.py`, `tools/test_reddit_gameplay.py`,
`tools/test_retail_door_commands.py`, `tools/test_reported_systems.py` and the
checkpoint/co-op/Slosh regressions listed in the October 18 audit.
`tools/audit_weapon_audio.py` writes its numerical simulation to ignored build
output without playing sound. Supplied retail disassembly/decoded proprietary
data remain under ignored `build/reddit-audit-20261018/`.

## Deployment

The Release EXE and matching PDB are the patcher inputs. Only those files are
installed in `D:\Documents\Metal Arms PC Port`. No patcher is built here. User
saves, sliders, texture packs and game data are not edited. The embedded resource
version remains 1.0.8.0, as requested by the existing work.

The prior other-chat patch still passes reverse applicability checks; its vendor,
arena, renderer/decal, liquid/effects and version changes are retained. The
continuation backup and install manifest are under
`build/backups/full-list-20261018/`. See `installed.json` there for final hashes.

Installed EXE SHA256: `FD318C5CAF81516312474C381A4CFED3B33141412C6AAC8CB289784A6B0CFFF4`.
Release build, targeted production checks, native probes above, installed hashes,
line endings, and incoming-patch preservation checks passed. No claim is made
that these replace the remaining listening/visual checks or a full campaign run.
