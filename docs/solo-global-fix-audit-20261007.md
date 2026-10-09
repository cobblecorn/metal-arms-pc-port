# Solo coverage of campaign and PC repairs — 2026-10-07

Compared the campaign notes with current source, including the changes from
`1839b06` through `1d3594b` and the existing working tree. Other-chat changes
were preserved. No additional unintended co-op-only gate was found in the
repair families below. This is a source/scope audit with bounded native tests,
not certification of an uninterrupted campaign or PvP playthrough.

## Door correction and stale records

`ma/App/ma/eboomer.cpp::_CoopCanBreachEntrance` originally required
`IsLocalCoop()`. Commit `1d3594b` removed that requirement on October 6.
The helper now accepts credited player damage at `WEWCcomm_02/frontie` with
one through four players. Despite its historical name, it is not co-op gated.
Health, armor, destruction and the DET-only/zombieball-only filters still run.
Other gates, NPC damage, invalid player indices and PvP maps do not match.

The October 3 playthrough note, HANDOFF entry and PORTING description still
described the original co-op-only scope; they now state the current solo scope.
The production-method test also incorrectly expected solo rejection. Corrected
it to require solo acceptance, all co-op owners and the existing exclusions.
`tools/test_coop_playthrough.py` now passes **336 checks**.

## Scope inventory

| Repair | Current source / mode behavior |
| --- | --- |
| Communications-centre blast gate | `eboomer.cpp` exact world/entity and credited-player check; solo and co-op. |
| Spy Factory wrench/claw actor and rig | `botglitch.cpp` binds the Spy actor when **not co-op**, or for P1 in co-op. Solo is explicitly included. `weapon_wrench.cpp` does not require co-op for disassembly. The claw pickup is not a friendly-fire damage operation. |
| Crane listener/camera | `SpyVsSpy.cpp::ListenerOrientationCallback` accepts solo listener zero and additional active co-op listeners. Ordinary solo is valid. |
| Factory assembly animation crash | `port/gcdata.cpp` compatible key-range overlap handling and Fang slider/interpolation fixes have no co-op condition. They apply to PC animation loading in every mode. |
| Factory packing animation | `SpyVsSpy.cpp::CProgramStage::SwitchTo` enables crate/arm animation and work under the PC build guard, with no co-op condition. |
| Factory packing crosshair | Solo already restores HUD/reticle at crate delivery. The later co-op-only reticle repair undoes the extra suppression introduced by `CoopSyncStage`; that suppression never runs in solo. No solo expansion is needed. |
| Fabricator release | `BotDispenser.cpp::ConfigureAI` and `IsBotExiting`, plus `AlarmSys.cpp`, protect campaign release. `IsMultiplayer()` excludes PvP, not solo. Both campaign modes use this repair. |
| Missing prison chip | `entity.cpp::CEntityBuilder` accepts the retail six-field goodie entry under the PC parser guard; no co-op condition. |
| Caged They Live allies | `Ai/AIGameUtils.cpp` cage release/recruitment repair uses campaign rules, including solo. |
| Sniper's Lair Goff parts | `collectable.cpp` sends `goffpickup` under `IsSinglePlayer()`, which includes solo and local co-op. |
| Asylum door kill goal | `port/pc_script_goals.h` matches the exact script/opcode window, changes 14 to 10 and retains 14 spawns. No co-op condition. This was deliberately documented as a solo/co-op gameplay change. |
| Booth guard arrival | `MAScriptTypes.cpp` requires the campaign/cutscene/script actor context, not local co-op. Solo receives the corrected destination. |
| Coliseum 3 arena gates | `Door.cpp` repairs only `outerdoor6` and `outerdoor3` in `LEVEL_COLISEUM_3` under the PC guard; no co-op condition. |
| DET countdown cleanup | `edetpackdrop.cpp::_ClearHudTimer` clears matching ownership on every active HUD, including the sole solo HUD. |
| Cut dialogue / flamethrower audio | `BotTalkInst.cpp` protects natural completion of finite undamaged cinematic voices; `weapon_flamer.cpp` restores the retail sound-group fields. Both use PC build guards without a co-op gate. |
| Slingshot lifecycle and firing | `weapon_mortar.cpp/.h` owns its combiner per instance and cleans up owned resources; PC digital release behavior also applies to solo. |
| Weapon visibility / Scatter Blaster ammo | `botglitch.cpp::_ChangeWeaponIndex` and `weapon_blaster.cpp` repairs do not require co-op. |
| RAT turret inversion/sensitivity | `site_botStateFns.cpp` applies the PC correction for the RAT gun subclass, independent of player count or game mode. |
| Texture, cutout, waterfall, particle-camera, baked shader repairs | Shared converter/render paths have no local-co-op requirement. Mode-neutral production draw/texture checks pass. |
| Sphere-sweep and teardown assertions | `fcoll_sphere.cpp` PC sweep arithmetic and entity/turret cleanup changes are shared engine paths, not co-op-only workarounds. |

`MultiplayerMgr.h` defines `IsSinglePlayer()` as the campaign flag,
`IsLocalCoop()` as campaign plus the local-co-op flag, and `IsMultiplayer()`
as the inverse campaign flag. A campaign guard should not be mistaken for
a solo exclusion.

## Spy wrench/claw native proof

Current Release: SHA256
`B60E8BF0E263CFFC924BEC0CE28AD21E24AA3F5A572086BC2AC7C264A4BAC2FA`.

`python tools/test_spy_crane_native.py --players 1` passed **25 native checks**:
natural introduction, real wrench disassembly, head pickup and listener handoff,
headless-body reconstruction and keyboard movement, torso and leg pickups,
conveyor transition, retail builder animation, intact rebuilt Glitch, and normal
control after the post-assembly Shhh transmission. No assertion/crash,
audio-error or script-error reports appeared. The final native snapshot shows
rebuilt Glitch facing the Mil guide during Shhh's transmission.

Log: `build/logs/spy-crane-native/1-0cde5f61/native.log`.
Snapshot: `build/shots/spy-crane-native/1-0cde5f61/shot_035.png`.

The fixture grants the wrench, selects the destroyed-console mesh and moves the
actor between the actual grates; it does not set collected-part flags or force
the crane/assembly transitions. This verifies the repaired sequence rather than
the preceding combat/navigation puzzle. All saves and logs are isolated, and
only the tool's own game process is closed.

The initial diagnostic (`solo-scope-spy-crane-20261007.log`) is **not a passing
run**: its forward-input taps ran into the dispenser, and its last checks
deliberately disturbed the solo inspection actor while expecting the co-op-only
forgiveness policy. Corrected the fixture to finish solo after assembly and
Shhh, retaining the retail solo punishment; the new runner injects lateral
movement. No gameplay claw, inspection or friendly-fire rule was altered.

## Other verification and limits

- Native solo Coliseum gate test passed the real battle callback's delayed
  gate-open check and authored closing commands:
  `build/logs/coliseum-gates-native/1-232af555.log`.
- Current-Release solo upper-fabricator fixture passed construction, power-up,
  departure before retreat and resumed combat:
  `build/logs/fabricator-native/upper-1-e82c8795.log`. This fixture also logs a
  large vertical descent after release; it proves release/attack behavior only,
  not correct subsequent navigation. No navigation change was made in this audit.
- Current production-method checks passed: animation conversions **1078** plus
  slider/interpolation checks; dialogue **462**; DET HUD **70**; retail loot
  **19 legacy / 32 PC**; cage allies **132**; waterfall **6901**; particles
  **2592**; slingshot/texture/shader/turret **9144**; RAT aim/colors **660**;
  weapon visibility **611**; Scatter Blaster **8347**; teardown **6008**;
  asylum **68**; booth arrival **7241**; guide **85**; packing controls **66**.
  The GPU cutout fixture also passed. These are focused checks, not full playthroughs.
- The slingshot/texture fixture still described the superseded RAT inversion
  formula and extracted only the first preprocessor block. Updated that test
  to the current production calculation. The separate RAT test still checks
  real input inversion and profile sensitivity.
- Fabricator and Coliseum native helpers now default to `build/Release`, rather
  than an old `encounter-candidate` binary. An earlier candidate fabricator run
  is excluded from the current-Release evidence above.
- Release rebuilt successfully. This pass changes notes and test coverage only;
  the native source change is confined to an opt-in regression fixture. Existing
  gameplay repairs were already in place, so no new gameplay patch was deployed
  to the Desktop EXE. Test games have been closed; personal saves were not used.

## Deliberately co-op-only behavior

Partner respawn/regroup, instructor retries/lanes and copied HUD state, floor-pad
partner exemptions, nearby encounter groups, shared radios/cameras, private shop
isolation, cached story-player recovery and partner kill-credit fixes remain
co-op-only. They address extra-player behavior and were not documented as
general solo repairs. Elevators, exits and ordinary solo instructor consequences
retain their existing rules.
