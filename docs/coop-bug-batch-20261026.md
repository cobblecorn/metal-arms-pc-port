# Campaign co-op reports: fixes and review handoff

26 October 2026. Scope: the current report batch after the Level 15 console repair. This checkout contains earlier changes from other chats; the per-turn source snapshot is `build/backups/pre-coop-batch-20261026-180608/`. This pass does not revert the effects limits, laser controls/25-energy rule, Slosh aim, friendly Miner death, tank fixes or previous console safeguards.

## Coverage

| Report | Result | Evidence and practical limit |
| --- | --- | --- |
| Level 15 first arcade Rivet Mil crash | Prior repair retained | See `docs/level15-console-crash-20261025.md`. Current native solo handoff and P2 interrupted fade are included in the release checks. No claim that the reporter's unavailable crash stack is identical. |
| Level 15 guards fail to finish a scene | Existing scoped fallback retained | 13,500 production-AMX checks pass. This pass does not add another timeout or shorten the scene. |
| Secret Rendezvous blue Mil / panel blocks progression | Fixed | Actual P2 possession and panel action now unlock `hubgate01`. The previous script compared the operator to its cached P1 handle; its booth trigger also treated another co-op player as an NPC intruder. |
| Secret Rendezvous tether requires being extremely close | Camera ownership defect fixed | Reticle projection and tether sight origin now use the owning co-op player's camera. Native projection is checked with the other player's camera active. A complete manual blue/laser-grunt tether interaction is still a follow-up, not established by the projection probe. |
| Level 2 enemy elevator does not descend | Not reproduced; no speculative elevator change | The actual `WEDMmines01` room objects `elevdown` / `m_elv_2a` descend in 2P and 4P when the party crosses in stages. The 4P run reaches unit position 0.903. An early fixture relocation deliberately suppressed tripwire updates; its initial failure was a test setup error and is not evidence of a game defect. |
| The Hand Is Mightier battery / Toaster only given to P1 | Fixed | The authored `xemc_gime.sma` in `WEMCcity_01` gives `Flamer L1`, then `Battery`. Four real player inventories each receive that Toaster and increase from five to six batteries exactly once. This is the authored Toaster weapon gift, not an invented L2 Toaster upgrade. |
| Co-op spinning washer displaced / too tiny after first adjustment | Fixed and resized | Align the spinning mesh with the count's actual text row, fit/recenter it inside its own projection, and clear that HUD rectangle's depth so world geometry cannot hide P2's icon. The final visible mesh is larger than the rejected first adjustment. Narrow 3/4P views shift the icon/count together to clear the starting health bar. 2P and 4P layout checks and native frames were reviewed. |
| Rivet alternate fire | Present in current build; reporter failure not reproduced consistently | L3 uses primary-fire hold on the embedded rivet, then release to explode it. Three inspected native mouse runs charge to full steam and detonate on release, consuming one round. No Rivet gameplay code was changed. One earlier run failed its charge/release check and did not reproduce in those subsequent runs; retain the report as a manual follow-up if it recurs. |
| Other missing alternate weapon mechanics | Source audit completed; not a full weapon playthrough | Laser charge, Mortar draw/release, Ripper L1 release latch vs upgraded repeat, Scatter Blaster upgraded second-barrel delay and rocket target tracking are implemented. Secondary input is still the selected item; do not invent right-click alternate modes for weapons that use primary hold/release or automatic upgrades. |
| Sustained turret accuracy loss | Left as authored | Report explicitly identifies it as an original mechanic. |

## Production changes

### Story gifts use the existing personal pickup system

`CMAST_GoodieWrapper::G_GiveToGlitch` previously always targeted P1's original bot, and `GiveToPlayer` marked the spawned goodie as that player's recipient. Personal world-pickup logic therefore never offered that gift to partners.

The PC wrapper now calls `CCollectable::GiveStoryToPlayer`. In local co-op, batteries, weapons and weapon upgrades are placed as an ordinary unowned world pickup at the gift location. Existing per-player claim bits, inventory updates, visibility and checkpoint serialization do the rest. Other scripted items and solo use the original recipient path. Shop purchases and explicitly targeted `GiveToPlayer` grants retain their recipient semantics. There are no new save fields or collectable layout changes.

Files: `collectable.cpp/.h`, the `G_GiveToGlitch` call in `MAScriptTypes.cpp`.

### Security events recognize the real co-op operator

`PortCoopScriptPlayerActor` is called from the PC event-dispatch path for each listener. It is restricted to `WEMCcity_02`, listener `xemc_hubgt.sma`, `switch` from `hubgateswitch`, and `tripwire` from `boothtrigger`. Only an actual current player actor is aliased to that listener's registered, still-valid cached player handle. NPC entries, unrelated sources/events/listeners, solo and PvP remain unchanged. The switch and possession system retain the actual operator; the global event is not rewritten for every listener.

The exit under test is `hubgate01`. `hubgate` is a separate alarm-related gate and must not be used as the success condition. The baseline P2 route test failed with the correct exit still locked; the repaired test unlocks it and leaves P2 in the Mil. Offline tests also confirm NPC booth events keep their original actor for the authored alarm branch.

Files: `MAScriptTypes.cpp`, `FScriptInst.cpp`, `port/pc_script_goals.h`.

### Tether projection uses the owning view

`CReticle::TargetSys_UpdateTargetedPoint`, `IsPointWithinReticle` and `ComputeUnitWithinReticle` previously used `FXfm_pView`, which may still belong to another split-screen player when weapon simulation runs. The tether obstruction ray had the same shared origin problem. These paths now select the safe owner camera in local co-op, including its existing shared-view source when appropriate. Other modes retain their existing active-view path.

No larger range, wall bypass, forced possession, relaxed dataport facing or altered lock duration was introduced. The regression constructs the point at the reticle's actual normalized origin (Y is 0.230 in this view), not at camera center. An earlier center-point assertion was invalid and has been corrected.

Files: `reticle.cpp`, `weapon_tether.cpp`.

### Washer render and size

The old square icon viewport was based on view width, while the counter's row height was based on view height. The new co-op viewport uses the text row as its size/alignment reference and explicit square FOV. The final mesh transform accounts for its bound center. It draws over freshly cleared depth in that small UI viewport, so the world cannot partially occlude it in subsequent split views. The original single-player branch is retained.

Files: `Hud2.cpp/.h`. `CHud2::PortTestWasherLayout` is a read-only opt-in fixture helper. `weapon_rivet.h` and `eproj_arrow.h` also add read-only test accessors, with no new members or normal-gameplay behavior.

## Validation and artifacts

Build: Win32 Release through `cmake --build build --config Release --target ma_port -j 4`.

Native test clients are real windowed, rendered game instances with muted audio, Discord disabled and separate `build/test-saves` folders. They are not headless simulations or physical-controller playthroughs. Fixtures move actors or select equipment where stated; they do not replace the actual inventory, panel, lift, projectile or script implementation being observed. The opt-in `report-batch-` modes bypass the controller-reconnect overlay for 3/4P fixture runs only; normal co-op still requires its controllers.

- `tools/test_coop_batch_native.py`: 2P/4P washer, 2P/4P authored gifts, P2 panel, 2P camera ownership, 2P/4P lift, L3 Rivet primary mouse hold/release. Each run records the executable SHA-256 in its `.run.json`.
- `tools/test_coop_cached_players.py`: 49 production binding/AMX and scoped event identity checks.
- `tools/test_booth_fallback.py`: 13,500 production-AMX scene checks.
- `tools/test_shop_purchase.py`: 66 production purchase checks.
- `tools/test_collectable_skiplist.py`: 180,016 PC collision/collection checks, plus the legacy branch check.
- Current final-build runs and hash: `build/logs/coop-batch-release-20261026.json`. Earlier source-identical gameplay results are in the `sized`, `final`, `diagnose`, `repeat1` and `repeat2` logs. Initial failed fixture runs were retained rather than relabeled as passes.

Native washer frames: `build/shots/coop-batch-washer-2p-owner1-final/shot_004.png` and `build/shots/coop-batch-washer-4p-owner1-release/shot_005.png`. PNGs are lossless format conversions of the native BMP captures; surplus temporary frames are removed after retaining representative frames and all logs.

The installation/staging receipt records exact output hashes and confirms unchanged earlier source hashes. The released `MetalArmsPC-1.0.10.zip` and user-renamed `MetalArmsPC-1.0.20.zip` are preserved; any new test ZIP is labeled as a dated co-op candidate instead of silently reusing a release number.

## Focused manual test flow

1. With keyboard P1 and controller P2, collect washers separately. Verify the rotating icon stays beside the correct count, is readable, and disappears normally. Repeat in four-player split screen if available.
2. Start The Hand Is Mightier. Walk each player through the starting Toaster and battery gifts. Each should receive one battery and their own Toaster. Reload a checkpoint after partial collection and check that claim state rolls back with the inventories.
3. In Secret Rendezvous, have P2 tether the required blue Mil and operate the security panel while P1 stays outside the booth. Confirm the intended exit opens and no false intruder response is triggered by P2. Repeat with P1, then test laser grunt lock from normal range while the other player's camera faces elsewhere. Walls and rear-facing closed ports should still prevent locks.
4. Revisit the reported early enemy lift with players entering at different times. The current fixture could not reproduce a stall; if normal walking still fails, retain the exact checkpoint/world and log rather than widening every elevator trigger.
5. With Rivet L3, fire into a nearby solid wall and keep primary fire held for several seconds after impact. Wait for the steam, then release. Compare a tap and a full hold. Report the weapon tier, world and input device if the charge fails. L1 is not the remote explosive version; L2 has a timed explosive rivet.
6. Retry Level 15's first arcade, release immediately during acquisition, and reload its checkpoint. The earlier safeguards remain in this build; keep a crash log if a failure persists.

Exavolt/randomizer investigation and proposed plain-file framework: `docs/exavolt-native-compatibility-20261026.md`. No external EXE was executed and no mod framework was installed in this pass.
