## 2026-10-06: Night Sneak fabricator exit

A known intruder reproduced the upper fabricator trap: alarm combat replaced the
release GOTO while the Jumper was inside, and retreat did not restore it. Campaign
bots now finish release movement before local reactions or alarm orders interrupt
it. Alarm protection uses the existing Target/Drop clearance sphere and ends once
the bot is outside. PvP dispenser rules bypass this change. Source review passed;
native cases and installation state are recorded in docs/fabricator-release-20261006.md
and build/logs/fabricator-install-20261006.json. The separate hidden rocket enemy
report has no confirmed placement diagnosis; authored long-range rocket grunts exist
in WECDsneak01. User root game PID13596 was preserved until it exited; the verified
EXE/PDB are now installed in root, Desktop and both staging directories. Native
upper solo/two-player and lower two-player tests passed; four-player setup did not
reach construction and is not verified.

## 2026-10-06: Secret Rendezvous countdown and ending

PC DET-pack HUD cleanup now follows countdown ownership across possession changes.
Finite 2D cinematic player dialogue finishes naturally (GL_22m_020: 3.815 authored,
4.556-second sample). This mission's co-op ending restores the story actor to Glitch
and releases the console before moving him. See docs/rendezvous-possession-timer-20261006.md
and build/logs/rendezvous-install-20261006.json. Vendor chatter, intermittent chamber
blockage and the unidentified end assertion remain unconfirmed.

## 2026-10-06: Sniper's Lair completion

The three Agent Goff quest pickups now send the missing retail `goffpickup`
notification; collecting the third completes level 28 in solo and co-op. Native
solo and two-player authored-pickup runs passed through the mission-complete
screen. Four-player testing required an unavailable fourth controller.
See `docs/goff-part-completion-20261006.md` and the install manifest in
`build/logs/goff-parts-install-20261006.json` for deployment state.

# HANDOFF - Metal Arms Windows port (read this first)

Current state for the next session or the next model, local (Windows) or cloud (Linux). It is written
so a model that has never seen the project can pick it up: what exists, how to build, run and test it,
the diagnostic commands, what is open, and how this user works. Other documents:

| File | What |
|---|---|
| `PORTING.md` | Reviewer-facing: build, run, controls, what changed and why, status checklist, known issues. |
| `CLOUD_SESSION_LOG.md` | Every change made from a cloud session, each with what to verify in a real run. |
| `docs/handoff-history.md` | Every earlier HANDOFF, chronological (sections 1-25), for the reasoning behind past decisions. |
| `docs/coop-audit.md` | Campaign co-op: what is done, what single-player assumptions remain. |
| `docs/mouse-menus-design.md` | Design for mouse-driven front-end menus. |
| `docs/world-export.md` | Export a level's static world meshes to OBJ, Blender, and Roblox Studio. |
| `docs/character-export.md` | Export character rigs, packed textures, and matched animations to Blender. |
| `docs/vendor-robot-porting.md` | Shady/Slim models, clip map, behavior, and source code pointers. |

## Nearby-group encounters (2026-10-06)

Desktop/staged EXE `A850549E5933B145141618153B7834EE61DAAE87BB52FE20CF3047FC6D9915CA`, PDB `C24AF745F28FCC6982C6F249187FB0972F64E7F0987C5EFB239B07648AE20CA6`.
Root PID39124 still plays the previous87AA build; **leave it running**. Hidden
one-shot installer PID12060 waits for its exit, then atomically copies the new
root EXE/PDB (unless superseded). Read authoritative current install status in
`build/logs/encounter-groups-install-20261006.json` before inspecting live symbols.
Candidate compiled in separate `build/encounter-candidate`, avoiding the locked EXE.

37 audited one-shot enemy/map triggers in14missions accept one real crossing
with all living original players within12-unit diameter /4units vertical.
Held releases require a current actor near that actual trigger/crossing; no old
queue release when grouping elsewhere or from a long warp's endpoint. No trigger
geometry expansion, fabricated arrival bits, or partner teleports. Elevators,
transport, exits, scenes, repeatable/named-actor/possession/unlisted triggers retain
existing rules. Solo/PvP untouched. Pure alarm networks were not blindly included:
they can drive doors and secondary script callbacks.

1,871 offline production-method checks pass; native two-Titan test9PASS/0FAIL,
no asserts/crash, isolated saves. All own test processes closed. User saves unchanged.
Docs `docs/coop-encounter-groups-20261006.md`; exact audited branches/script lines
`docs/coop-encounter-trigger-list-20261006.json`. Native helper
`tools/test_coop_encounter_native.py` / opt-in mode encounter-group; not for user play.
Backup `D:\Documents\metal arms source port\build\backups\pre-encounter-groups-20261006-121521`. One-shot build/patch/install scripts must not be rerun.

## Ruins DET-pack report resolved (2026-10-06)

User confirmed they missed the second pack. No production fix needed.
Live log showed P1 detpack02 rejected with zero DET-pack inventory. Read-only
live enumeration found uncollected authored packs at (766.51,-19.66,37.63) and
(827.84,18.03,-356.04). Neither titan1 nor titan2 drops DET packs. Isolated native
visibility run confirmed ground pack allocates its gwdmdetpack mesh in the
side alley by the open hexagonal metal bin and green dumpster. Original
player run Desktop27692 had closed before this check. Own isolated Release
PID21204 closed after user found it; no saves changed. No computer-use skill,
no subagents. Original live evidence `build/logs/ruins-detpack-live-20261006.json`.
Note: -start-at can run during intro, then authored intro resets position;
test had to rearm it after control returned. A test-only angle outside ground
caused recovery/death; not a normal mission crash. User found pickup manually.

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

## One-run box resume (2026-10-06)

Installed Release/staged/Desktop EXE `3545D930719C2C3D98D7E8F3F4849F78347D8C6E530AD70FD57F2A42CB3E2BFF`; PDB
`C39732719D6C7B7BED0B7F4052A5F8F96BED9372346BF5B6590C8D05E9EC3F94`. Record `build/logs/box-resume-install-20261006.json`;
backup `D:\Documents\metal arms source port\build\backups\pre-box-resume-20261006-002212`. Previous inspection fix is included unchanged.

User requested boot through to the box. Added --stage box to
`tools/resume_spy_combat.py`; normal/default combat resume stays unchanged.
The opt-in `spy-box-resume` branch enters actual PROGRAM stage and places P1
at the real box trigger, before fixture invincibility. Then exits once; no
automatic destruction/menu return or repeating test. Normal profile/checkpoint
storage. Direct -coop 2 controller routing caveat still applies.

Release and 66 production packing checks pass. Live Desktop PID 7368 at handoff,
user-owned play session, **leave running**. `spy-box-resume.log` confirms closing,
conveyor, delivery and restored P1; native snapshot under
`build/shots/spy-box-resume-current-20261006/latest.png`. No asserts/crash logged.
Partners watch until authored box exit. No game/UI automation skill or subagents.

## Factory inspection imposter softlock (2026-10-05)

Latest installed Release/staged/Desktop EXE: `F078E3D4F070570A540734A0C98D60B1996AD49D94201CB6193A90C11E5B8A47`.
PDB: `4D14BDCFE03124D5E13345FFEF863DF6F086653B47AF0032BB399EE3D712C322`. Record `build/logs/inspection-warning-install-20261005.json`;
backup and evidence in `docs/spy-inspection-warning-20261005.md`.

Live normal run 11016 was SEARCH / KILL_GLITCH with five open lockers. Earlier
_CheckDeath guard missed WALK_BACK's multiple/repeated open-locker failures.
The co-op-only state-entry guard now converts both lethal entries to authored
locker warning, closes all open lockers, and resumes search/control. A no-open
fallback restores own camera/control and patrol. Solo rules/one-locker closure
stay original. Chip search/contents survive; inspection is not auto-passed.
Normal controller ownership work is included and unchanged.

Release build; guide 85, party 149, DDR 117, camera 1202 pass. Final two-player
native `inspection-warning-pickup-native-20261005.log`: **17/0**, no asserts,
crashes, allocation/audio/script errors. Initial failures were paused locker
fixture animation / weapon-style chip grant / checking before deferred pickup; corrected test uses unpaused
retail animations and actual item pickup. No production chip-rule change.
Own runs closed, including old-binary interrupted fixture 10272. Current normal
startup PID must be checked, not assumed. No computer-use skill/subagents used.

## P1 controller ownership and explicit local joins (2026-10-05)

Installed normal Release/staged/Desktop EXE: `98343EE5AA32EDE9A1D7F17CA7D41632F928691B7860D98191B87E8AE1405BA6`.
PDB: `A863387EDBFE4A15409CBB0573AAA037B23590189C5FB937A679FCE45FDB060D`. Record: `build/logs/explicit-input-install-20261005.json`.
Backup and details: `docs/p1-controller-ownership-20261005.md`.

The controller opening Co-op/PvP selection now stays with P1 (any XInput slot).
Keyboard/mouse always remain P1. Keyboard entry still allows the first pad to
explicitly join P2, per user clarification. Preview connections become owned
only on selection; actual joined mask is frozen on forward navigation. Unclaimed
pads/hotplug can reach keyboard P1 after launch without creating a partner or
stealing a joined device. Partner unplug/reconnect preserves XInput slot ownership.
Normal SHARED solo/front-end routes the actively used controller to P1. Fresh
selection resets bindings; profile settings are unchanged. Held A/B cannot replay
across remapped join rows. Explicit layouts/direct `-coop N` CLI retain prior
routing; use normal menu entry to test the new policy, not combat-resume CLI.

Release/input build passes; input tests pass; **480 production routing checks**
pass. Native startup/profile-menu smoke has no crash/assert, not a physical pad
join test. Physical co-op/PvP/hotplug remains user verification. Existing wrapper
`dropfreq` warnings remain. Desktop old factory game had already closed before
install. Opened normal Desktop EXE for device selection; don't assume current PID.

Normal build was used for final install. A transient PDB link failure led to an
intermediate `/p:OutDir` build; it was superseded because CMake hardcodes normal
Release library paths. No CMake output settings were changed persistently.

## Factory packing recovery and normal combat resume (2026-10-05)

Installed Release/staged/Desktop EXE: `83BE4953BEBD4CE8C2ABFF962A802B768ECF9263D6E44877E40F9F388E146077`.
Record: `build/logs/spy-packing-install-20261005.json`.
Details and backups: `docs/spy-packing-recovery-20261005.md`.

All six combat enemies were dead; no friendly-fire change was needed. Enabled
retail crate/arm animation driving, glued only P1 to the moving box, and blocked
his translation/jump while carried (aim/select/fire remain available). Partners
stay hidden/held and watch P1 until the box is destroyed and P1 reaches checked
ground. Then partner controls/HUD/models/own views return. Solo receives the
animation/containment/ground recovery correction. Slingshot combiner config is
now per instance, fixing the multi-weapon teardown leak seen in the user's log.

Final native packing: solo **17/0**, two players **23/0**; no assertions/crashes
or undeleted class reports. Offline input 66, camera/menu 1202, party 149,
instructor 117 pass. Physical 3/4-player packing remains unverified. Existing
missing NPC/dialogue sound assets remain outside this change.

User requested not repeating completed instruction. `tools/resume_spy_combat.py`
and Desktop `Resume Spy Combat.cmd` launch the authored combat stage using
normal profile storage and one-process `spy-resume` environment. This does not
auto-destroy the box, return to menus, enable invincibility or alter ordinary
boots. Updated Desktop normal play PID was **23216** at handoff; don't assume
it is still running. Old process 37196 and all own completed fixture processes
were closed. User noticed test box destruction/menu returns; explicitly explain
fixture actions and promptly close completed tests. Avoid disrupting live play
with further full-screen test windows unless needed.

## Instructor hologram flash restored (2026-10-05)

The prior patch held holograms for the entire command response window. Removed
that co-op override at the user's request: the original alpha fade now runs in
all modes. Each command flashes its guides and they disappear on the authored
timer, before the response deadline. Partner guides keep distinct lane positions
and colors, mirror the primary fade, and disappear together. Delayed fake-command
previews retain their scheduled insertion. Per-player command checks are unchanged.

Verified: **117 production checks** including original solo/co-op fade, partner
expiry and command validation; **42 native co-op PASS, zero FAIL**, including
guides visible shortly after the command, all hidden while its response window
is still active, missed-command rejection and correct-command advancement.
Native log: `build/logs/spy-hologram-flash-native-20261005.log`.
No assertions, crashes, allocation, audio, script or schema errors were reported.
Physical 3/4-player play and full solo instruction remain unverified.

Installed live and staged EXE: `211B8FCA1D93594DB4D1EE222C0E8911C1900C2E5A1C4724223F14F8F6581257`.
PDB: `2811B8254A75429052E43C233EEC238C223CAA8D4C7D884BA547BD5DABBCF858`.
Record: `build/logs/spy-hologram-flash-install-20261005.json`.
Previous build backup: `D:\Documents\metal arms source port\build\backups\pre-spy-hologram-flash-20261005-180049`. Bink unchanged.

## Factory instructor challenge and real partner holograms (2026-10-05)

Latest installed EXE SHA256:
`8FDE393CD8E0A6D6F7DEFD232CFB6F5EFD781D4F206168AA6A5539C270189895`.
PDB: `E88053AEAEDCD19E0D33280A8D73FB6726850867BF3E58B9B30B46E6B48BB938`.
Record: `build/logs/spy-ddr-challenge-install-20261005.json`.
Backup of previous tested build: `build/backups/pre-spy-ddr-challenge-20261005-143024`.

- User reported standing still passed commands. Added failure timers/grace exceeded
  retail command windows and were reset before rejection. Earlier five-second
  isolated validator checks missed this real command timing. Co-op now validates
  each player's endpoint/facing/jump before advancing; 3.5-foot leeway remains,
  bounded below a whole step. Response window includes the extra movement grace.
  Current-frame input is consumed once before the deadline; completed jump state
  is retired before the next announcement. Training placement clears velocity
  and all human-control fields, preventing stale movement on reset.
- Partner guides were not allocated: resource loading happened before player
  initialization. Load reserves three co-op guides, then shows active slots only.
  Guide replication also used the inverse world transform; forward transform is
  now used. Green/cyan/orange/magenta guides persist through the response window.
  Fake preview timing and solo/NPC demonstration behavior remain unchanged.
- Final native `spy-ddr-challenge-tested-release-20261005.log`: **41 PASS, 0 FAIL**,
  no assert/crash/allocation/audio/script/schema errors. Actual short retail step
  rejects idle players and idle P2 with correct P1; imperfect completed steps pass.
  Distinct persistent holograms, checkpoint/death retry and authored exit checked.
  Native deadline fixtures isolate human input/velocity; no user's saves touched.
- `tools/test_spy_ddr.py`: **91 production checks**, including real command
  advancement under short windows for 2-4 players, position/turn/jump misses,
  last-frame jump input, zero-player-count guide loading and distinct forward/
  inverse transforms. Existing party 149 and guide 13 checks pass.
- Final installed solo opening `spy-ddr-challenge-tested-solo-20261005.log`:
  **9 PASS**, no assertions/crashes/errors. Full solo instruction playthrough,
  physical 3/4 controllers and every random dance remain unverified.
- Intermediate failing logs are diagnostic, superseded by the final runs above.
  Detailed notes: `docs/spy-instructor-challenge-20261005.md`.

## Goal

A working native Windows build of *Metal Arms: Glitch in the System* that runs the user's retail
**GameCube** disc data (disc ID `GM5E7D`, rev 0), with mouse-driven menus and, later, local campaign
co-op. The whole campaign is playable today; the work now is polish, the remaining log errors, and
co-op.

## Spy Factory rebuilt camera, escort and instruction leeway (2026-10-05)

Latest live Release EXE SHA256: `38DFD1A6AA8B3C2580CBD6CA0D418E9C4404CA53EDA85D52296608E23B506041`;
PDB: `EB6836D72BEFD59F0A267C2244D9E8F66E7385EFE4F375E766C033865BC01C23`. Installed 2026-10-05T13:03:11.7399491-04:00.
Backup: `build/backups/pre-spy-escort-leeway-20261005-130311`. Record:
`build/logs/spy-escort-leeway-install-20261005.json`. Bink unchanged.

Post-assembly inspection explicitly resets P1's own camera pitch. Partners now
use scripted escort routes behind P1, with independent views through the walk
and NPC demonstration; normal transition returns their controls. Instruction
lanes are spaced four feet behind P1 with distinct marker tints. Co-op has wider
target tolerance and extra timing/turn leeway; announcing the next command no
longer checks movement/facing against the previous target. Placement clears a
stale jump press to avoid another rejection in the reset frame. Solo rules stay
retail. Failure logs now identify the rejected player and timers.

Verification: final candidate native instructor **34 PASS**, escort/camera
**11 PASS**, solo opening **9 PASS**, no assertions/crashes/audio/script errors.
Actual authored escort/demonstration release, checkpoint restore, P1 death,
independent jumps, sustained wrong turn and large miss are exercised. New
production validator **23** offline checks cover solo and 2â€“4-player lane math;
existing party **149**, guide **13**, placement **296**, playthrough **328** pass.
Four-player native play could not run without missing physical controllers.
Fixtures enter later stages directly and do not certify all random sequences
or an uninterrupted entire factory mission. No game/test process left running.
Details: `docs/spy-factory-escort-leeway-20261005.md`.

## Spy Factory instructor/team recovery (2026-10-05, installed 12:12 EDT)

Latest live Release is now EXE SHA256
`479D60854D576F7444D3E10B09ADF5D4D89F79B351F54FFD5CCCD8687EEE0452`;
PDB `9D4015732AB143D4AF903E3E23F0884C7A6B0E4D6ECB05B1ECFD300E7A8E4B0B`.
Backup: `build/backups/pre-spy-party-20261005-121217`; install record:
`build/logs/spy-party-install-20261005.json`. Bink DLL unchanged; CMake Release
output overrides removed. No test/game process was left running.

The instructor now places every co-op player on the training floor with matching
unarmed/HUD state and validates each player's own commands in a separate lane.
Real checkpoint restore repairs partners' stale opening spawns. Instructor
failure or player death retries the whole team without needing living partners
to die or replaying the long briefing. The authored exit regroups/equips the
party for combat. Search/program stage transitions synchronize partner state;
any partner can search lockers and supply the objective chip. Cinematic snapshots
read the owning camera instead of a stale borrowed P2 view; the factory keeps
P1 as its scripted actor. Solo rules and the NPC instructor demonstration remain.
The initial claw puzzle is still P1-owned, as previously agreed.

Verification: 31 native co-op checks (real restore, independent jumps/P2 wrong
turn, actual P1 death with P2 alive, authored exit), 9 native solo opening checks;
no assertions/crashes/audio/script/allocation errors. Production helpers pass
149 solo/2-4-player mode/objective checks; existing guide 13, placement/checkpoint
296 and playthrough 328 remain passing. Targeted stage entry is test-only and
not a complete uninterrupted factory playthrough; physical 3/4-controller dancing
remains untested. Details: `docs/spy-factory-party-20261005.md`.

## Spy Factory parts puzzle, animation crash and partner regroup (2026-10-05)

- User saw the head collected, then the camera wait looking at the remaining
  body. This is authored: camera pauses at 11 seconds, wrench auto-rebuilds
  after eight seconds. Special bone callbacks retain each collected part.
  Move the rebuilt headless P1 body to the torso grate, use the wrench, then
  rebuild/move to the legs grate/use the wrench. Actual keyboard movement and
  all three retail claw pickups are verified without setting part flags or
  forcing stage/camera transitions. The factory parts puzzle remains P1-owned.
- Full testing exposed a real assembly crash: `aocff_asmb2` was rejected by
  `port/gcdata.cpp` for overlapping key arrays; builder slot 1 was NULL in
  `CMoveStage::_StartGlitchBuild`. Retail has four compatible partial overlaps.
  Converter now merges ranges with matching scalar widths and swaps each
  byte once. Incompatible overlaps and bounds errors still reject before any
  mutation. `tools/character_anim.py` follows the same structural rule.
- The builder root also has an authored constant rotation track with timestamps
  `[128,128]`. Fang's slider asserted outside that interval and interpolation
  divided by zero. All three scalar-width key sliders now hold endpoints;
  ratio helpers clamp endpoint samples and hold duplicate timestamps without
  division. Normal interior interpolation remains tested.
- Guide follow-up: retail `_CheckDeath` enters a kill-only inspection failure
  if P1 turns, moves >4 units, jumps or melees. It assumes a solo death restarts
  the mission; co-op can leave this stage stuck with living partners. Local
  co-op now skips this punishment and keeps the guide scripted. The guide
  ignores bot collisions/avoiding players while retaining world obstacle
  avoidance, so regrouped partners cannot push/block the scripted actor.
  `spy-guide-offline-20261005.log`: 13 production checks cover disturbed P1
  in co-op and retained solo punishment/normal inspection behavior. Native
  final test also disturbs P1 and places P2 beside the guide after recovery.
- User requested all partners recover at the rebuilt Glitch, preferably after
  Shhh's transmission. `CSearchStage::_WorkGuyState/GUY_STATE_WAIT_FIRST_WORK`
  now uses existing checked placement and checkpoint revival helpers in local
  co-op, after the blocking post-assembly transmission and before the guide
  starts walking. Alive partners relocate; downed partners revive intact,
  with HUD/controls restored. Deferred death transitions retry before advancing.
  Existing placement loops support P2/P3/P4; solo/PvP do not execute this block.
- Native `spy-regroup-dead-coop-20261005.log`: **34 PASS, 0 FAIL, 0 assertions,
  0 crashes** through actual parts pickups, assembly, Shhh transmission, P2
  death/revival/regroup and independent human controls. Test game was stopped
  after success; human game/save/log untouched. Earlier `spy-assembly-endpoints-
  coop-20261005.log` verified full assembly with 30 PASS and no assertions/crash.
  Prior `spy-complete-assembly-coop-20261005.log` had duplicate-timestamp
  assertions and is retained as diagnostic evidence, not a passing result.
- Offline `gc-animation-offline-20261005.log`: 1078 conversion cases (1068
  supported retail animations; one pre-existing 148-bone resource remains
  rejected by the 127-bone engine limit; nine alias/malformed cases), plus
  production slider/interpolation checks. Rejected input remains unmodified.
  `spy-regroup-checkpoint-offline-20261005.log`: 296 checks including checked
  placement for 2-4 players. `spy-regroup-playthrough-offline-20261005.log`: 328.
  Whole factory completion and physical four-controller play are not certified.
- Installed staged Release EXE/PDB/Bink with verified hashes and backup:
  `build/backups/pre-spy-assembly-regroup-20261005-003900`.
  Record: `build/logs/spy-assembly-regroup-install-20261005.json`.
  EXE SHA256 `5E7E417DFBCBB07879C04B5C1A627482048CF138B05D0A2E1807670A2C49E674`.
  PDB SHA256 `94EBDC02E646D37D9D3F63BAE23922C296C070834339537CC02F0021CB08A7FD`.
  Bink unchanged. CMake Release output overrides restored to normal.
  Installed Release `spy-assembly-installed-solo-smoke-20261005.log`: 9
  native checks pass, no assertions/crash; natural opening restores solo P1.
  User then reported the Mil guide aggroing P2 and interrupting P1.
  Final guide build installed after `spy-guide-final-coop-20261005.log`
  passed **37 native checks, 0 failures, 0 assertions, 0 crashes**. This includes
  placing P2 beside the guide and moving/turning P1 outside the inspection zone;
  guide stays scripted and P1 responsive. No test processes left running.
  Final install supersedes the earlier assembly/regroup EXE:
  record `build/logs/spy-guide-install-20261005.json`.
  Backup `D:\Documents\metal arms source port\build\backups\pre-spy-guide-20261005-005809`.
  EXE SHA256 `5E62E8079E97826226773C73A41FD968D99415D694E487A2C8CF69499B89E666`.
  PDB SHA256 `7EF0194C8BA7681943B850746E33644B42B484F8EAFD09C9606380BFDFDD2848`.
  Full factory completion and physical 3/4-controller play remain unverified.
  Restart the mission to clear any previously triggered hostile guide state.

## Spy Factory radio, wrench/claw assertion, co-op sensitivity (2026-10-04)

- The factory uses native `CSpyVsSpy::StreamStart`, bypassing the script radio
  adapter. It now uses the existing shared transmission helper in local co-op:
  P1 owns the single audio stream; every HUD/Glitch receives text/antenna effects.
  Completion, restart and unload follow the same owner/stop path. Solo keeps
  the original targeted transmission behavior.
- Every created Glitch previously overwrote the factory's shared actor pointer
  and installed its body-part animation callback. Local co-op now binds only
  P1's story body; partners retain their ordinary rigs. This fixes the factory
  looking at P2 while P1 used the wrench, and prevents shared part transforms
  from modifying partner bodies.
- User's screenshot/assertion at installed `SpyVsSpy.cpp:6418`, `!uPlayerIndex`,
  was the crane audio listener callback rejecting the second local listener.
  It now accepts the active co-op listener indices and applies the authored
  listener transform to each. The wrench uses `BreakIntoPieces`, and pickup
  checks on-ground pieces/proximity directly; neither goes through friendly-fire
  damage filtering. Use the grenade-slot wrench on P1 under the first broken
  dispenser/claw. Disassembly is intentional.
- Native Debug `spy-radio-restart-coop-20261004.log`: 18 PASS. Both HUDs/antennae,
  natural completion, P1 control handoff and checkpoint intro replay checked.
  `spy-claw-dispenser-20261004.log`: 19 PASS, actual retail wrench disassembly,
  head pickup, authored crane camera/listener and intact P2 body; no assertions,
  script errors, allocation or audio failures. Captures include `shot_051.bmp`
  showing the active crane camera. Test-only fixture destroys the main console
  and positions P1 at the grate near `conv_disp01`; it does not bypass pickup
  or set part flags.
  Earlier claw fixture runs landed on an upper conveyor and did not reach
  pickup; they are not validation evidence. Four-player run paused for absent
  controllers and is also not a pass. Full factory mission completion remains
  for human playtesting.
- Co-op sessions previously overwrote saved sensitivity from personal settings
  on every boot. Saved campaign player records now retain their sensitivity,
  even when multiple players select the same personal profile. Settings updates
  persist only that value in the existing co-op file; no save-format change,
  no in-run inventory/progress copied, no personal solo/PvP profile changed.
  Unsaved guest profiles remain temporary. `coop-sensitivity-save-20261004.log`:
  5 native PASS including duplicate names, disk reload and exact unchanged
  inventory/progress/personal profiles. Offline playthrough suite: 328 PASS
  (`spy-radio-offline-playthrough-20261004.log`); its stale vendor stub was
  updated for existing empty-offer fields, without production vendor changes.
- Release installed with backup `build/backups/pre-spy-radio-settings-20261004-232759`.
  EXE SHA256: `45A4A82CDE2FB1BD25D4BB33A26FE0A386C3D1D87E47E548D4B4B2D878E39E8D`.
  PDB SHA256: `5E5A4EBA3E556682B6C02EAB25A4731BF3422277344CDA2357FC6202A806A79F`.
  All three staged/installed hashes verified (including unchanged Bink DLL).
  Install record: `build/logs/spy-radio-settings-install-20261004.json`.
  Native tests use isolated logs/saves and timed windows; the user may have
  attempted to play in one such window, which auto-closed before positioning.
  Solo claw fixture disassembled successfully but did not reach pickup before
  timeout; do not claim that run validated a complete solo crane sequence.
- Final installed Release `spy-claw-grate-release-installed-20261004.log`:
  19 PASS, actual pickup/camera and both listeners, no assertions, script errors,
  allocation failures or audio errors. Initial dispenser-root fixture could
  miss the six-unit grab sphere: the pickup is at the front grate, roughly
  `(3,-6,-203)`, while the dispenser root is `(3,-8,-211)`. Fixture now uses
  root minus seven times local front and waits for grounding, without changing
  retail pickup rules. This explains earlier root-position Release/solo
  misses; logs are retained, not counted as passing runs. Test windows closed.

## Spy Factory intro control handoff (2026-10-04)

- User reported P1 unable to move at the start of `WECFfacty01` (Mil Spy
  Factory: Unhandled Exception), P2 apparently floating/stuck in the intro,
  and checkpoint restarts repeating the problem. Live state showed the
  opening transmission already finished, with the factory still marked as
  blocking and the current player context pointing at P2.
- `CSpyVsSpy::TakeControlFromPlayer` always handed P1's body to AI, but
  disabled/restored human controls through `CPlayer::m_pCurrent`. In local
  co-op that is commonly P2, leaving P1's controls NULL after AI deactivation.
  The helper now targets P1 for both sides of the handoff in local co-op.
  Solo retains the original current-player path. Partner controls are untouched.
- The opening `AS_18o_010$` transmission intentionally lasts 26.4 seconds.
  Opt-in native tests wait for its natural completion, then exercise another
  handoff with P2 current. Debug co-op: 6 PASS; solo: 5 PASS, no assertions or
  script errors. `spy-restart` additionally restores retail checkpoint slot 0
  and waits for the repeated introduction: Debug co-op 11 PASS, no assertions
  or script errors. P2 has human controls and is grounded after restart;
  native captures show independent views after the intro. The separate
  visual floating symptom was not reproduced as a persistent placement fault.
- Release update installed with backup
  `build/backups/pre-spy-control-20261004-203029`.
  EXE: `9F0C66711EF73BA5646E5F85ACD670E8EB165B8CCE554F27BAD5F232287EE736`.
  PDB: `53128E21A2D00D12139104FC7578DF165276EFA9BA7759C7A5F1EA2F453469AC`.
  Install record: `build/logs/spy-control-install-20261004.txt`.
  Installed Release `spy-restart-release-installed-20261004.log`: 11 PASS,
  no assertions, script errors, allocation failures or audio errors. Test
  process closed; user's log and saves were not changed by these tests.

## Mil City captain interaction and profiler shortcut (2026-10-04)

- User was lost in `WEMCcity_01` after Agent Shhh, reported friendly bots not
  responding, a building corner fall-through, and E changing a debug overlay.
  The overlay is global ProTrack, drawn at normalized Y=0.5; it is not P2 UI.
  Its fire/fire/jump activation chord shares normal PC controls, and E cycles
  its display once open. `PROTRACK_WORK` now requires `Gameloop_bDrawDebugInfo`
  on PC, consistent with existing perf/cheat gates. Explicit `-debug-info`
  retains the profiler. It does not consume normal interaction input.
- Retail objective after Shhh: search for rebels, avoid the city center with
  weapons armed. `xemc_crabt.sma` lines 195-199 accept an action on CapnPeanuts
  only from the cached `Bot_GetPlayer` handle, then start the escape sequence.
  SugarBaby intentionally has no such action. No hologram boss kill is required.
- `CEntity::ActionNearby` adapts only CapnPeanuts' script event in this exact
  world during local campaign co-op, mapping an original player body's action
  to the current story actor. Real operator remains unchanged for proximity,
  buddy AI, alarm/action callbacks. Solo/PvP and other NPC actions retain retail
  handling. Diagnostic mode now logs actual nearby action targets in `bot.cpp`.
- Native test `city-action2` first sends the unadapted P2 event (rejected),
  then uses the normal nearby action path (accepted and starts the authored
  escape scene). `city-captain-p2-verified-20261004.log`: 3 PASS, no assertions
  or script errors. P1 co-op and solo also start this scene (2 PASS each).
  Earlier fixture waited after teleporting beside the captain's raised alcove,
  dropping the player below the interaction height; corrected to interact at
  his height. The first patch also confused loaded registry index with the
  level enum; corrected to the exact world name before the passing test.
- Building-corner hole remains **unlocated/unfixed**. User cannot yet identify
  the corner more precisely. Live log showed partner recovery working, but
  that is not a collision repair. Do not invent collision blockers without
  reproducing the actual location. Full mission completion needs playthrough.
- User closed the game and authorized installing the pending audio and these
  fixes. Both builds pass; combined update installed in `build/Release`, with
  backup `build/backups/pre-city-audio-20261004-194243` and verified hashes.
  EXE: `B819D9E91C272CBD5DEB17FAFFE6FFBE6C70418F820C56C0FF4967F898271BA8`.
  PDB: `9D79E21AD95A7F1FF1F36F908A5B906A69538EAEBF788044796F8BD3A1928387`.
  Bink: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`.
  Install record: `build/logs/city-audio-install-20261004.txt`. Installed Release
  native check `city-captain-release-installed-20261004.log`: 3 PASS, no script
  errors/assertions. Test process closed; user's Release log/saves untouched.

## Rescue dialogue / flamethrower audio fixes installed (2026-10-04)

- User heard Glitch cut off during the final Slosh rescue conversation, then
  missing flamethrower/other sounds in `WERRreactr2`, and another later line cut.
- `CBotTalkInst::Work` previously destroyed a voice as soon as the animation
  schedule ended. Retail `GL_13m2_130` is scheduled for 2.254 seconds but its
  decoded sample is 2.711 seconds, with audible speech after 2.4 seconds.
  Other rescue lines have similar mismatches. Under `FANG_WINGC`, natural
  completion of forced-2D cutscene dialogue now waits for its finite active
  voice clip. Explicit termination/skip, damaged voice effects, infinite loops,
  and ordinary ambient/non-cutscene talk retain their previous behavior.
- Flamer's retail property prefix stopped before its inserted particle fields,
  leaving every sound handle zero. Read the six sound-group names from the
  verified retail columns 24,25,29,30,28,27, and resolve them after level banks
  load (like the existing retail laser path). Applies to all PC game modes.
  Retail variants all use `TstFire/TstTail/TstClip/TstSlap/TstEject/TstMT`.
  DOL vocabulary report: `build/logs/flamer-retail-vocab-20261004.txt`.
- Live Release log had repeated `Create3D` failures at the virtual pool limit
  (line 2886). Increased Windows/GC virtual instances from 160 to 512; audible
  DirectSound voice limits remain separate. Other platform budgets remain 80.
- Both Reactor defaults are explicitly **sfx ambience** in retail mission data:
  `ser_reactor`, `ser_genint`. Fresh native logs and read-only live process
  inspection confirmed playback, unpaused state, nonzero stream/master gains;
  there was no continuous missing music theme to restore. Did not invent music.
- Native opt-in checks in `port/coop_regression.cpp`: `audio-dialogue` works
  in solo or co-op, `audio-capacity` allocates 256 distant instances plus a
  finishing dialogue clip, `audio-flamer` directly exercises Slosh firing and
  stopping the weapon. It is a fire/stop stress fixture, not human held input.
  Final logs: `reactor-rescue-dialogue-final-20261004` (7 PASS),
  `reactor-solo-dialogue-20261004` (7 PASS), `reactor-audio-capacity-20261004`
  (4 PASS), `reactor-slosh-flamer-capacity-final-20261004` (2 PASS, actual
  `FlameLoop/FlameOut` playback logged), all under `build/logs`, no assertions
  or emitter-creation failures in those final runs. Offline cinema/UI: 1178 PASS.
  Earlier dialogue fixture wrongly used `delete` instead of `fdelete`, causing
  a test-only teardown assertion; fixed and rerun clean. Earlier 160-instance
  fire/stop stress hit capacity; the final 512-instance run passed cleanly.
- Debug and staged Release compile. Release output overrides reset afterward.
  Initially staged as `F24D08D9...` while user played PID 15812, then combined
  with the Mil City/profiler changes and installed after the user closed the
  game. Current installed hashes/backup are in the Mil City section above.
  Exact authored full rescue scene/later reported line still needs playthrough
  retest on the new build; native checks use the real rescue clip/talk system.

## Reactor floor pads and P1 camera restoration installed (2026-10-04)

- User's live `WERRreactr1` log confirms the timed-door deadlock: P1 held at
  `door1_trigb`, P2 held at `door1_triga`. Active `reactelec01.sma` sends both
  pads' enter events to `Door_GotoPos(door1,1)` without testing the operator.
  Extended the exact co-op floor-pad allowlist to Reactor 1's seven door pads,
  `bridge1_triga`, `orb_trig1/2`, and Reactor 2's four door pads,
  `bridge1_triga`, `orb_trig1` (active `react2door1` / `xerr2elec01` scripts).
  Original scripts, timers, enter/exit behavior and actual operator are retained.
  No radius change or broad `trig` name heuristic; elevators, exits, swarms,
  scene gates and unnamed/unverified triggers still gather. Solo/PvP unchanged.
- Production gate fixture passes 834 checks: all operators/2-4 players,
  separation across both pads, dead operators, solo/PvP and excluded gates.
  Native four-player focused door-pair test passes both pads with P2 alone,
  other players at spawn: `coop-reactor-door-pair-final-20261004.log`.
  Initial fixture held every player's controls, causing cinematic borrowing
  away from the distant door; leaving P1's view enabled lets the door animate.
  Expanded visible fixture also passed door2 a/b and door3 a, but did not
  trigger door3 b or door4 in a fresh load. Do not count it as a full pass or
  bypass retail arming to manufacture one. Final native fixture focuses on the
  reported door1 pair; other pads have offline/script coverage.
- `CMAST_CamWrapper::Cam_Deactivate` used `_ScriptPlayer()` (P2 when P1 died)
  to restore P1's camera. This permanently targeted P2 after P1 revival.
  Co-op now restores P1's camera slot to P1's current body, matching the
  existing CamAnim ending policy. Story actor selection, solo/PvP and shops
  retain their rules. New production regression failed before the fix; 1,178
  camera/menu checks pass afterward, including death/revival before/after
  deactivation for 2-4 players and non-co-op selection.
- First native camera fixture tested the handoff only, without activating a
  visible manual scene. User correctly challenged the stronger description.
  Expanded `camera-revive` fixture now calls real BeginCutScene + Cam_Init +
  Cam_Activate, holds the visible scene for four seconds while P1 is dead,
  verifies shared view, ends it and revives P1 afterward. Six checks pass,
  no assertions/script/audio errors: `coop-camera-visible-p1-revive-20261004.log`.
  Inspected shots 025 (shared scene) and 048 (separate owner views). This is
  the real manual camera API path in Reactor 1, not an exact Slosh-sequence
  replay; that exact playthrough still needs user confirmation.
- Debug/Release build and scoped whitespace check pass. Installed EXE/PDB/Bink
  after two closed-game checks; all three hashes verified. Backup
  `build/backups/pre-reactor-pads-camera-20261004-175210`; EXE SHA-256
  `0EAE902A0AC4A7A878AB4426588005EC81D940D8680E5F535DF702DCABE4E510`.
  Includes the previously staged I, Predator beam fix. Default Release output
  overrides restored. Native fixtures remain opt-in and use isolated saves.

## I, Predator bridge beam accepts one co-op Glitch (2026-10-04)

- User's beam screenshot and live Release log identify `WERMmorbot2` /
  `bridge1trig`. Active retail `xermstart01.sma` opens `fancy_trig` and
  `bridge_1`, then starts the authored grunt ambush. It is a local activator,
  not a transport or mission exit. Added only this exact world/name to the
  one-player co-op floor-switch rule; actual entering player is retained.
- Offline production gate fixture passes 485 checks, including all operators
  with 2-4 players, unchanged solo/PvP handling, and continued partner gather
  for Morbot 1 bridges and other Morbot 2 triggers. No radius change.
- User's existing game was running during the build. Stage the Release update
  under `build/pending-update/Release`; do not treat the current process as
  updated or interrupt its playthrough to replace the executable. Native bridge
  validation has not been run on this change yet. Release build passed; staged
  EXE SHA-256 `AC75EF05D9D9132DA91C3C5D7DB9285CD51D3723053A8938ABC48B8182ED2187`.
  Default CMake Release output overrides restored after staging.

## Morbot floor pads accept one co-op Glitch (2026-10-04)

- Active retail `WERMmorbot1` pads `circtrig1`, `circtrig2`, and `fliptrig`
  now release their authored enter event for one living original player, using
  that actual actor. This is an exact mission/name exception inside the co-op
  gate; solo/PvP, possession/named-objective filters, kill volumes, checkpoints,
  elevators, mission exits, and every other team gate keep their existing rules.
  No geometry/activation-radius change. Script `xemo_mbot.sma` confirms these
  three events only operate `leftdoor`, `rightdoor`, and `flipdoor` structures.
- Offline production gate/crossing/occupant fixture: 460 checks, covering every
  P1-P4 operator with 2-4 players, wrong mission, downed actors, solo/PvP and
  existing team gates. Debug/Release compile. Native four-player run verifies
  P2 alone moves both circle structures to 0.93 and flipdoor to 1.00 while
  partners remain at spawn, no wait HUD and no assertions/script/audio errors:
  `build/logs/coop-floor-pads1-enter-20261004.log`.
- Initial native fixtures did not exercise the switch: long intro movie, then
  incorrect box-only assumption (pads are spheres), and relocation disabled
  tripping. Corrected to skip the intro through native test keys and enter with
  the real tripwire sphere center and tripping enabled. Test-only warps seen by
  user are not spawn changes. Ordinary spawn code is untouched.
- Initial candidate names from unused `xemorbot2.sma` are absent in the active
  `WERMmorbot2` world/scripts. Removed those candidates; do not broaden a gate
  exception from unused assets. Native floor-pads2 candidate run failed to find
  them and is not a pass. No second-mission floor override remains.
- User's screenshots at 13:58/14:13 and old Release log ending 14:23 predate the
  render-color fix installed at 14:50. Recorded assertion stack is decal vertex
  color conversion (`fdx8math.inl:221`), matching the previous byte-clamp fix.
  Airship correlation is plausible, not established as its sole source.
  Production aim/color checks 660 pass. Native two-player `WERMmorbot2` test
  assigns attack goals to the three retail airships; repeated firing is recorded
  (`SWDMl2lfire`) with no assertion/script/audio errors, two fixture checks pass:
  `build/logs/coop-airship-combat-20261004.log`. Not a full mission playthrough.

- Installed normal Release EXE/PDB/Bink after two closed-game checks; all three
  copies hash-verified. Backup `build/backups/pre-floor-pads-20261004-153356`;
  EXE SHA-256 `8C974F6C696F39BFE7B25C00FF83EFE6CB47429681C5E4109DCE6869C468AA59`.
  Default CMake Release output paths restored.

## Shared Chase turrets and seated co-op gunners (2026-10-04)

- Wasteland Chase (`WEWHchase01`, `player_rat`) attaches up to three extra RAT
  turret pods to the original story vehicle. Each partner controls a separate
  gun/camera; the existing NPC driver/path and story passengers stay intact.
  Pods use vehicle-owned `fnew` lifetime, manual work after vehicle movement,
  and normal remove/destroy cleanup. No extra vehicles or competing AI routes.
- Whole-RAT aiming/projectile skip lists include extra turret and occupant
  meshes, so teammates' pods do not absorb shots. Co-op human RAT gunners
  (including the primary seat and other missions) attach with glue, zero entry
  velocity, and disable body collision while seated. Exit restores the previous
  collision flag and detaches normally, including removal from the world.
  NPC/solo/PvP seating paths retain their prior behavior; the RAT stays damageable.
- PC RAT pitch no longer applies a second inversion after player controls already
  apply the preference. Pitch uses profile sensitivity and is capped to the
  authored yaw speed; gunner camera follows actual gun elevation for both input
  types rather than independently adding height. Native feel still needs user
  confirmation with their controller/preferences.
- Repeated `fdx8math.inl:221` assertions came from decal float-color conversion,
  not square-root/collision code. PC render color bytes now clamp negative/NaN
  to zero and overbright/infinite values to 255 before unsigned conversion.
- Empty co-op shop offers give the shopper a brief "No items available for you"
  prompt; no inventory/stock change. User confirms this feedback works. User
  also confirms Level 1 still waits for partners after cached-player changes.
- Debug/Release build; production runtime guards 44,535 and aim/color 660 checks.
  Native four-player boarding/movement/body-collision/exit checks pass, followed
  by normal return-to-menu teardown without assertions. Native two-player
  checkpoint save/reload and restored exit pass (nine checks total), log
  `build/logs/coop-rat-seat-checkpoint2-final-20261004.log`.
  A first checkpoint fixture checked asynchronous saving too early; corrected
  to check on the following frame. Full mission completion not exercised here.

- Installed normal `build/Release/ma_port.exe` after two closed-game process
  checks; EXE/PDB/Bink copies verified by SHA-256. Backup
  `build/backups/pre-rat-seat-20261004-145006`; EXE hash
  `BEFB2B150955C97CB71893D90691591A44CA3BFE60E8DAB50340930886739201`.
  Restored default CMake Release output paths after staging.
- Empty-shop native fixture did not reproduce an empty inventory offer state:
  the shop opened, so its full-inventory assertion failed. Log
  `build/logs/coop-shop-empty1-final-20261004.log`; do not count it as a pass.
  User's actual no-offer prompt is confirmed; stock/offer rules remain unchanged.

## Vendor audio cleanup installed; Drill exit investigated (2026-10-04)

- User confirms unexpected vendor interaction was speech only, not shop entry.
  Authored encounter moves vendors to barter2; 135-unit attract radius is large.
  That track eventually stopped in the live log, so permanent sticking in that
  run is unproven. Co-op listener now follows occupied point, not inactive shops.
- Co-op audio stops/fades only named vendor tracks, releases mission-music pause,
  leaves replacement transmissions intact, and waits for speech before starting
  attraction. Shop preserves/pauses mission music rather than StopAllStreams.
  All behavior changes gated IsLocalCoop; solo/PvP retain previous paths.
- Production audio/listener fixture 19 checks; cinema 1,141, cached players 39,
  playthrough 328. Debug/Release pass. Native shop regression ten checks, actual
  outside-radius departure ends vendor tracks and restores mission music.
  First departure used start still inside 135-unit radius; fixture corrected.
- Drill run records PLAT FALL but no levelend entry or script errors. Fresh
  native co-op pipe_lift ride passes; lower (-46.557,-213.227,-298.695) raises
  48.3 units. Exit trigger (-44.156,-101.492,-439.523) requires walking the upper
  route; boss kill alone does not win. Fresh test does not establish the user's
  post-explosion checkpoint state or prove complete route traversal. No bypass.
- Installed after game closed and two process checks, backup
  `build/backups/pre-vendor-audio-20261004-042256`; EXE/PDB/Bink verified.
  Current EXE `EB08BFB0DF113B4AC66581DFEAA9D3A3A4B7D27427D9108518FA377BE007BF88`.
  See `docs/vendor-audio-drill-exit-20261004.md` for evidence and limitations.

## Cached story-player recovery: co-op only, installed (2026-10-04)

- You Know the Drill (`WEWRresrch4`) elevator setup retained P1's init-time
  handle, rejecting live P2 after P1 died. Retail audit found the same direct
  cached-player pattern in 147 script files (not 147 confirmed broken gates).
- `Bot_GetPlayer` tracks confirmed global assignments; before script work,
  those cells follow the living story actor. Active cutscene actor stays fixed;
  checkpoint handles supported, repurposed/null cells preserved, level reset.
  Registration/refresh strictly `IsLocalCoop`; solo/PvP unchanged.
- Retail production VM fixture 39 checks; cinema 1,141, gates 400, scene/lifts
  434, asylum 68. Debug/Release builds pass. Debug live native regression passes
  P1 death, surviving P2 encounter setup, real lift activation and ride to top.
  Details `docs/coop-cached-player-recovery-20261004.md`.
- User now authorizes live launch/close, superseding prior no-launch/kill rule,
  but explicitly forbids computer-use for this task. Use native commands and
  snapshots (`tools/port_run.py`, opt-in `port/coop_regression.cpp`) with isolated
  saves. Current tests close only their own processes; player saves untouched.
- Installed together with vendor fix after two closed-process checks; backup
  `build/backups/pre-coop-cached-player-20261004-033758`, all three hashes match.
  EXE at that update `B4DBF1900F6C121A2B2EFAF1B5CFF250FF47CFB4CDC02131B2F1DB5E75881ABF`.

## Co-op vendor camera isolation: installed and live-tested (2026-10-04)

- User reports P1 shop view mirrored onto P2 after cinematic-camera sharing.
  Shopping disables shopper controls; sharing misclassified that as a scene.
- `_CoopWatchCamerasWork` now rejects a source in CONTROLMODE_BARTERSYSTEM
  after clearing links, preserving private vendor camera, partner view/control,
  and ordinary scripted/letterbox camera sharing. No shop behavior changed.
- Production-method cinema/menu fixture 1,141 checks; Debug/Release pass.
  Native vendor gameplay test verifies independent views, partner controls/model,
  shopper-only hiding/purchase and exit. The first test fixture disabled P2
  before teleportation, leaving its mesh behind; fixture corrected and rerun.
  Details `docs/vendor-camera-isolation-20261004.md`.
- Installed with cached-player fix; staging output overrides reset.

## Krunk asylum exit: user-requested ten-kill goal installed (2026-10-04)

- `WEWRresrch3` retail script counted 13/14 deaths; two players possessed bots.
  User explicitly requests exit at ten kills while retaining fourteen spawns.
- PC `FScriptInst::Init` calls `port/pc_script_goals.h` after AMX expansion;
  exact script/instruction checks change only `EQ_C_PRI 14` operand to ten.
  Actual roster/spawns/death counter/exit camera and checkpoint remain authored.
  Applies solo and co-op. Restart full mission; old checkpoints past ten will
  not newly reach the equality comparison. Legacy debug marker still says 14.
- Real retail script + production AMX VM regression 68 checks, all 14 references
  preserved, door sequence at kill 10 exactly once. Debug/Release pass. No game
  launch. User now confirms ten-kill exit works in co-op gameplay.
  Details `docs/asylum-kill-goal-20261004.md`.
- Game closed during work; installed after two process checks, backed up to
  `build/backups/pre-asylum-goal-20261004-025122`, hashes match staging.
- Current Release EXE SHA256:
  `B747A5ADCBF889A3DBBE3B3A76F404BBAD0162366CD90A96A527CC5098E83FE6`.

## Search for Krunk booth cutscene stall: installed (2026-10-04)

- Live co-op room-alert scene stuck indefinitely, then felt unresponsive. Log
  `build/logs/booth-cutscene-stall-20261004.log` shows only three guard entries;
  `elite2` never entered `trigger_group`. Retail script waits for all four.
- Its waypoint lies only 1.57 feet inside the trigger; normal stopping/avoidance
  tolerances can complete outside. Narrow PC campaign `Bot_GotoE` correction
  aims that living/unpossessed guard inside the trigger during this scene only,
  keeping floor Y and normal camera/door/checkpoint events. Actual live guard
  path was not inspected; diagnosis is supported by authored geometry/AI behavior.
- Offline production-method fixture 7,241 checks; checkpoint 296/cinema 851;
  Debug/Release pass. Real replay/return of control remains unverified.
- Installed after game closed, with two process checks and matching EXE/PDB/Bink
  hashes. Backup `build/backups/pre-booth-arrival-20261004-021008`. No game launch.
- Current normal Release EXE SHA256:
  `30BEEC03C654FDD79A6C532D12FA10CD4E80F35A4A8105CC18FAEEEFC8C8B325`.
  Details: `docs/booth-arrival-20261004.md`. Replay scene to use new destination.

## Hold Your Ground mission-end crash: combined fixes installed (2026-10-04)

- User completed turret mission, got active-AI assertion during resource teardown,
  then full crash in level-complete entity fixups reading freed memory. Snapshot:
  `build/logs/hold-ground-teardown-20261004.log`. Added gun was not AUTODELETE;
  world cleanup skipped it, leaving AI/resources and list entries until frame release.
- `PortCreateDefenseGun` now sets AutoDelete after successful Create, matching
  authored world ownership. Normal DestroyAll handles removal/brain/resource teardown
  before AI shutdown. No assertion suppression or broad AI list clearing.
- New production-method teardown fixture 6,008 checks; strengthened defense 215,
  Debug/Release pass. Actual completion/retry gameplay remains unverified.
- User says game fully crashed and explicitly asks install. Confirmed closed twice,
  installed this plus tracker overflow and Fang allocator corrections below together.
  EXE/PDB/Bink hashes match staging; CMake overrides reset. No game launch/save changes.
- Normal `build/Release/ma_port.exe` SHA256:
  `A65212082F85E3BB5002E53A8973763744870C2FE257628A1D03765614670D1D`.
  Backup: `build/backups/pre-mil-assertions-20261004-012201`.
  Details: `docs/mil-assertions-20261004.md`.

## Mil Comm Centre pickup / co-op turret assertions (2026-10-04)

- Confirmed `WEWCcomm_03` stack: rising Titan loot repeatedly appends shared tracker
  exclusions without resetting count, overflowing in chaingun append. PC pickup
  collision now starts a fresh list, including falling/ownerless cases. Actual
  extracted production methods reproduce legacy overflow and pass 180,016 PC checks.
- Confirmed loading `WEWChold_01`: added turret helper uses plain `new`, forbidden
  by Fang aligned class allocator. Changed to `fnew` / `fdelete` with null check and
  matched rollback. Strengthened allocator fixture; defense 209 checks pass.
- User continued the loading assertion and confirmed both turrets work cleanly in
  the mission. Three/four-player gameplay and complete mission still unverified.
- Debug/staged Release pass; retail chip 19/32 and playthrough 328 checks pass.
  See `docs/mil-assertions-20261004.md`. No game launched/interrupted/save edits.
- Initially staged while PID 13572 was running. Subsequently installed together
  with the mission-end ownership fix above after the user reported full crash.

## Mil Comm Centre prison chip (2026-10-03)

- User reports no chip for ally cage panel in the Titan-gated mission. Confirmed saved log
  `WEWCcomm_02` / `prison2`: loot parse rejected six-field `chip 1 1 1 X safe` because source
  supports only four/five fields. Guard's loot is empty before combat. Solo/co-op affected.
- PC parser accepts known retail six-field form, validates X/safe extras, preserves normal
  quantities, resource notification, cloning/drop. No chip grant or mission gate bypass.
- Offline production parser/clone/drop: 19 legacy checks reproduce omission, 32 PC checks
  pass; campaign 328 pass, Debug/Release pass. See `docs/retail-prison-chip-20261003.md`.
  Mission must be reloaded to parse corrected loot. No game launch/save mutation.
- Installed with no game running in normal `build/Release`; verified EXE/PDB/Bink against
  staging, output overrides reset. Backup `build/backups/pre-retail-prison-chip-20261003-225534`.
  EXE SHA256: `FCFD25540AE0EBF210167A9C1DF5D6B24D4F7C92127B829BFE59435E6B703E86`.

## Freed They Live zombie allies (2026-10-03)

- User requests both freed caged zombies follow Glitch in solo and co-op. Retail bots have
  a wait job and script recruits them, but their class lacks recruitment eligibility/data port.
  Added a narrowly scoped script recruitment exception for these two world/name pairs, plus
  normal follow-thought assignment after shock and recovery after checkpoint/follow failure.
  Prefer P1, surviving partners while P1 dead, and current controlled bodies during possession.
- New offline fixture 132 checks; existing playthrough 328 pass. No game launched. See
  `docs/caged-zombie-follow-20261003.md`. Gameplay route/navigation remains unverified.
- Debug/Release builds pass. Game had closed, so installed this and the previously staged
  rollback/RAT camera/model/audio/collision follow-up together in normal `build/Release`.
  EXE/PDB/Bink hashes match staging; output overrides cleared. Backup:
  `build/backups/pre-cage-follow-rat-runtime-20261003-224426`.
  EXE SHA256: `56BB2FFB49A8F34624BAE8FA2A0604DB8EAC70ADE09D9A3723D3FCF552D6E329`.
- User confirmed earlier fence transparency fix in gameplay.

## Unexpected rollback and RAT runtime follow-up (2026-10-03)

- User reported level 10 checkpoint rollback while both alive/moving. Log slot 1 / notify 0
  matches the pause Respawn signature, but lacks caller evidence. Hardened pause confirmation
  ownership/lifecycle and stopped same-frame menu processing after exit/result. All production
  restore callers now log named reasons plus player state. Exact spontaneous trigger unproven.
- User confirmed RAT black surfaces look fixed. Additional reports: vertical still inverted,
  P2 looks like NPC in turret, loud jet-like hum at start (cleared near jet, then stayed clear
  on restart), RAT collision assertion that allowed continuing. Corrected controller gunner
  camera's independent vertical input and decorative NPC draw reactivation for human gunner.
- Positional gain refresh now handles stationary loops/listener switches/radius changes; unknown
  3D/no-listener gain is silent. Jet loop gain/position initialized at allocation; new logs added.
  Exact hum cause still requires gameplay confirmation. Stable PC sphere-edge math reproduces
  and fixes legacy invalid push values in offline stress tests (5 old, 0 revised / 120,000 sweeps).
- Debug/staged Release pass. New checks 44,535 runtime + 136 confirmation; checkpoint/RAT 296,
  scene 434, sling/texture 9,144, selection 6,147 pass. No game launch/save mutation. Live PID
  34964 remains running; update is STAGED in `build/pending-update/Release`, not installed.
  Release output overrides reset. See `docs/checkpoint-rat-runtime-20261003.md` for limitations.

## RAT, boss entry, slingshot and fences (2026-10-03)

- Installed combined update in normal `build/Release`, including the previously staged weapon
  visibility correction. Game was closed; no game launch or save/profile mutation. EXE/PDB/Bink
  hashes match staged copies; output overrides reset. Backup:
  `build/backups/pre-rat-spawn-sling-render-20261003-202116`.
  EXE SHA-256: `E24BE14A8FADD3E8B419B0A03F7A4EFEA9B1EB62A32A2A53659270741534F6A6`.
- Human RAT vertical aim reversed. Zombie boss intro's specific story-bot `glitchgoto` snap now
  repositions partners using floor/body/wall/occupancy checks after the scripted relocation.
  Existing load-time placement had run too early. Tests cover 2ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â€šÂ¬Ã…â€œ4 players and each lead index.
- PC slingshot releases no longer silently fail the retail release-speed threshold; transient
  stance gating retains held trigger samples. Draw power and normal throw/loading paths retained.
- Confirmed fence root cause: tolerant multipass depth allowed the surface pass to repaint black
  holes discarded by cutout lighting. Repeat alpha rejection for PC cutout surface variants.
  Actual arena fence alpha is intact; decoding was ruled out for that asset. A two-pass real GPU
  fixture reproduces old black holes and passes with the production surface alpha helper.
- Preserve combined reflection/lighting shaders across VB changes; fullbright uses baked instance
  colors. Corrected CMPR palette endpoint ordering and RGB5A3 alpha expansion independently.
  The simple shader helper's enum/handle fix concerns diagnostics, not gameplay's fast path.
- Debug/Release pass. New slingshot/texture/aim/shader fixture 9,144 checks; checkpoint/RAT 293;
  selector 6,147, waterfall 6,901, scene 434, cinema/UI 851, visibility 611, playthrough 328 pass.
  Full in-game coverage of RAT black surfaces, new entry placement and slingshot behavior is
  pending. See `docs/rat-spawn-sling-render-20261003.md`. Valve position correction remains open.

## Weapon visibility after Wasteland Journey valve (2026-10-03)

- User initially reported slower infinite-ammo Scatter Blaster/missing second shot, then clarified
  the upgraded weapon works fine. No cadence changes made. Audio silence was Windows mute,
  confirmed by user; no audio changes made.
- Weapons temporarily vanished from Glitch's arm after the Wasteland Journey valve, then recovered
  while swapping. User also reports a position correction/teleport when walking against the valve.
  Log records valve `switch1`/`start_running`, checkpoint 1 and earlier checkpoint 0 restore,
  without a weapon-render failure. Exact valve/visibility trigger remains unconfirmed.
- Found a concrete propagation defect in `CBotGlitch::_ChangeWeaponIndex`: outgoing weapon's
  hidden flag passed to the replacement even if Glitch was visible. Replacement now follows
  Glitch's visibility and force-refreshes entity/mesh drawing flags, retaining normal attachment,
  deployment, empty-hand and failed-asset behavior. Mismatches log `weapon visibility resync`.
  This guards a plausible persistence path; it does not prove the valve caused that state.
- `tools/test_weapon_visibility.py`: 611 offline checks reproduce old hidden propagation and
  cover P1-P4, both hands, repeated swaps, hidden owners, stale mesh flags and invalid slots.
  Selector/menu 6,147 and Scatter Blaster 8,347 checks pass. No game launch or save mutation.
  Valve position correction is still unresolved; switch actions do not explicitly relocate Glitch.
- Debug and staged Release builds passed; scene restore 434 checks pass too. Live game PID 29556
  remained running, so normal `build/Release` was not overwritten. New EXE/PDB/Bink are staged
  in `build/pending-update/Release`; output overrides have been reset. This was later included
  in the installed RAT/spawn/slingshot/rendering update above.

## Co-op cinematic views and join panels (2026-10-03)

- Co-op join panels now use screen fractions and thin pixel borders, eliminating width-scaled
  retail panel overlap. Text/controller/profile logic is retained; PvP keeps its retail meshes.
- Partners borrow the live scripted cinematic camera within their own split rectangles. Source
  pose/shake/lens/clip planes match, with horizontal framing adapted to each split's aspect.
  Normal on-foot/vehicle/turret camera controllers stay assigned and resume when the scene ends.
  No new player-body freezing or movement. Pause/teardown clears links and perspective pointers.
- See `docs/coop-cinema-ui-20261003.md`. New offline fixture passes 851 checks; scene restore 434,
  playthrough 328 and PvP join 145 also pass. No game launched; visual confirmation is pending.
- Debug/Release builds and 6,901 waterfall checks passed. Game was closed; installed the staged
  EXE/PDB/Bink in normal `build/Release`, verifying all three hashes. Previous binaries are in
  `build/backups/pre-coop-cinema-ui-20261003-173526`. Release output overrides were reset.
  EXE SHA-256: `A651ED8E6DDEB21A3D04B342B5EBF34FA092AB80DC1AA2BA342C7BC7093ACB2E`.
  PDB SHA-256: `F6848E3265F16E78E76431BC3C76496C8441D22957A3B3CFDEFA7699E07E043E`.

## Campaign playthrough update (2026-10-03)

- Latest user clarification: no possessable Titan at the blocked communications-centre entrance.
  Retail `WEWCcomm_02` gate `frontie` is Titan-cannon-only; scripts spawn Titans and assign movement
  goals, with no explicit gate-breach order found. Co-op now accepts damage credited to any player
  at this particular gate, preserving armor/health/destruction events and all other damage filters.
- Added shared radio HUD/antenna presentation with one audio owner, shared vendor introductions
  and recovery of encountered post-checkpoint vendors, neutral caged They Live zombies, checked
  initial partner placement, whole-RAT aim/fire exclusions, authored ending-movie playback and
  user-volume/headroom for movies. Wait prompt delay is 0.85 seconds; switch-parent hints work
  through nearby trigger children.
- Wasteland Journey's green sheet was explicitly invisible/harmless in retail data. Liquid entity
  parsing now honors those fields, follows moving parents, and checkpoints height/animation.
  Fixed render-plane one-past-array updates and centre relocation. Hidden liquids keep collision
  callbacks once per frame without ripples/splashes. Baked external-color meshes no longer select
  incompatible texture-lightmap shaders. Rendering still needs user visual confirmation.
- See `docs/coop-playthrough-20261003.md` for evidence, checks and unresolved hum/music/wire issues.
  Offline fixtures: playthrough 328, checkpoint/RAT/startup 200, gates 400, scene restore 434,
  waterfall 6,901, PvP join 145. Never launch the game unless the user reverses that instruction.
- Debug and staged Release builds passed. Scatter Blaster 8,347, cheats 413 and PC input checks
  also pass. Game was closed; installed EXE/PDB/Bink to normal `build/Release` and verified all
  hashes against staging. Backup `build/backups/pre-playthrough-20261003-165656`. Release output
  overrides reset. No game launched or profiles edited. Installed EXE SHA256:
  `161AC62F2C8216D443839A6533473FBFE30A5338BB1DB92166324C62CAC87907`.

## PvP join routing after co-op (2026-10-03)

- User reports returning from co-op, choosing PvP, and P2's first XInput controller driving P1's
  slot rather than being able to join independently. They request installing both this fix and
  the previously staged Scatter Blaster fix. Do not launch game; user tests.
- Startup/main-menu reset correctly restored configured SHARED layout; PvP entry did not create
  a multiplayer routing session, so keyboard/mouse plus controller 1 were merged into port 0.
- `wpr_system.cpp::_PcBeginLocalJoinRouting` now starts fresh AUTO routing for both local join
  modes. Co-op selects campaign join flags, PvP explicitly clears join/launch flags. With fewer
  than four pads, keyboard/mouse stays in slot 1 and controllers take independent slots 2-4;
  four pads fill all four slots. Existing controller-only slot/profile IDs remain intact.
- PvP retains that mapping through rules/level selection and gameplay; does not compact ports or
  reassign an already joined controller. Back from PvP join restores configured input layout,
  as startup reset and co-op Back already do. This input setting does not select co-op game rules.
- `python tools/test_pvp_join_routing.py` executes production menu entry, join-section binding,
  forward/back decisions and AUTO assignment with fake XInput transport: 145 checks pass,
  including repeated co-op-to-PvP switches, all 16 pad-connectivity masks, stable late join/
  disconnect/reconnect, configured shared/separate reset, and distinct PvP/co-op forward routes.
  Scatter Blaster 8,347, cheats 413 and existing selector/menu 6,147 checks also pass offline.
- Debug/Release builds and `ma_input_tests` pass. Game closed; combined Scatter Blaster + PvP
  update installed to normal `build/Release`, EXE/PDB/Bink hashes match staged files. No game
  launched. EXE SHA256 `964D4138A22FFBA6B28CA3C5318679F13AC941E1159DDD11ADAF35B5FF1450D3`.
  Previous live binaries: `build/backups/pre-pvp-blaster-20261003-142445`. CMake Release output
  overrides reset. Profile/save files untouched.
- User playtest: return from co-op, choose PvP, keyboard P1 joins and controller P2 joins in its
  own box; start a match and confirm separate controls; Back and re-enter each local mode.

## Scatter Blaster infinite-ammo crash (2026-10-03)

- User reports co-op crash immediately firing Scatter Blaster, confirms P1 with infinite ammo on.
  Do not launch game; P1/P2 gameplay testing is user-run. Running normal Release was detected;
  do not stop it or overwrite its EXE/PDB. The saved Release log did not contain this crash stack.
- `CWeapon::RemoveFromClip` intentionally retains finite clip counts under the per-player cheat.
  Both `_L1_Fire` and `_L23_Fire` then unconditionally removed shell mesh `[GetClipAmmo()]`.
  On a full clip, count equals array capacity, so this dereferences one past allocated CFWorldMesh
  storage. Offline checked storage reproduces this exact defect for P1-P4 and different capacities.
- `weapon_blaster.cpp/.h` now use `_ConsumeFiredRound` for both fire paths. Only remove a shell
  if ammo actually decreases and the resulting index is within clip capacity. Infinite ammo
  retains visible shells; finite shots continue removing the consumed shell. Pellet/effect/audio/
  feedback/cadence remain unchanged. This also guards the engine infinite-ammo sentinel index.
- `python tools/test_blaster_ammo.py` executes production fire methods and RemoveFromClip against
  checked shell arrays: 8,347 checks pass, including original full-clip reproduction, partial/full
  cheat clips, all variants, separate player cheat state, dual barrel shots, buddy fire, camera and
  rumble. `tools/test_pc_cheats.py`: 413 checks pass. No game launch or profile/save mutation.
- Initially staged while game PID 20716 was running. Now installed with the PvP join fix above
  after game closed; normal Release includes both fixes and all prior co-op work. No game launch.
- Gameplay confirmation still required: fire L1 and upgraded Scatter Blaster with infinite ammo
  on/off, with P1 and P2. If another crash occurs, retain its new log/stack for separate diagnosis.

## Hold Your Ground co-op defense attempt (2026-10-02)

- User explicitly authorized attempting one AA gun per player for Hold Your Ground (WEWChold_01),
  keeping one set of enemy waves and shared success/failure. Do not launch the game; user playtests.
- `MG_HoldYourGround.cpp` creates P2-P4 guns at POST_ENTITY_FIXUP, before checkpoint 0 is saved.
  P1 retains `biggun` and the retail walk/jump intro. Extra guns alternate along its right axis at
  +16, -16, +32 world units, retaining original height and facing. **Placement is provisional**:
  terrain support, platform fit, obstruction and bridge sightlines need a real playtest.
- `botAAgun.cpp/.h` adds PC-only clone/configuration and instant assigned-seat helpers. Each gun
  copies original aim limits/rates, mortar attachment, armor/health, invincibility and team, with
  independent controls, reticle and camera. Camera entry/exit starts from that player's camera,
  fixing the former global-active-camera assumption. Creation failure rolls back extra guns.
- Intro selects P1/body/HUD explicitly even when P2/P4 was worked last. Partners' controls are
  disabled during the intro and restored once after letterbox ends, before automatic boarding.
  Waves wait for every assigned driver's seat and camera transition. Exits remain locked as in
  retail. A breach disables aiming/reticles on all guns; checkpoint restore resets the intro,
  and ordinary entity checkpoint restoration restores all guns before reseating the team.
- This is a participation attempt, not a verified map edit. Enemy wave timing/balance are unchanged;
  extra guns add firepower. No extra static foundations were added. P3/P4 use the same logic and
  have offline coverage, but all player counts still need placement/gameplay confirmation.
- `python tools/test_coop_defense.py` executes production clone/boarding/readiness/intro/failure/
  restore/camera helpers against engine fixtures: 189 checks pass. Existing checkpoint/RAT (178),
  gates (400), scenes (434) pass; Debug and staged Release builds pass. No game launched.
- Game was closed; staged EXE/PDB/Bink were installed to normal `build/Release` with matching hashes.
  EXE SHA256 `2A0F515BF3171B0DCAC00F573C87A2F247526C58E8147319BE821F946593CFC5`.
  Previous binaries: `build/backups/pre-coop-defense-20261002-215825`. CMake Release output
  overrides were reset. No launch or profile/save modification performed.
- Manual checks: enter this mission in 2-player co-op; both seated after intro, independently aim
  and fire both weapons, pause/resume, let a RAT breach and confirm both guns reset/reseat, then
  complete the shared waves. Inspect gun bases/line of fire before treating placement as final.

## Checkpoint revival / shared RAT and mission timer (2026-10-02)

- User on Nuts of Steel lost one player and continued expecting a revive. The latest Release log
  includes Clean Up (`WEDTtown01`): `townzone.sma` reaches source line 552 (Next Zone 3), directly
  after its line 551 `Checkpoint_Save` call, without a completed checkpoint save. The compiled
  townzone script has no difficulty query; this request is not disabled by hardest difficulty.
- Safe revive queries used `~ENTITY_BIT_BOT` to exclude bots. Fang's tracker prefilter accepts any
  matching bit, so Glitch's additional subclass bits still admitted his mesh. A body-sphere sweep
  starting at the survivor could therefore reject every offset, including the final same-position
  fallback. Queries now explicitly reject bot meshes, their owned weapons, vehicle-only props and
  detpacks through a callback, matching the relevant normal movement exclusions. Floor, surface
  damage, body clearance, height and wall-crossing checks remain enforced. Save requests and the
  first safe-revival defer/completion now log without per-frame spam.
- User requests P2 automatically start in the RAT turret, with manual boarding as fallback.
  Wasteland Thunder's `rat_glitch` has `ZobbyGunning`; route/end triggers name that exact RAT.
  In local co-op racing missions the friendly scripted gunner object stays allocated for script
  references but is removed from the world and auto-work, reserving that seat for a player.
  Once a human driver is present and the scene/body transition is finished, the first eligible
  other Glitch boards immediately through the existing gunner controls and camera path. P2 is
  normally selected beside P1; reverse roles are supported. Initial boarding flags use the already
  saved RAT flag word. Manual first boarding consumes the automatic offer, and voluntary exit is
  not undone. Enemy gunners, non-racing NPCs and solo play retain their existing behavior.
- Boarding/reboarding a second human preserves shared vehicle health and driver input ownership.
  Script mission timers now bind the same timer to every co-op HUD and maintain flags across
  vehicle modes; show/hide and checkpoint restore no longer use the last-worked player's HUD only.
- `tools/test_coop_checkpoint_rat.py` passes 178 offline checks, including reproducing the original
  safe-ground rejection, hazard/void/wall protection, pending-save versus wipe priority, initial
  and manual RAT boarding, solo/enemy exclusions, health/driver ownership and P1-P4 timer sharing.
  Existing gate (400) and scene/lift/liquid (434) checks and Debug/Release builds pass. The game was
  closed; staged EXE/PDB/Bink were copied to normal Release and hashes verified. Previous binaries
  are under `build/backups/pre-coop-rat-checkpoint`. Live EXE SHA256 is
  `F33DB42CD2F58A40B150DBC2BE9BC81A309A7672F8540298F93C765C36117A45`. No game
  launched. Gameplay confirmation for RAT controls, restart/reboarding and the Clean Up revive is
  still required. This supplies one driver/gunner pair; P3/P4 vehicle capacity, other single-seat
  vehicle missions and turret-defense mission participation remain separate work.

## Nearby waterfalls / particle camera handoff (2026-10-02)

- User screenshot confirms the Seal the Mines cave fall is correctly positioned at distance,
  but shifts toward P2's camera while the splash/mist remains at its world anchor. The earlier
  liquid-mesh fixes did not resolve this case. Active log identifies `WEDMmines03`; an offline
  read of its world-init tables finds waterfall splash/top particles and liquid volumes, but
  no LiquidMesh entities. These falls therefore also require checking transparent world geo.
- Windows `CFPSpriteGroup::_RenderEmulatedGroup` sets the graphics view to identity to draw
  CPU-transformed, view-space quads, but failed to restore it. Its Xbox counterpart already
  restores the view. Approaching an emitter can switch from hardware point sprites to this
  emulated path. Sorted transparent world meshes then compute their projection using that
  identity camera; a later ordinary actor draw can restore the view, making recurrence depend
  on the translucent draw order. Renderer switching alone does not restore these matrices.
- Windows emulated particle draws now restore the active view, including its mirror transform.
  `fxfm_SetViewAndWorldSpaceModelMatrices` also refreshes the view before shader constants and
  resets the fixed-function world matrix, so world geometry cannot inherit screen-space or
  actor transforms. Particle positions, world assets and collision data are unchanged.
- `tools/test_particle_camera.py` runs the full production Windows emulated draw and production
  DX matrix helpers with the port's actual D3DX math. It failed on the original missing camera
  restore, then passed 2,592 checks covering P1-P4 camera movement, near/far path selection,
  world-anchor screen projection, mirrored views, ring-buffer wrapping, behind-camera and
  empty draws. Existing waterfall (6,901) and scene/lift/liquid (434) checks pass. Debug and
  staged Release builds pass. With the user's game closed, normal Release EXE/PDB/Bink were
  synchronized with the staged build and hashes verified. No game launched. The user subsequently
  confirmed the nearby waterfall displacement was fixed in gameplay.

## Weapon gestures and main-menu crash follow-up (2026-10-02)

- User reports repeated selector attempts while moving, occasional hold-delay bypass, wants
  double-tap weapon/grenade cycling, and reports pause Quit still closes the app. The latest
  normal Release log confirms `PC: returning from the game to the main menu`, wrapper reload,
  then an access violation in `_DXSetTexture` from `_PcMenuArtLabel`; this was a return-menu
  draw crash rather than the pause handler requesting application exit.
- The PC menu label texture was a static instance retaining its first wrapper-frame atlas after
  that frame was released. `_ResetSystem` now clears both the texture pointer and load-attempt
  flag before freeing the frame, so each menu visit loads the new atlas. Pause return continues
  through the existing retail front end; main-menu Close Game retains application exit.
- PC selection input now tracks each player's two buttons independently using actual press/release
  edges. Single taps use the retail reload/secondary-target-clear callback without starting the
  selector. Two completed taps within 0.3 seconds cycle to the next available weapon/throwable,
  wrapping and skipping empty slots or missing runtime assets. Normal inventory callbacks still
  own equip animations. Borrowed bots retain reload-only behavior and cannot quick-cycle.
- Only a fresh continuous 0.3-second hold opens the list. Repeat latches cannot rearm it; elapsed
  time before a new press cannot count as a hold. Death, body/controller changes, loss of focus,
  pause/barter/cutscene modes, level loads and checkpoint restores clear pending gestures. The
  HUD no longer reopens a closing selector or hands off to the other side from raw held input.
  Releasing the button drops queued movement scrolls, finishing only the current scroll.
- `tools/test_weapon_select_menu.py` passes 6,147 offline production-code checks, including P1-P4
  tap/hold timing, wrap/rejected inventory switches, released movement scrolling and 40 wrapper
  atlas lifetimes. Existing scene fixture passes 434 checks; input mapping checks pass. Debug and
  normal Release builds pass. No game was launched, and gameplay confirmation remains pending.
- The old input test still isolated the former LOCALAPPDATA settings path, so its first run touched
  the live sensitivity preference. The original 0.100000 value was restored; the harness now sets
  MA_PORT_SAVE_DIR to its disposable folder. Its passing rerun left the live settings hash unchanged.
  Invalid explicit co-op layouts again fall back to shared routing; supported layouts are unchanged.
- No game process remained during this build. Normal `build/Release` and `build/pending-update/Release`
  are synchronized to the complete update, including the previously staged waterfall changes.

## Waterfall animation and draw follow-up (2026-10-02)

- User reports a waterfall graphic briefly sticking to the camera center and falls disappearing
  when viewed head-on. Their normal Release process was running; it was not stopped or replaced,
  and no game was launched. Normal Release's executable hash remained unchanged during the build.
- `CFLiquidMesh::EvalFunc` evaluated `(x-1)^exponent`, where animated exponents are fractional.
  That generated NaN world positions for most of the waterfall grid. The curve now uses the
  positive normalized distance `(1-x)`, clamping endpoint rounding to [0,1]. Existing even-integer
  curvature shapes are preserved. Waterfall shape/UV work now runs once per frame, rather than
  advancing the shared mesh again for each split-screen viewport.
- `LiquidFallVtx` contains three UV pairs (36 bytes), while the waterfall reflection shader and
  molten FVF passes requested four (44 bytes). Windows now has a separate three-UV declaration
  using the existing planar-reflection bytecode; molten passes use TEX3. Pool quad declarations
  retain TEX4. Appending the shader entry preserves all existing shader indices.
- Every waterfall draw explicitly enables depth testing with LESSEQUAL and disables stale alpha
  testing, instead of inheriting ALWAYS/EQUAL/alpha-test state from fog or earlier world effects.
  Both sides remain visible. These changes address concrete geometry/render-state defects; the
  reported camera attachment and head-on disappearance still require visual confirmation.
- `tools/test_waterfall_render.py` passes 6,901 offline checks, executing 544 complete reflective,
  textured and molten draw passes. It checks finite animated grids, split-screen timing, vertex
  strides, shader enum/table alignment, indices and depth/alpha state under tainted prior state.
  Existing scene/lift/liquid fixture still passes 434 checks. Debug and staged Release builds pass.
- The complete new EXE/PDB/Bink DLL are in `build/pending-update/Release`. Release was redirected
  there using temporary CMake runtime/PDB output settings; those settings were removed afterward.
  Apply the staged files to normal Release only after the running game closes. The staged build
  includes all previous co-op fixes and Cheats; normal Release still has the preceding build.

## Grounded scenes, lift restores and recurring square follow-up (2026-10-02)

- User corrected their remembered elevator mission as uncertain; the latest normal Release log
  is Seal the Mines, with `bradys_attack` and `jailelev01` releasing between checkpoint restores.
  It does not prove why the elevator failed. No game was launched during this investigation.
- PC doors/lifts now use the mesh base checkpoint save/restore, retaining animation clocks,
  speed, pause flags and selected mesh. Restore rebuilds the saved line/bone/animation pose for
  stationary and moving snapshots, including same-endpoint restores, without discarding saved
  pickup/open timers via `SnapToPos`. Moving loops resume in the saved direction without arrival
  sounds. Checkpoint requests and restored lift state/position/lock/timers are logged for the
  next playthrough. These are in-memory checkpoints; existing profile files are not migrated.
  The reported elevator failure is not yet verified as resolved in gameplay.
- `CBotGlitch::PortWorkForCoopScene` runs neutral-control spectator physics and animation until
  a real floor contact (not airborne/jumping/cable). Grounded stationary spectators retain X/Z
  and orientation but keep floor correction in Y. Sticky platforms and parents own all axes;
  spectators are not relocated back to their previous world position on a moving lift. Scene
  actors/AI/vehicles remain on their existing update paths. Terminal exit parking is unchanged.
- Waiting HUD text now requires 0.35 seconds of continuous waiting, per player. Leaving the gate,
  release, scene entry, body changes and load/restore clear the display timer. This only removes
  brief flashes; shared-event release and terminal parking still use actual arrivals.
- The user's screenshots confirm the moving square persisted after the previous liquid patch.
  That patch reset shader constants but omitted fixed-function world transforms. Liquid planes,
  waterfall entry and waterfall glow scales now set the appropriate fixed-function matrix too,
  so FVF passes cannot inherit the preceding droid's matrix. No collision behavior was changed;
  visual recurrence and the reported physical interaction still require a manual check.
- Window startup stores an explicit Windows arrow instead of `GetCursor()` (which could capture
  the app-starting spinner). `WM_SETCURSOR` also explicitly selects the arrow for an unfocused
  client area, even if the custom menu-pointer flag was retained.
- `tools/test_coop_gate_recovery.py` passes 400 checks across P1-P4, including wait debounce and
  clock rewind; `tools/test_coop_scene_restore.py` passes 434 production-method fixture checks
  for grounding/platforms, checkpoint layouts/poses/directions/timers and liquid entry transforms.
  Fixtures do not start the game or touch profiles. Debug/Release builds pass. Normal Release and
  `build/pending-update/Release` contain this update; gameplay and cursor appearance are pending.

## Pause-menu campaign cheats (2026-10-02)

- PC Pause > Cheats has per-player infinite ammo and invulnerability toggles, refill ammo,
  +1,000 washers, give available weapons, max weapon upgrades and heal. The title identifies
  the player who paused; keyboard, controller and mouse use the existing menu input paths.
  Back returns to Options. The expanded Options list keeps Quit to Main Menu as its last entry.
- `port/pc_cheats.cpp` owns the session masks. Infinite ammo skips ordinary clip/reserve
  consumption and refills to normal caps before world work, including the player's possessed
  bot. It does not save a new infinite-ammo sentinel. Invulnerability extends `IsInvincible`
  for that player's current/original bodies without rewriting native protection flags.
  Scripted deaths and kill volumes retain their normal behavior. Toggles survive checkpoint
  and mission transitions within the session and reset on return to the main menu; they are
  not serialized. Granted washers/weapons/upgrades use normal inventory/checkpoint saving.
- Washers go to `CPlayer::GetInventory(player)` even during possession, capped at 32767.
  Healing acts on the live current body and refuses removed/dead bodies; it does not revive.
  Weapon grants/upgrades require the original live Glitch. Grants use the existing prepared
  level pickup pools, skip already-owned guns (retaining upgrades), and report partial
  availability when assets are missing. Removed Nuke/Water Grenades are excluded.
- Direct `CCollectable::GiveWeaponToPlayer` grants now scope and restore pickup bot/HUD/player
  context, instead of inheriting the last world pickup. Missing guns are checked for inventory
  space and an unused prepared weapon before the retail grant indexes its temporary pool.
- `python tools/test_pc_cheats.py` passes 413 offline checks using production cheat, ammo,
  invincibility and grant-context methods across P1-P4. Asset loading, UI rendering and actual
  gameplay still need a manual check. No game was launched under the user's standing instruction.
  Debug and normal Release builds pass; `build/pending-update/Release` is synchronized to the
  new Release EXE/PDB/Bink DLL so a later fallback cannot overwrite it with an older build.

## Progression and profile follow-up (2026-10-02)

- **Checkpoint/gate recovery:** recognized retail checkpoint trigger families (`saveNN`,
  `checkNN`, numbered checkpoint names, and the additional Mines checkpoint names) now fire
  their normal script enter for the first living original player, including P2/P3/P4. Ground-safe
  revival and deferred checkpoint saves remain in `checkpoint_Work`; this does not synthesize
  objective completions. Other shared gates retain their team wait. PC box triggers now detect
  complete crossings between frames, matching sphere triggers; kill-volume tests are unchanged.
  Held gates also recheck the team's actual movement once per frame, recovering missing enters.
  Crossing history is cleared on restore/load and excludes dead bodies, possession changes and
  movement over 10 world units between samples. `_EntityIsOutsideTripwire` compared an occupant
  using assignment; it now removes the correct entity, preserving other players' membership.
  `python tools/test_coop_gate_recovery.py` passes 400 offline checks across 2-4 players, including
  all occupant departure orders, checkpoint-first-player behavior and recovered terminal crossings.
  Special checkpoint names outside the audited families remain playthrough checks.
- **Moving square investigation:** the old Mines 2 session had no liquid diagnostics, so its log
  cannot identify the square or establish its reported collision. `fsh_DrawLiquidMesh` did inherit
  the preceding draw's world/skinning state despite CPU world-space vertices, and its first molten
  pass inherited the vertex shader. It now establishes identity world/current view, disables
  vertex blending, selects the molten vertex layout before drawing, and invalidates the selected
  vertex buffer after DrawIndexedPrimitiveUP. This fits a surface moving with successive droids;
  visual and physical behavior still require a manual Mines 2 check. No physics was changed for it.
- **Live update:** after the user closed their softlocked session and authorized applying the
  prior patches, the normal `build/Release` EXE/PDB/Bink DLL were updated. The checkpoint/gate
  recovery and liquid draw changes also build in Debug/Release. No game was launched. The later
  true Mines 2 wait involved P1-only `bradys_attack` and `trigger_btr01` records; the HUD-only
  change below did not itself resolve those logical waits. Retest them with this build.
- **Stuck P2 waiting HUD:** the user could still play normally after the waiting message flashed
  and stayed on P2's screen. Mines 1's log shows P2 entering `goagain2`, then both players proceeding
  through `goagain` and later checkpoints; `goagain2` never received P1. The HUD treated that sticky
  arrival as a current wait anywhere in the level. `CoopTripwireWaiting` now requires a live, armed,
  unresolved trigger containing the player. Sticky arrivals still release thin gates in any order,
  and deliberately parked terminal arrivals keep their wait indication. Debug and staged Release
  builds pass, along with eight offline HUD-state checks. The updated EXE is also in
  `build/pending-update/Release`; at that time the active Release session was left untouched. Gameplay
  verification of this HUD fix is pending. The user also reported odd ally movements around successive bot pairs;
  their cause has not been established and no AI behavior was changed for this report.
- **Pause-menu return:** the PC pause option is now labeled `Quit to Main Menu`. After the existing
  confirmation/settings-save step it schedules the retail front end and stops handling pause
  input for that frame. PC `LAUNCHER_FROM_GAME` always returns to the retail menu, including direct
  `-mission`/`-level` launches that previously returned to the development picker. The main-menu
  Quit option still exits the application. Debug and Release builds pass; gameplay verification
  is pending. The user's Release process was still running, so its EXE/PDB were left untouched.
  The updated Release EXE, PDB and Bink DLL were staged in `build/pending-update/Release` and have
  since been applied to the normal Release location after the user's game closed.
  No game was launched to verify this change.
- **Mines 2 end-cinematic softlock:** the user finished with P1 dead and P2 alive. The world kept
  updating, but `xedm_end` logged `Bot_FaceE` at line 143 and `Bot_GotoE` at line 233 against
  `Player0 not in world`. The script refreshed `Bot_GetPlayer` immediately before beginning the
  scene, then waited for that dead actor's movement-complete event. `game_GetStoryPlayerIndex`
  now prefers living P1, otherwise the first living player, and pins that player for the whole
  cutscene. Script player lookup, AI activation/deactivation, fall actions and camera targets use
  the same actor. A checkpoint revival cannot change actors halfway through the scene. Scripts
  that only cache a player handle at level initialization still need checking during progression.
  The user has since confirmed a cutscene progressed with P1 dead: P2 temporarily served as
  the story actor. This verifies that reported fallback scenario in gameplay.
- **Cutscene spectators:** living spectators run Glitch's work with neutral controls, landing
  before being parked and retaining moving-platform displacement. The script
  actor keeps normal AI movement and passes scene triggers without waiting for spectators whose
  controls are disabled. Downed viewers follow the active scene camera; waiting-for-checkpoint
  text is hidden during the scene. Retail camera animations still occupy P1's camera slot and
  return that slot to P1's body when they end.
- **Mines 1 zipline exit:** the user's log showed `levelend` retaining P1's arrival after a checkpoint
  restore. Restore now clears co-op arrival records. A player waiting at the common retail terminal
  triggers `levelend`, `levelend1`, `levelend2` or `endlevel` releases the cable and parks in place,
  avoiding movement into floorless geometry and timed fall recovery while partners catch up.
  Ordinary checkpoint and combat gates remain movable; custom terminal names need playthrough checks.
  The user confirmed the new zipline waiting gate worked in their active run on October 2.
- **False Reset screen:** a fresh profile's difficulty screen used retail row 33, which is the
  format-confirmation layout, because source and retail screen enums diverge after row 31.
  `wpr_system::_Init` recognizes the retail front-end table and maps difficulty to row 32, format
  confirmation to row 33 and the missing rename-space error to retail create-name-space row 35.
  Source-order and pause tables retain their mapping. Enter started the level because the handlers
  were still those of the difficulty screen; this was not a save deletion. The PC format function
  only reports that formatting is unavailable.
- **Profile audit:** current `blizzard` and fresh `co-op` profiles passed signature/version/CRC
  validation. Current and legacy saves were copied, without modifying them, into
  `build/backups/profile-audit-20261002-112745` with a SHA manifest. Old logs identify the missing
  test profile as `Profile1&&&`, last saved at level 3, but its file was not found. The user accepts
  that they may have deleted it and does not require further recovery work.
- **Validation:** final Debug and Release builds pass. Eight offline checks of the production actor
  selector pass; all 35 front-end and 35 pause-screen mappings resolve their retail assets.
  These changes have not been tested in gameplay. The user independently ran and closed the latest
  Release session; no game was launched by the agent. Keep the user's no-launch instruction until
  they authorize testing. Next manual checks: Mines 2's ending with P1 dead, the same ending with
  P1 alive and animated spectators, Mines 1's staggered zipline arrivals/checkpoint rollback,
  and creating a fresh profile to verify the actual difficulty screen.

## Finishing pass (2026-09-30)

- **Earlier user verification:** co-op saving/reloading and ordinary deaths/checkpoint revives worked
  with the vanilla animation. A later airborne-checkpoint softlock is described below; death/revival
  is not yet fully verified. Co-op cutscenes have no letterbox, and partners' cameras
  lock onto player 1 during player 1's cutscene. Alt-tab and using other applications while the game
  runs caused no observed issues. The user reports smooth play and no specific audio issues.
- **Remaining manual checks:** controller hot-swapping, a full campaign playthrough for progression
  softlocks, and additional liquid surfaces. Fullscreen switching and live resizing are not covered
  by the alt-tab report. Liquid candidates: the second Mines 1 lava pool, water in `WECRruins01`,
  `WERMmorbot1` and `WERRreactr1`, mercury in `WEDMmines02`, and slime in `WEWJjourn01`.
- **Experimental Nuke/Water Grenades removed at the user's request:** restore ordinary Coring Charge
  and EMP pickup placement, item registration and projectile behaviour; remove the experimental
  projectile pools, effects and `-cut-content` / `-cut-enemies` switch. Keep the general `-test-give`
  aid and the separate `-coop-hold` test switch. Existing saved experimental grenade names restore
  as Coring Charges/EMP Grenades, retaining ammo; retail item identities do not change. Do not revive
  these asset-only weapons again.
- **Possession objective fixed:** `WEDMmines02` (Seal the Mines), `xedm_vats`'s `gruntdeath` trigger
  must receive the actual borrowed grunt, without waiting for original player bodies to gather.
  `_CoopHoldTripwireEnter` now bypasses borrowed bots and named entity filters; ordinary shared
  progression gates retain their gathering rule. Both players completed the objective through the
  console and returned alive to their bodies; the user confirmed both work. The first diagnostic
  moved the grunt into the trigger one second after direct possession, causing unnatural animation
  overlap. The normal test now waits for the cutscene/body transition, uses the console interaction,
  and lets possession finish before entering the objective.
- **Possession keyboard HUD:** remove the redundant large Q keycap and blue box; the existing
  "Hold Q to exit" text remains. Controller glyphs remain device-specific.
- **Water fixed and user-confirmed:** Windows renders the existing water/mercury cube target and
  uses complete planar-reflection shaders with a real clip-space W and cube-reflection pixel shader.
  Restore the camera transform after the target pass, and enable depth testing for liquid planes.
  Liquid work updates the selected active volume once per frame rather than a different entry in
  the full array. `WEDMmines03` (Seal the Deal) cave pools draw in both viewports. The intermittent
  dark blue circle was not directly reproduced; projection/depth fixes address a possible source,
  but recurrence still needs manual checking. Diagnostic masks are cached per viewport to avoid
  alternating split-screen log spam.
- **Co-op shop:** either player can use Shady/Slim, one shopper at a time. That player's controls,
  camera, UI, goods and washer wallet own the transaction; the partner keeps the normal control mode.
  Empty/unavailable stock no longer traps controls. Fix the four-vertex purchase button overrun,
  initialize the washer HUD viewport before a same-frame reveal, and reserve display meshes for
  world pickups, player HUDs and the shop. Mesh exhaustion retries instead of asserting. Unsupported
  retail EUK-only/battery-upgrade catalog entries are omitted rather than displayed as the unrelated
  `goffhead` custom pickup; ordinary supported weapon levels and ammunition remain available.
- **Personal world weapons:** every player can collect each world weapon pickup once. It disappears
  from the collector's viewport while remaining for partners, and retires/respawns after everyone
  collects it. Checkpoints save partial claims. Full-ammo players cannot block partners, and the pool
  does not recycle outstanding personal weapons. Paid/scripted `GiveToPlayer` grants remain exclusive
  to their recipient, including when that recipient is currently possessing a different bot.
  Washers use the already separate inventories: each physical washer credits its collector, and
  purchases deduct only the shopper's funds. Checkpoints preserve both balances.
- **Discord:** mission details remain unchanged; multi-player campaigns set the activity state to
  `CO-OP Campaign (N players)`, including checkpoint reloads. Normal play keeps Discord on; isolated
  diagnostic runs turn it off. The presence log was checked; Discord's client display was not.
- **Airborne checkpoint softlock (latest report):** P2 died in the untimed `WEDMmines01` session and
  revived underneath the playable route, blocking P1's next gathering gate. The log shows `save03`
  firing while P1 was airborne, followed by `revived at the checkpoint, with no partner to stand
  beside`; P2 remained at the death location near `(681,-317,40)` while P1 was near `(778,-151,64)`.
  Checkpoints are invisible script triggers, so this looked like a revival before any checkpoint.
  `CoopReviveForCheckpoint` now returns FALSE until safe placement exists, leaving the save pending.
  Search grounded, living partners for a walkable non-damaging floor, body clearance and a clear path;
  reject lower floors/outside-world anchors. Prefer separate space for each player, with the partner's
  checked position as the narrow-platform fallback. Never resurrect at the corpse's position when
  placement fails. Finish death/possession transitions before saving, reset recovery timers, and
  exclude dead/dying bots from automatic fall recovery. A requested restore takes precedence over
  the pending save, so a team wipe can restore the previous checkpoint.
  Release and Debug builds pass. Runtime verification is PENDING: the user explicitly said not to
  launch the game while they are running other things. Do not launch any play/diagnostic session
  until they authorize resuming. A new opt-in `revive` case covers a downed player below the route,
  a checkpoint while the survivor is airborne, landing, personal wallets and checkpoint restoration;
  it is compiled but has not been run (`--cases revive --config Debug` when authorized).
- **Repeatable checks:** `python tools/test_coop_polish.py` runs opt-in retail fixtures for pickups,
  primary weapons, wallets, retired-save migration, either player's shop, normal console possession
  and rapid borrowed-bot death. Add `--config Debug` for engine checks, or
  `--players 4 --cases pickups weapons` for four-player pickup ownership. Reports go to
  `build/logs/coop-polish-*.json`; saves stay in `build/test-saves`, audio is on, and the runner closes
  only its own process. Fixtures require `MA_PORT_TEST_COOP_POLISH`; ordinary play never runs them.
  In this mode, assertions log a failure and abort instead of leaving a modal dialog.
  Three/four-player ownership fixtures skip the reconnect overlay because the fixture moves those
  bots without hardware; these checks do not validate physical controller routing or hot-swapping.
- **Verification results before the new revival fix:** Release fixtures passed 50/50 checks across ten cases. Four-player
  weapon/Coring Charge ownership passed 20/20, including partial claims and checkpoint restore.
  The Debug P2 shop passed 5/5 with no engine assertion or runtime-check failure. Release and Debug
  builds succeeded; source line endings and whitespace checks passed. Stop repeating P3/P4 checks:
  the user explicitly prefers extending the proven P2 wiring without constant testing.
- **Last manual session (now closed):** the user requested an untimed two-player weapon-pickup test.
  `play_finish_weapon` ran Release `WEDMmines01` with audio/Discord on, isolated saves under
  `build/test-saves/play_finish_weapon`, and no test fixtures, forced relocation or closing timer.
  Its old PID is recorded in `build/logs/play_finish_weapon.pid`; no game process remains. Keep its
  log as evidence of the revival failure. The user has asked for no further game launches at this time.
- **Vending navigation squares:** after PC key/controller prompts, `_DrawUserInterface` was still
  using solid-color drawing for the four arrow quads. Restore `FDRAW_COLORFUNC_DIFFUSETEX_AIAT`
  before drawing the HUD arrow texture. Release and Debug now include this fix; visual confirmation
  is still pending until the user is ready to resume play.

## Latest local pass (2026-09-29)

- **Liquids confirmed drawing.** `-port-diag` now logs `PORT-LIQ` lines: each liquid volume's type,
  surface centre, extent and layer textures at load (procedural ones name their world mesh), and each
  change in how many are drawn. `WEDMmines01` has a procedural texture liquid on the world mesh and two
  molten volumes (`tedt_lava`/`tf_lava`) at (673, -507, 355) and (1643, -687, 1152). A Release run with
  `-start-at 623,-449,400,90` (a walkway over the first lava river) drew the lava textured and glowing
  (`build/shots/liq_lava1/`); the user also saw it. Not yet looked at: the second pool (a ledge at
  `-start-at 1580,-654,1142,90`), water (reflect/refract with full-screen targets: `WECRruins01`,
  `WERMmorbot1`, `WERRreactr1`), mercury (`wedmmines02`), slime (`WEWJjourn01`), and liquid meshes.
- **Mines music vs. dialogue, measured.** `preopen_streams` is a retail-only table the source never
  reads. The level script plays `L02_Rager` (the ambience track with screams) at 0.40 in `WEDMmines01`
  and `L02_Rager3` at 0.50 in `WEDTtown_01`/`wediinvas01`. Decoded, those tracks are no hotter than the
  action music (-12 dBFS RMS vs `Mil_Action1` -12), so after the retail volumes they sit about 6 dB under
  the level music and about 12 dB under speech. No per-track fix is needed.
- **Gun loudness (user: "the gun audio and stuff is quite loud").** Every retail MusyX SoundMacro (all
  1,363) is StartSample/WaitMs/StopSample/End with velocity 127, so the retail data has no per-sound
  volume beyond the game's own emitter volumes, which the port applies through the GameCube chain. The
  player's laser (`SWDMl2lfire`, 2D, volume 0.6) peaks near -14 dB output, about dialogue level and
  about 8 dB over the music: that is the retail balance unless the chain itself is wrong. The chain has
  not changed since `6cfcd81` (2026-09-26). One untested suspect: MusyX's centre-pan law. The GameCube
  pans 2D effects and mono speech to the centre, while stereo music is panned hard left/right; the port
  plays mono sounds at full level on both speakers. `main.dol` has no pan table (only the DLS volume
  table at file offset 0x3de80c and a second curve at 0x3e0478, used near 0x8035fa6c), so MusyX computes
  the pan law in code.
- **PC mix: dialogue over effects (user asked for it).** Mission dialogue and bot chatter are bank sound
  effects on the same Sound Effects level as gunfire. With a -6 dB player trim the user still found
  effects too loud, and liked them at 3 of 18 slider pegs (master 0.17, about -11 dB), but that made
  mission 1's dialogue (`DM*` in `level_02d`) hard to hear. `fdx8audio.cpp` `_PortSfxGain` now:
  dialogue waves (`_PortIsDialogueWave`: level dialogue banks `level_NNd`, barter/buddy/b_intro/multi/
  grunt_26/`mg_*` banks, bot remarks `br*`, speaker lines like `ca_08o_010`, announcer `*_ann*`) keep the
  retail level; every other effect is -11 dB (`-sfx-db`, `MA_PORT_SFX_DB`); the player's own 2D `sw`/`sr`/`sd`
  sounds a further -6 dB (`-player-sfx-db`, `MA_PORT_PLAYER_SFX_DB`). 0/0 is the retail mix. Streams are
  untouched. `PORT-MIX` lines show each sound's trim. The rule was checked against every retail wave
  name; a muted Mines run logged dialogue at 0 dB, effects -11, player footsteps -17. User confirmed in
  play (2026-09-29): "sounds good now"; they turn Sound Effects down a little from full and consider the
  loud top end retail-like, so leave the defaults unless they ask.

### Loading screens (2026-09-29, latest)

- **They never showed before.** `level_Load` asked for `load620x340.bik`; the retail data renamed it and
  `main.dol`'s level_Load (call at 0x801b0c24) plays `gc_loading.bik` (496x272, Glitch walking past the
  moon) with the level heading. The front end's `wpr_system` `_Init` (call at 0x80157578, closed at
  0x80157e2c/0x80157ee8) plays `gc_loading2.bik` (512x272, Glitch running) with no heading; the source
  only set a reset-check callback there. With the movie missing, `loadingscreen_Update` drew nothing and
  the last frame stayed up. Both now play under `FANG_WINGC` (`level.cpp`, `wpr_system.cpp`). Retail's
  `loadingscreen_Init` has a third flag, set only for the front end, that polls a GameCube device while
  loading; the port does not need it.
- **`-mission` now loads behind the loading screen** with the campaign heading
  (`wpr_system_CreateLoadHeading`, new optional `pwszLoadHeading` on `game_LoadLevel`). The dev level
  picker and `-level` still load without one, as in the source.
- **Movie placement** (`fdx8movie2.cpp` `_BltWinFrame`): movies were all stretched to the 4:3 area,
  which suited only the 512x448 cutscenes. The GameCube centred smaller movies unscaled inside its
  512x448 full-screen texture, so the port now scales a 512x448 canvas to the 4:3 area and places the
  movie in it at its own size. The canvas grows to fit larger (for example upscaled) movies. The
  back-buffer size check now applies only to the unscaled fallback copy.
- Verified with captures: `build/loadscr_level_sheet.png` (Mines load, about 1.8 s) and
  `build/loadfront_sheet.png` (front-end load, about 1.2 s, then the logos). Level loads take 1.5-2 s here.
- **Movie copies for the user's upscaling:** all 39 `.bik` files copied to
  `D:\Documents\Metal Arms Movies` (outside the repo; retail data). 37 are 512x448 at 29.97 fps,
  one Bink audio track each. `$` in a name marks a localized movie. `for upscaling\` holds an H.264 MP4
  of each (CRF 12, same frames and rate, verified by packet count) and `audio\NAME.wav` originals, with a
  README on converting back through RAD Video Tools (Bink 1; round-trip one short movie first, since
  the port's runtime is Bink 1.5r and every retail movie is revision `BIKi`).

### Local co-op pass (2026-09-29, with the user testing 1-3 players)

- **Controls are dealt automatically** (`PCINPUT_LAYOUT_AUTO`, `pc_input.cpp` `UpdateAutoPadAssignment`),
  for the Co-op menu and `-mission W -coop N` alike. The keyboard/mouse is always player 1. With a pad
  per player they go in XInput order (player 1 keeps the keyboard too); otherwise players 2-N get pads
  first. A pad keeps its player until it disconnects; pads connected later go to players 2-N, then to
  player 1, so player 1 can switch between keyboard and a pad at any time. The first deal happens in
  `pcinput_SetLocalCoopSession`, so the level's first frame already has everyone. `PC co-op controls:`
  log lines record each deal. The Co-op page lost its Controls row; it lists each player's device
  live and refuses Start (with a message) without a pad for every player after the first.
- **Reconnect prompt:** it can no longer trap a keyboard player. Esc or B at "Please reconnect the
  controller" leaves for the menus, shown as a footer (`CMsgBox::SetPcFooter`; the retail box data has
  no buttons). The reconnect check uses the input layer's live state (`game.cpp` `_PortsOnline`), not
  the sample history, which trailed a deal by a few frames.
- **Pause menu and settings follow the player who paused** (`CPauseScreen::m_nPlayer`): prompts,
  HUD-mode pages, the controls chart and Advanced Settings (look sensitivity, invert and vibration are
  per player). A controller-only player never sees keyboard keys, and Mouse Sensitivity is hidden for
  them. User confirmed pad and keyboard pauses.
- **Cutscenes:** in-game cutscenes freeze every player, so anyone's Start skips; Bink
  movies (`cutscene.cpp`) take any player's A/Start/Back.
- **No friendly fire:** player-to-player damage is refused (`CBot::InflictDamageResult`), and Glitch's
  melee no longer breaks a partner's limbs directly (`CBotGlitch` melee), which had left partners as legs.
- **Deaths:** a downed player stays down, with "Waiting for a checkpoint", while a partner stands. The
  next mid-level checkpoint revives them beside a partner, just before it is saved
  (`CPlayer::CoopReviveForCheckpoint` from `checkpoint_Work`): Resurrect, parts rebuilt, controls back.
  If everyone is down, the retail checkpoint restore brings all of them back. User confirmed clean
  co-op deaths and checkpoint revives on 2026-09-30.
  Every co-op return (checkpoint revive, and the fell-out-of-the-world / stuck return beside a partner)
  plays the retail checkpoint-restore respawn effect and sound (`CBotGlitch::PortPlayRespawnEffect`,
  the same calls as `CBotGlitch::CheckpointRestore`): shell, particles, respawn animation, controls
  held for its second. User confirmed the vanilla checkpoint revive animation on 2026-09-30.
- **Partner cameras during scenes** (`game.cpp` `_CoopWatchCamerasWork`): while player 1's camera is a
  cutscene or script camera (`MAScript_bPortScriptCameraActive`, set by `Cam_Activate`), a partner is
  letterboxed, or player 1 and a partner are both frozen (`HasEntityControl`), each partner on a
  third-person camera gets a manual camera 8 units behind them (1.5 up, pulled in at walls) that eases
  to look at player 1's Glitch, or at what player 1's script camera shows (ray hit within 60, else 25
  ahead). It eases back behind them afterwards. `Co-op: player N's camera follows ...` log lines. The
  user reported the first version never engaged; the frozen-together condition is the fix. User
  confirmed partners lock onto player 1 during player 1's cutscene on 2026-09-30.
- **Tripwire gate test switch:** `-coop-hold off` (`MA_PORT_COOP_HOLD=0`) disables the gate.
- **Demo movie** on the idle main menu waits 300 s instead of retail's 25 (`_MAIN_MENU_INACTIVE_TIME`,
  `wpr_system.cpp`); mouse movement also resets the timer.
- **Tripwire gate** (`entity.cpp`, `CEntity::_CoopHoldTripwireEnter` / `CoopTripwireWork`): a
  player-tripped enter event waits until every standing player has arrived (sticky, any order), then
  fires once with player 1 as the tripper. Collectables and kill volumes are never held; an attached
  door still opens at once. The waiting player sees "Waiting for your partners". Verified in
  `WEDMmines01`: `alloy`, `buddygo1` and `elevup` held for player 1 alone; `alloy` released when
  player 2 arrived.
- **Main-menu Co-op / Close Game entries in the retail art** (`wpr_system.cpp` `_PcMenuArtLabel`): the
  retail entries are quads onto `tehmalogo$` (Campaign / MultiPlayer / an unused Game Demos, gold
  italic, 0.17 slant). The two PC labels are cut from those letters at draw time along the slant
  (slice table in atlas pixels; the port ships no art): "Co-op" = C o [i-dot hyphen] o p, "Close Game"
  = C l o s e + "Game". No Q or k exists in the art, hence "Close Game". Same scale as the retail quads
  (0.8823 model units per 2 atlas px, scale 1.15, times the menu's scale multiplier), left-aligned with
  MultiPlayer at the retail 0.15 spacing, retail `gfh_logo05` highlight centred on the word (it is a
  glow bar narrower than the word, as on the retail entries), fading in with the menu.
  `build/menuart_zoom.png` shows it.
- **Co-op menu is the retail "Players Join In" screen** (`_bPcCoopJoin` in `wpr_system.cpp`), retitled
  "Co-op" with a "Saves with player 1's profile" note. Entering it starts an AUTO session for four
  ports (keyboard alone on player 1's box, pads on 2-4); each player joins and picks a profile as in
  multiplayer; two or more are required. The launch copies each chosen profile's settings (invert,
  auto-centre, assisted targeting, four-way select, controller config, look sensitivity, vibration,
  sound/music levels) and colour into a virtual session profile, so no one's profile is written, and
  keeps the join screen's pad deal (`pcinput_SetLocalCoopPlayers`). The old generated Co-op page is gone.
- **Co-op saves (2026-09-30, user verified save/reload):** after joining, player 1 (the first box in play)
  gets the retail Launch menu retitled "Co-op", without Edit Settings: Start Adventure (Start Over
  warning, difficulty), Continue Adventure, Replay a Level; Back returns to the join screen (everyone
  joins again, as retail multiplayer does from Game Type). It shows the campaign saved with player
  1's profile (`coopsave_*` in `gamesave.cpp`; `_PcCoopEnterCampaignMenu` puts it in `_paProfiles[0]`
  as a virtual profile, everyone's own profiles wait in `_aPcCoopPersonal` and come back on Back or
  at the next `wpr_system_ResetToStartupScreen`). The file is `Co-op\coop-<hex name>.sav` in the save
  root (`fstorage_PcCoopRead/Write`, `.tmp` then replace): up to 8 records (player 1 plus partners,
  least recently saved one replaced), each a profile name and a `GameSave_ProfileData_t` with level
  progress and per-level inventory; CRC and record size checked on load. `coopsave_BeginSession`
  matches partners by profile name; a new partner, one who missed levels, or one without a saved
  profile (not kept) takes player 1's inventory for those levels; a new game resets every record.
  `CPlayer::UninitLevel` now records a completed level for every co-op player (was single-player
  only, so co-op partners restarted each level with the starting inventory), and the level-complete
  screen calls `coopsave_SaveSession` instead of the profile save. Log lines start `Co-op save:`.
  Player 1 without a saved profile plays the same menus but nothing is written.
  Verified by the user (2026-09-30): a new game finished `WEDMmines01` and wrote
  `Co-op\coop-...sav` ("saved 'Profile1&&&' at level 1 for 2 players"); the user subsequently confirmed
  co-op saving and reloading worked perfectly in their testing.
- **Co-op cutscenes and deaths (2026-09-30, user verified):** no letterbox bars in local co-op
  (`letterbox_Draw`; every split view drew its own). The letterbox hides and restores every player's
  HUD (retail: only `CHud2::GetCurrentHud()`, which could leave a partner's HUD off after a cutscene);
  a player who went down during it stays without one. The co-op checkpoint revive turns the revived
  player's HUD and weapon select back on (`letterbox_PortRestoreHud`; death turns them off and only
  the retail checkpoint restore turned them on). Partner cameras no longer turn to watch a dead
  player 1; with player 1 down only a script camera is followed.
- **Transmission box in split screen** (`Hud2.cpp`, "Transmission from Colonel Alloy"): the text is
  printed through the split view's scale/offset transform, but ftext draws the area's box later from
  the area's own coordinates, which `Hud2_XFormPrintf` had restored; the box stayed left of the text.
  The area is now moved with the text each frame (from `_TRANSMISSION_AREA_*`). Built, untested.
- **Save root** (`fdx8storage.cpp`, `pc_input.cpp`): everything under `%APPDATA%\MAGITS` (or
  `-save-dir`): `Profiles\`, `Co-op\`, `settings.ini`. The first time `Profiles` is made, profiles
  are copied (not moved) from the root itself and from `%APPDATA%\Metal Arms PC Port\Saves`;
  `settings.ini` is copied from `%LOCALAPPDATA%\Metal Arms Source Port` when missing. Verified: the
  user's two profiles and settings came across (`build/logs/magits_migrate.log`).
- **Xbox save-screen text:** the retail `saving_prof_text_xb` lines "Do not power off your" /
  "Xbox console." (and the memory-unit variant) now read "Please do not close" / "the game."
  (`_aPcText` in `wpr_datatypes.cpp`).
- **Player colours:** `CMultiplayerMgr::SetupPlayer` tints each co-op Glitch with the profile colour
  (made unique, as in multiplayer; no profile = yellow, blue, ...), and `Multiplayer_PlayerColor`
  returns it for HUD/radar use. Co-op Glitches load the multiplayer mesh `GRDGgltchMP` (its paint
  takes the colour; the campaign mesh `grdggltch00` has baked yellow paint, which a tint only muddied),
  chosen by `CMultiplayerMgr::IsLocalCoop()` (set in `PreLoadInitLevel`, before the world's bots are
  built); level-specific Glitch meshes are kept. User confirmed the colours look right.
- **Colour choice on the co-op join screen:** a ready player's box shows `{ Colour }` in that colour
  (`_anPcCoopColor`); left/right on their own controls cycles it, skipping colours another ready
  player has. It starts on the profile's colour and is what the launch copies into the play profile.
  `build/coopcolorpick_sheet.png` shows Yellow then Blue after Right.
- **Pause menu runs as the player who paused** (`game.cpp`, `CPauseScreen::PausingPlayer()`): it was
  worked after the player loop with the last player current and player 1's inventory, so the last
  player's controller drove everyone's pause menu (player 1's Esc could not unpause).
- Not done: online (user chose Parsec); controller hot-swapping and a full campaign progression
  playthrough remain untested; texture upscaling needs a texture-replacement loader.
- Test windows take focus when they open, so the user's controllers drive them: `menuart2` went into
  co-op by itself. Warn the user before a capture run while they hold a pad.

### Console-limit audit, debug shortcuts, results music (2026-09-29, later)

- **PC graphics limits lifted** (all `FANG_WINGC`, Release-tested on Mines 1/2, Ruins 1, Journey 1: no
  crash/assert, 126-141 fps unlocked):
  - Anisotropic filtering: `port/compat/d3d8_compat.cpp` turns every LINEAR minification into ANISOTROPIC,
    16x or the GPU cap (`-aniso N`, `MA_PORT_ANISO`; 1 = off). The consoles had bilinear/trilinear only.
  - Shadow buffers (`fdx8shadow.cpp` `_PORT_SHADOW_*`): 1024/512/256 texels, were 256/256/128 (even the
    "512" pool was 256). Render-target viewports follow the texture size; each pool's shared depth
    buffer is created at the same size.
  - Liquid reflection cube 512 (was 128), dynamic sphere reflections 256 (was 128) in `fdx8sh.cpp`.
  - Sniper scope target `FSRT` (`game.cpp`): 2048x1024, was 512x256 stretched over the whole screen.
  - Particles (`fparticle.cpp` `CalculatePercentOfParticlesToDraw`): no distance thinning between the
    skip-draw and cull distances; near-camera thinning kept.
  - Placed objects with an authored cull distance (`meshentity.cpp`) draw twice as far.
  - Decals: 1,000 / 16,000 vertices (`fang.cpp`); `wedmmines02` still ran out at 400/6,000. **Built into
    the next build only** (not yet compiled when the user's session was relaunched).
  - Checked and left alone: per-level `far_plane` tables (authored with each level's fog), 8 hardware
    lights, mip bias 0, particle/emitter budgets (500 emitters, 3,000 particles; usage far below).
  - **MSAA not enabled.** The engine supports it (`FVidWin_t::fUnitFSAA`), but Bink frames reach the
    back buffer by `StretchRect` from a plain surface (`fdx8movie2.cpp`), which D3D9 refuses into a
    multisampled back buffer, and the fallback locks the back buffer (also refused), as do the port's
    screenshots and `CopyRects` readbacks and the engine's `D3DPRESENTFLAG_LOCKABLE_BACKBUFFER`. The clean
    route is an MSAA render target resolved into the ordinary back buffer before movies/Present.
- **Debug shortcuts off player 1's controls.** On Windows `Gamepad_nDebugPortIndex` is port 0, the
  player's own controls (the consoles used a second pad). Right mouse (secondary fire) toggled the
  script monitors (`MAScriptTypes.cpp` `CMAST_BotWrapper::Work`: the "NONETRIPWIRE ENTER EVENT" lines
  the user saw), quick-select up/down cycled the perf overlay, and the dev level-win cheat sequence was
  live. All three now need `-debug-info`. The AI and checkpoint debug shortcuts were already gated.
- **Results-screen music fixed.** `MA_Theme` was created and `Play()`ed at once while the stream still
  loaded on the worker, so the request was dropped. `_SP_Work` now starts it when the stream reports
  STOPPED (ready), at the menus' 0.25, like `wpr_system.cpp`. Verified in a log: `PORT-SND stream 'MA_Theme'`.
- `-test-win-level S` (`gamepad.cpp`): completes the level after S seconds of unpaused play through the
  level-win path. Use it to reach the results screen and the next level in tests.
- User note (not pursued): one ambience track with destruction/explosion sounds "sorta sounds out of
  place" (likely `L02_Rager`); low priority.
- Another work session was active in this checkout at the same time (cut-content revival:
  `botsniper.cpp`, `grapple.cpp`, `CMakeLists.txt`, `-spawn-sniper-test`, `docs/cut-content-revival-audit.md`).
  Check `git diff` for its changes before committing either session's work.

### Cut-content Mil Sniper in the campaign (2026-09-29) - REMOVED

> **Removed later on 2026-09-29 at the user's request** after meeting one in play: the recovered Mil Sniper
> model "doesn't fit, it's very tall and poorly animated". All of it is gone from the build: the Grunt
> conversion and the `BotSniper` entity type (`entity.cpp`), the AI defaults (`AIBuilder.cpp`), the
> `CBotSniper`/`CGrapple` system registrations (`gameloop.cpp`) and `DrawEffects` call and test spawn
> (`game.cpp`), `-spawn-sniper-test` and `-snipers-every`, and the `botsniper*.cpp`/`grapple.cpp` build
> entries (`CMakeLists.txt`; the files are back to their pre-session source). The general NaN guards found
> on the way stay (`CBot::HandleCollision` push vector, `CVehicleSentinel` cannon aim). Don't reintroduce
> cut enemies by swapping models into levels. The notes below record what was tried.


- From `docs/cut-content-revival-audit.md` / `docs/enemy-revival-map.md`: the Mil Sniper is the only cut
  enemy with enough code to revive. Queen, Water/Nuke grenades, grenade launcher and the visual-only models
  would need new gameplay invented; not attempted.
- **Spawning** (`entity.cpp` `_PortGruntBecomesSniper`): world Grunts named `sniper<digits>` (wewjjourn02's
  script-spawned `sniper1-3`) become `CBotSniper`; so does about 1 Grunt in 8 elsewhere (`-snipers-every N`,
  default 8, 0 = only the retail roles), picked by a hash of level and position so the same ones every load.
  `-cut-enemies off` restores all Grunts. The reused `snipergrunt1` template name in the Space Stations is
  not a sniper role and is not matched by name.
- **AI** (`AIBuilder.cpp`, Sniper branch): single aimed shots, stop-and-shoot, long sight, no vehicles or
  grenades.
- **Bugs fixed in the recovered actor** (`botsniper.cpp/.h`):
  - `AppendTrackerSkipList()` used the old no-argument signature, so it no longer overrode `CEntity`'s (which
    adds nothing): the Sniper collided with its own mesh, never landed, piled up fall speed and was flung
    (the "floating, flung away" report). Now the current signature.
  - `_CalcLaserPt` guarded with the bullet-trail count, and `DrawEffects` (which resets the laser-sight
    count) was never called: `m_avLaserPt[20]` overflowed every frame and corrupted statics (crashes in
    `tracer.cpp` `_GroupWork` and `CDamage::Work`). Guard fixed; `game.cpp` now calls
    `CBotSniper::DrawEffects()` beside the Scout/Elite beams (trails, laser sights, grapple cable).
  - The tracer's kill callback was empty, so shots did no damage. It now submits impact damage like the
    Snarq's, with retail `RivetL2` (NPC) / `RivetL3` (player-controlled); retail has no Sniper profile.
  - The retail `b_sniper` gen table names `None_Im_Invincible` armor; PC uses the Grunt's.
  - `_TracerMovedCB` bound check was off by one.
- **Verified:** Release Mines 1 fight (`build/shots/sniper_fight2/`): Snipers land, close in, fire 14
  aimed shots, drain Glitch's battery and destroy him. Debug fight 80 s, 19 shots, no assert. Debug sweep
  (`snipersweep_*`: Comm 1 14 Snipers, Journey 2's 3 retail roles, Reactor 1 8, City 1 3), each completed with
  `-test-win-level 60` through the results screen: no crash/assert/new errors. `-port-diag` logs
  `PORT-SNIPER` status every second and each shot.
- **Not verified:** possessing a Sniper (it has a data port from the gen table, so possession should reach
  its rifle, scope and grapple), the AI ever grappling (no AI grapple decision exists), and Journey 2's
  snipers after their script spawns them. `tools/mission_parallel.py --game-args "..."` passes extra
  game arguments to every instance.

### Cut throwables: Nuke and Water Grenades (2026-09-29 experiment) - REMOVED

> Removed on 2026-09-30 at the user's request: not enough surviving code to justify remaking them.
> Their items, pickup replacements, projectile pools and custom effects are gone, along with
> `-cut-content` / `-cut-enemies`. Ordinary Coring Charges and EMP Grenades are restored. The
> `-test-give` aid remains useful for retail weapons. The notes below describe the removed experiment.

- The unused pickup models `gp_snuke`/`gp_swater` now have items and behaviour, built on existing weapons:
  - **Nuke Grenade** = Coring Charge (`weapon_gren.cpp`, `m_bPortNuke`): own projectile pool of `gp_snuke`;
    detonate callback adds `HugeRatExplosion` at the centre plus a ring of six coring blasts 12 units out.
  - **Water Grenade** = EMP Grenade (`weapon_emp.cpp`, `m_bPortWater`): pool of `gp_swater`, EMP shutdown
    x1.5 and radius x1.25, `e_wtrexpl01` water blast particle.
  - Items appended in `ItemRepository.cpp` (clones of the Coring Charge/EMP entries; retail indices do not
    move); `Item.cpp` MakeWeapon/GetWeaponName; collectable types `COLLECTABLE_WEAPON_NUKE/WATER` (IDs are
    not saved) cloned from the retail pickups with one grenade each.
  - About 1 in 4 level Coring Charge pickups and 1 in 3 EMP pickups become them (position hash; drops roll
    the same odds). Gated by `-cut-content` (older name `-cut-enemies`).
- `-test-give ITEM` (player.cpp) gives and selects a weapon; `-export-character-meshes` lists accept
  `tex:NAME` to decode a retail texture (gcmesh `gcmesh_ExportTextureByName`).
- **Verified:** Release build; Comm 1 and Mines 2 convert pickups with no crash; Mines 2 `-test-give "nuke
  grenade"` threw one and logged `Nuke Grenade detonated`. **Not verified:** how the blast/pickup box look
  (no icon art exists: HUD reuses the Coring Charge/EMP icons), Water Grenade thrown in play, Debug asserts,
  pickups by walking into them, save/load with these items in the inventory.
- Not feasible with retail data: grenade launcher/key models (textures missing), hang glider (no vehicle
  code), Swarmer Queen (disabled prototype, no arena).

## Earlier local pass (2026-09-28)

- Added `tools/export_characters.py`, a character catalog, a `.mtx` animation reader, and a Blender
  scene importer. The clean Blender package is under
  `build/export/character_porting_kit_20260928/`: the full collection has 29 rigs, 29 meshes, 96,695
  triangles, 107 packed textures, and 586 animation actions; a focused Shady/Slim vendor file has 2
  rigs and 43 actions; an enemy sample has 8 rigs and 199 actions. The importer converts Y-up rigs to
  Blender Z-up, centers/grounds each model, packs items by their measured bounds, and keeps character
  surfaces opaque. Blender reopened all three files; no ground-plane overlaps, alpha-to-opacity
  links, missing animation curves, or ungrounded models were found. The `ARDH`/`ARDI` vendor
  animations are now catalogued, including Slim's `arditable02` fold-table clip referenced by source
  code. The extraction-only game process uses `-no-audio`; it does not alter normal game audio. See
  `docs/character-export.md` and `docs/vendor-robot-porting.md` for details.
- Added an opt-in static world OBJ exporter in `port/gcmesh.cpp`, run with
  `python tools/export_world_obj.py wedmmines01`. The world-only loader exported all 46 embedded
  `wedmmines01` mesh chunks: 119,233 vertices and 55,813 triangles. The output has a combined scene
  OBJ, 46 individually importable mesh chunks, 530 materials, 41 decoded game textures, a manifest,
  and loader logs under ignored `build/export/`. The export uses the game's texture decoder and MTL
  diffuse maps; Blender 5.2 imported the combined OBJ as 861 mesh objects with 55,813 faces, resolved
  all image paths, and saved a native `.blend` in Material Preview mode with view clipping scaled to
  the level bounds. Exported chunks retain native level-space coordinates, normals, and UVs. Only the
  first available surface texture is mapped;
  shader layering, lightmaps, placed world-shape objects, collision data, runtime liquids, and gameplay
  behavior are not reproduced. See `docs/world-export.md`. The Release build, full world-only export,
  and Blender import passed.
- Exported all 14 multiplayer world resources present in the retail master (`we01multi01`ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“`we12multi12`,
  `we14multi14`, and `we15multi15`) to `build/export/multiplayer_maps_20260928_030038/`. Each has a
  combined OBJ, per-resource chunks, MTL and decoded textures, manifest/logs, and a Blender 5.2 file
  framed with the map-scaled viewport clip range. `we13multi13` is not present in the master index.
- The Windows launcher now sets `bPlayerDeath=TRUE`, matching the original launcher. This lets
  players and Droid allies reach the existing death/respawn paths; recruited-friendly damage and
  multiplayer friendly-fire rules were left unchanged. Debug and Release builds pass. An interactive
  enemy-kills-player and enemy-kills-ally check remains.
- The end-of-level results screen now starts `MA_Theme` after unloading the completed level and
  destroys the stream when leaving the screen. This uses the existing front-end theme because the
  retail `we_1victory` world has no music entry; the intended victory cue is not present in the data
  audit. The results screen still needs an audible runtime check.
- Windows now runs the liquid system from the visibility pass; that call had been compiled out for
  Windows, leaving liquid volumes and meshes unrendered. The Direct3D liquid plane and mesh code is
  enabled on Windows. The user will visually review the result in-game; this change has only been
  build-verified so far, so falling meshes and molten surfaces remain unconfirmed.
- PC gameplay meshes now stay on LOD 0 while shadow rendering keeps its existing LOD bias. This
  avoids the visibly pointy distance models. A 50-second Mines run and a 55-second Research Facility
  run show rendered gameplay frames with no asserts; close-range bots remain detailed.
- The screenshot assertion is at `fdx8math.inl:131`, the nonnegative-input check in `fmath_Sqrt`;
  it is distinct from older `FloatToU32` tracer-alpha asserts already fixed by commit `1b6210b`.
  Current Debug runs did not reproduce it. Invalid square-root inputs now log their value, and
  Fang assertions capture a symbolized stack to the run log before showing the dialog. A sentinel
  cannon also falls back to its barrel axis if its target-to-muzzle aim vector is degenerate; that
  candidate has not been tied to the reported assertion.
- `tools/port_run.py` and `tools/mission_parallel.py` now leave audio enabled by default. Use `--mute`
  only for a deliberately silent test; `--no-audio` remains available when audio setup itself should
  be skipped. All test windows are labeled with their audio mode.

## Previous local pass (2026-09-27)

- `SetDamageable=false` on four inactive grunts in `WEDMmines01` now sets the existing invincibility
  flag. `NoLiftBlockChecking=true` on two lifts in that world skips only the bot blockage check during
  line movement; ordinary physical collision remains. `useby=Mil` on three `WEWCcomm_02` switches
  was already applied, but the switch parser fell through and emitted false unknown-command warnings;
  it now returns success.
- Debug and Release `ma_port` builds passed. Three parallel, isolated, muted Debug runs of
  `wedmmines01`, `wewccomm_02`, and `wemccity_01` loaded, with no crash, assert, allocation failure,
  or audio error. Mines property warnings fell from 13 to 7; Communications fell from 4 to 1.
  The remaining seven Mines warnings are malformed two-field `goodie1` tables followed by valid
  four-field drop tables. Communications has a six-field chip drop (`chip,1,1,1,X,safe`) that the
  current parser skips; the same authored pattern appears in `WESSstatn02`. Neither has been changed
  without gameplay evidence of the intended drop. City still reports two malformed `ColorRed`
  entries; the same pattern occurs in other retail worlds. Leave its color fallback alone until the
  rendered tint can be compared with retail. `dropfreq` on two front-end LiquidMesh entries remains
  unimplemented; the source only supports it on LiquidVolume.
- Actual lift blockage behavior and the newly invincible grunts still need gameplay observation;
  automated level loads confirm property parsing, not those events.

## Diagnostics follow-up (2026-09-27)

- The front-end logo transition is not explained by `BinkOpen`/`BinkClose`: an isolated Debug run
  measured each close at 0 ms and the two following opens at 47 ms. Transition frames reached about
  63 ms; the separate 1,166 ms worst frame occurred during initial loading. Keep the reported logo
  gap open until it is reproduced with frame timing tied to the visible transition.
- Added opt-in `PORT-VB` timing around vertex-buffer locks. A 55-second two-player Debug town run had
  one Present stall (109 ms), but no matching lock stall. Several individual locks were 15-16 ms,
  including dynamic `NOOVERWRITE` locks. This does not establish that the older 110 ms zipline lock
  reproduces; collect another sample before changing buffer policy.
- Corrected the audio mixer diagnostic: listener state `EXITED` (value 2) was incorrectly counted as
  in-range/voiceless. The updated log condition reports only ENTERED, PRESENT, or SWITCHED listeners.
  The prior 321 `L02_rslide3` voiceless records were all EXITED; the same run had voiced instances of
  that wave, so there is no confirmed ambient playback bug from those records.
- After the latest changes, Debug and Release builds passed. A 24-second isolated Release run loaded
  `WEDMmines01` with no crash, assert, allocation failure, audio error, or script error. It retained
  the seven known malformed-goodie warnings. Startup produced one WndProc/Present hitch each; check
  whether either repeats before diagnosing a runtime issue.
- Fixed launch from Explorer/build directories: if the current working directory has no master file,
  the launcher searches upward from the executable for the selected relative data folder. An invalid
  data path now gets a visible message box. Verified both a no-`-data` launch from `build/Release` and
  a relative `-data gamedata/files` launch from another working directory; both found the retail
  master file and reached the main menu. A captured 1280x960 frame confirmed the menu rendered.

## Repository

- Private GitHub repo `cobblecorn/metal-arms-pc-port`. `main` is the verbatim source drop; all work is
  on `x86-port`, so `git diff main..x86-port` is the whole port.
- **Never commit retail data.** `gamedata/`, `main.dol`, disc images (`.rvz`/`.iso`) and anything
  extracted from them are git-ignored; tool reports that contain asset values go under ignored `build/`.
  Before pushing: `git ls-files | grep -iE '^gamedata/|main\.dol|\.rvz$|\.iso$'` must print nothing.
- Push to `origin x86-port` after the change is built and tested. Fetch and rebase your own unpushed
  commits first if another session may have pushed.
- Use a `Co-Authored-By:` trailer only when a collaborator is explicitly credited.
- Sources under `ma/` are **CRLF**; keep them CRLF. Port files (`port/`, `tools/`, docs) are LF.
  `python tools/eol.py check` lists files whose endings differ from the committed file;
  `python tools/eol.py fix FILE...` repairs them. Run it before every commit.
- The untracked `windows icon/` folder is the user's; leave it alone.

## Layout

| Path | What |
|---|---|
| `ma/` | Original Swingin' Ape source: engine in `ma/Lib/Fang2`, game in `ma/App/ma`, tools in `ma/App/*`. Port changes are small and in place, under `#if FANG_WINGC` (the Windows build using GameCube data) or `MA_PC_INPUT`. |
| `port/main_win.cpp` | Win32 entry point (replaces the MFC launcher): options, logging, crash/CRT diagnostics, stall sampler, test keys, boot. |
| `port/gcdata.cpp`, `port/gcmesh.cpp`, `port/gcaudio.cpp` | GameCube data converters: tables, textures, meshes and collision, worlds, animations, particles, fonts, camera animations, sound banks and streams. |
| `port/pc_input.cpp` | Keyboard/mouse and XInput mapped onto Fang's pads; mouse look; menu pointer; typed text; input layouts; prompt style. |
| `port/discord_rpc.cpp` | Discord Rich Presence over the IPC pipe (no SDK). |
| `port/compat/` | Direct3D 8 API on top of D3D9Ex (`d3d8_compat.cpp`), minimal D3DX. |
| `tools/` | Test runners, log tools, retail-data tools, and the opt-in `export_world_obj.py` level exporter. |
| `gamedata/` (ignored) | `sys/main.dol`, `files/` (`mettlearms_gc.mst`, `Movies/*.bik`, `*.wvs`), `mst/` (extracted files). |
| `build/` (ignored) | Build output; `build/logs/` test logs; `build/shots/` captures; `build/test-saves/` test profiles. |

## Build and run (Windows)

Visual Studio 2022 (x86 tools), CMake 3.20+, Python 3 (Pillow for screenshot helpers). 32-bit only.

    cmake -S . -B build -G "Visual Studio 17 2022" -A Win32
    cmake --build build --config Release --target ma_port -- -nologo -v:m
    cmake --build build --config Debug --target ma_port -- -nologo -v:m
    build\Release\ma_port.exe -data gamedata\files -mission wedmmines01 -log build\logs\play.log

- **Release** (`build/Release/ma_port.exe`) is what the user plays: ~1.5 ms of frame work. It is built
  with `/Zi /Oy-` and linked `/DEBUG`, so crash stacks and the stall sampler still symbolize.
- **Debug** has the engine's asserts (`FASSERT`) and the CRT's checks: use it for sweeps and bug hunts.
- Input unit tests: `cmake --build build --config Debug --target ma_input_tests -- -nologo -v:m`, then
  `build\Debug\ma_input_tests.exe` (must print that all tests passed). Run after touching `pc_input`.
- A running game locks its exe: close that PID before relinking.
- Git Bash turns `/flag` arguments into paths: pass MSBuild options as `-nologo -v:m`. PowerShell wraps
  native stderr as errors.

### Game options (all in `port/main_win.cpp`'s header comment)

| Option | What |
|---|---|
| `-data DIR` / `-mst FILE` | data directory (default `gamedata\files`) / master file (default `mettlearms_gc.mst`) |
| `-mission WORLD` | start a campaign level with its mission data (the normal way to test a level) |
| `-coop 2..4` | with `-mission`: local campaign co-op, 2-4 players (shared or separate inputs) |
| `-coop-hold on\|off` | co-op tripwire gate (events wait for every standing player); default on, `off` for tests |
| `-level WORLD` / `-world-only WORLD` | debug-launch a world / load a world and exit |
| `-dev-menu` | boot into the development level picker instead of the retail front end |
| `-res WxH`, `-fullscreen`, `-no-vsync` | window size (default 1280x960), fullscreen, present without vsync (for measuring) |
| `-mute` | keep audio active but silence game and Bink output; use only when silent playback is intentional |
| `-no-audio` | skip audio setup entirely (faster loads; audio errors are then meaningless) |
| `-log FILE`, `-asset-log FILE`, `-console` | engine log, resource-loading log, a console window showing the log |
| `-port-diag` | the port's `PORT-*` diagnostics (perf, hitches, stalls, audio mix); also `MA_PORT_DIAG=1` |
| `-debug-info` | the game's on-screen debug overlays (script messages/errors, fps, AI and checkpoint drawing) |
| `-shots DIR -shot-every N` | save the back buffer as `DIR\shot_NNN.bmp` every N frames (default 300) |
| `-test-keys "S:VK,..."` | press virtual key VK S seconds after launch; `gS` counts from the first gameplay frame. Works without focus |
| `-sfx-db N` | trim in dB for sound effects other than dialogue; default -11, 0 = retail mix |
| `-player-sfx-db N` | further trim in dB for the player's own 2D sounds (weapons, footsteps); default -6, 0 = retail mix |
| `-start-at X,Y,Z[,YAW]` | move player 1 to a world position (yaw in degrees) half a second after they get control; find spots with `PORT-LIQ` lines or a `tools/export_world_obj.py` export (native coordinates) |
| `-discord-app-id ID\|off` | Rich Presence application (default: the port's own); `off` for tests |
| `-input-layout shared\|separate`, `-button-prompts auto\|keyboard\|xbox\|playstation`, `-mouse-sensitivity N`, `-aim-assist auto\|on\|off`, `-save-dir DIR`, `-instance-label NAME` | input, prompts, saves, a window-title label |

Useful virtual keys for `-test-keys`: Esc `0x1B` (pause; skips movies), Space `0x20` (jump; skips
movies), Enter `0x0D`, E `0x45` (use/drive), Q `0x51` / R `0x52` (weapon lists), W `0x57`.

Environment variables: `MA_PORT_DIAG`, `MA_PORT_STALL_MS` (stall threshold, default 100),
`MA_PORT_TEST_KEYS`, `MA_PORT_SHOTS`/`MA_PORT_SHOT_EVERY`, `MA_PORT_POINTER_DEBUG=1` (outline menu hit
boxes), `MA_PORT_TEXPROBE=1` (dump texture instances), `MA_PORT_INPUT_LAYOUT`,
`MA_PORT_BUTTON_PROMPTS`, `MA_PORT_MOUSE_SENSITIVITY`, `MA_PORT_AIM_ASSIST`, `MA_PORT_SAVE_DIR`,
`MA_PORT_DISCORD_APP_ID`, `MA_PORT_DISCORD_LARGE_IMAGE`/`_TEXT`.

## Where development is (2026-09-27)

| Area | State |
|---|---|
| Data | Every retail asset type converts: tables, GX textures, static/skinned/streamed meshes and kDOP collision, worlds and visibility, animations, AI graphs, particles, fonts, camera animations, sound banks, DSP-ADPCM streams. All 393 scripts bind every native. |
| Missions | All 42 campaign missions load and run under the sweep (`-mission`, Debug, 90 s each, scripted jumps). See "Mission sweep" below for the last result. |
| Rendering | Direct3D 9Ex behind the D3D8 API. Baked vertex lighting, HUD, particles, decals. Glitch's dark legs fixed (`fmesh_InitNormalSphere`). User confirmed alt-tab/other applications caused no issues (2026-09-30); fullscreen switching and window resize still need checking. |
| Front end | Retail front end (language, logo movies, main menu, profiles, campaign) with the mouse: hover, click, right click = Back, wheel. Pause menu and its settings screens take the pointer; On/Off, arrows and bars take clicks (user confirmed). Generated controller chart. Back keeps settings changes. |
| Input | Keyboard/mouse (raw mouse look, auto capture) and XInput; mouse aiming in vehicles and manned guns including the AA gun (Hold Your Ground, user confirmed). Keyboard/Xbox/PlayStation prompts, flush left of the text. Typed profile names. |
| Audio | MusyX banks, streams, and the GameCube volume chain. Gameplay audio after the intro movies fixed this session (39 of 39 samples audible in a meter run); the user confirmed audio in play. |
| Movies | Bink on the game's DirectSound device, 16 MB read-ahead, heap allocations. |
| Performance | Log writes on a background thread, stream loads on a worker; no stalls in normal play except a rare vertex-buffer lock wait (Open work 2). |
| Saves | Profiles in `%APPDATA%\MAGITS\Profiles`, co-op campaigns in `%APPDATA%\MAGITS\Co-op`, `settings.ini` beside them; checkpoints (1 MB). Checkpoint write-failure handling compiles but is untested. |
| Discord | Connects under the port's application; Discord accepts the activity. No image asset. |
| Co-op | PC menu entry (2-4 players, automatic input assignment), separate campaign saves per player 1 profile, downed players revive at checkpoints. User verified save/reload, deaths, checkpoint animation and partner cutscene cameras (2026-09-30). Hot-swapping and a full progression playthrough remain. See `docs/coop-audit.md`. |

### Co-op page styling and retail bot impulse property (2026-09-27)

- The PC Co-op page now has a dark blue framed panel over the animated front end.
  Its rows are spaced above the standard Accept/Back prompts, with a separate
  network-status line. `build/shots/pc_coop_page_verified/latest.png` and
  `build/shots/pc_coop_page_wide/latest.png` visually confirm 1280x960 and
  1920x1080 layouts. The page is still an experimental entry; no
  networking or campaign progress saving was added.
- The retail `DisableVelocityImpulses=true` bot property is parsed into a
  checkpointed bot flag. `CBot::HandleVelocityImpulses` clears pending impulses
  for flagged bots while leaving normal movement intact. Retail uses the
  property on six `botblink` miners in `WEMCcity_01`, `WEDMmines01`, and
  `WEWCcomm_02`. The exact retail runtime branch is unverified; a flagged
  miner's cable-release impulse is also suppressed by this implementation.
- Debug and Release build. Parallel isolated muted Debug mission runs loaded all
  three levels with no crash, assert, audio or script error. None logged
  `disablevelocityimpulses` as unknown. The front-end capture showed no overlap
  or literal formatting escapes after correction. Automated navigation proved
  the page rendered, not physical pointer/controller use.
- The run logs still have other retail property warnings: malformed `ColorRed`
  in `WEMCcity_01`; `setdamageable`, `noliftblockchecking`, and short `goodie`
  tables in `WEDMmines01`; `useby` in `WEWCcomm_02`. `tools/port_run.py` now counts
  "Error interpreting Max User Properties" blocks and prints their reason.

### Retail city goodie, grunt shields, and draw buffer (2026-09-27)

- The retail `megawasher` goodie now has a distinct PC collectable ID after the
  existing weapon range. Its retail table supplies 25 washers on pickup; ordinary
  washers still supply one. This keeps goodie-bag spawning on the correct table.
- Grunts now parse the retail `Shield=On/Off` property. Shielded grunts use the
  `GruntShield` armor profile and the recharge fields in the retail `b_grunt`
  table, following the same `CEShield` lifecycle as Titans. Shield appearance
  and combat behavior have not been inspected interactively.
- The PC dynamic fdraw vertex buffer is 8,192 vertices instead of 2,048, reducing
  expected DISCARD wraps. This has not been proven to eliminate rare D3D lock
  waits. The front-end static mesh-load lock stall is a separate path.
- Debug and Release build. An isolated 50-second muted Debug `WEMCcity_01` run
  loaded, displayed gameplay (`build/shots/retail_city_fixes/latest.png`), and
  logged no crash, assert, script or audio error, or long-frame stall. Its old
  `megawasher` and grunt `Shield` parser errors are gone. Two retail
  `disablevelocityimpulses` commands still log as unknown.

### Information item counts and pause-page scope (2026-09-27)

- The active pause build sets `_4_SCREEN_SETUP` to FALSE, intentionally using
  Options and Information only. Primary/Secondary Equipment screens in source
  are inactive in this configuration; previous attempts to reach them were
  misclassified as incomplete navigation tests. Do not enable four-page mode
  solely for a visual test.
- PC item counter text now uses the slot's text-area scale rather than fixed
  retail texels. Isolated muted Debug captures `pc_info_counts_wide`
  (1920x1080) and `pc_info_counts_classic` (1280x960) show the counters readable
  inside their item slots. The Washer description and objective remain contained.
- Debug/Release builds pass. No crash/assert/audio/script errors in these runs.
  Physical pointer and longer item descriptions are still pending.

### Information page readability (2026-09-27)

- PC pause Information page item descriptions and mission objectives no longer
  force one-to-one retail font pixels. They use their existing text-area line
  sizing and wrapping; console builds retain the retail formatting.
- Debug/Release builds pass. Isolated muted Debug captures
  `pc_info_readable_wide` (1920x1080) and `pc_info_readable_classic`
  (1280x960) show the Washer description and Mines objective readable and
  contained within their panels. Logs show no crash/assert/audio/script errors.
- `pc_inventory_primary` and `pc_inventory_info` both reached Information via
  scripted pause-page keys; the later section clarifies the two-page build.
  Longer item text, physical pointer and live resize remain pending.

### Widescreen pause layout correction (2026-09-27)

- Pause fdraw layout and mouse cursor now scale x from half-width and y from
  half-height / 0.75, matching the retail normalized coordinate range. The old
  width-only scale pushed highlights and pad prompts off their text at 16:9.
- The helper supplies matching full forward/inverse matrices, confined to 2D
  fdraw. CFXfm uniform-scale metadata is not used for sphere/normal calculations
  in these passes. Generated PlayStation symbols additionally compensate their
  local x scale to stay round while the layout fills the display.
- Debug/Release builds pass. Debug captures `pc_pause_wide` (1920x1080,
  PlayStation) and `pc_pause_classic` (1280x960, keyboard) confirm aligned Options
  highlights, frame and unclipped prompts. `pc_pause_wide_final/options.png`
  confirms round PlayStation symbols after the final correction.
  Its final capture also verifies Advanced Settings Cross/Circle footers at
  1080p. No crashes/asserts/audio/script errors; loading/Present stalls remain.
- Tests use muted isolated instances. Physical pointer alignment, inventory
  pages beyond Information, Xbox art proportions, alternate aspect ratios and
  live resize still require coverage. This is not a claim of complete widescreen UI support.

### Xbox dialog alignment and two-button coverage (2026-09-27)

- Xbox dialog prompts retain their retail atlas art but now use the measured
  action label's center and height, with allowance for the atlas cell padding.
  This matches the placement used by the generated PC themes.
- Debug/Release builds passed. Debug `pc_dialog_xbox_confirm` and
  `pc_dialog_playstation_confirm` visibly reached the Quit confirmation at
  1920x1080: A/B and Cross/Circle align with Accept/Cancel. Final captures are
  in build/shots/<run>/latest.png; corresponding logs have no crash/assert,
  audio or script errors. Existing loading/render hitches remain.
- Runs were muted and isolated, ended after 57 seconds, and did not accept the
  quit confirmation. Physical pads and alternate Y/Triangle remain unverified.

### Widescreen pause-menu issue found (2026-09-27)

- 1920x1080 captures in `pc_dialog_xbox_pair` and
  `pc_dialog_playstation_pair` show the pause Options highlight displaced from
  its label and bottom/top pad prompts clipped. This is outside the fixed dialog
  coordinate path: do not claim the full pause screen is widescreen-correct.
- Starting point: CPauseScreen::Draw uses xfmTemp.BuildScale(HalfRes.x) for both
  axes of a layout whose y range is +/-0.75. The cursor repeats that uniform
  scale. Audit those transforms, the ortho viewport and wrapper entry/return
  state together; preserve the working 4:3 layout and mouse hit coordinates.
- Script navigation: W/S drive pause-menu selection. Arrow keys drive the right
  stick and did not move selection in the initial pair of runs.

### PlayStation and widescreen dialogs (2026-09-27)

- Fixed PlayStation message-box glyphs using centered coordinates in a top-left
  pixel viewport. The glyph helper now accepts y-down drawing (including an
  upright Triangle); regular wrapper callers keep y-up coordinates.
- Widescreen verification exposed dialog text using display aspect instead of
  the PC ftext 0..0.75 vertical range. Corrected title, body and all three action
  labels under MA_PC_INPUT. This prevents text drifting above its dialog at 16:9.
- Dialogs now measure each action label before drawing generated prompts. Their
  center and right edge follow the actual text bounds; PlayStation icon radius
  follows the text height. Keyboard keycaps retain their capped readable scale.
- Debug/Release builds pass. Muted isolated Debug captures verify the Accept
  Cross at 1920x1080 (`pc_dialog_ps_aligned`) and keyboard Space at 1280x960
  (`pc_dialog_keyboard_aligned`), under build/shots and build/logs. Both reached
  the settings write-failure warning without crashes/asserts/audio/script errors.
  Loading/render hitches remain. Physical controller input, Circle/Triangle
  dialogs and the Xbox dialog theme still need runtime coverage.

### Dialog keyboard keycaps (2026-09-27)

- Message boxes used top-left screen pixels but the generated keyboard keycap
  helper drew centered y-up geometry. Text appeared, while the raised key and
  border were misplaced. Added an explicit screen-pixel mode to both keycap
  helpers; message boxes select it and wrapper callers retain their default.
  Keycap drawing restores culling after supporting either coordinate direction.
- Debug/Release builds passed. Muted, isolated Debug `pc_dialog_keycap` capture
  `build/shots/pc_dialog_keycap/warning-keycap.png` confirms the Space keycap
  border and label align next to Accept; earlier frames show normal wrapper
  keycaps intact. No crash/assert/audio/script errors; existing stalls remain.
- The PlayStation coordinate follow-up is addressed in the newer section above;
  that section records the remaining theme coverage limits.

### PC settings write-failure feedback (2026-09-27)

- Advanced Settings now checks both PC preference writes before leaving, in the
  frontend and pause wrapper. Locked launch overrides are skipped. A failed
  write shows the existing warning dialog once per visit; dismissing it and
  selecting Back again retries and permits leaving with session values intact.
- Fixed keyboard message-box prompts overlapping their action text: key labels
  now have a capped scale and align to the retail glyph's right edge. Xbox and
  PlayStation glyph paths are unchanged.
- Debug/Release builds passed. Isolated Debug `pc_settings_final` deliberately
  blocked the settings directory with a file. Visually confirmed the fitted
  warning (`build/shots/pc_settings_final/warning.png`). The earlier
  `pc_settings_warning_fixed` run confirmed dismissal and return to Options.
  No crash/assert/audio/script errors. Loading/Present stalls still occur.
- Test uses -mute, isolated saves and LOCALAPPDATA. Physical pad operation and
  frontend runtime coverage of this warning remain pending; both wrapper paths
  share the same helper. No changes to actual checkpoint write-failure handling.

### Save-folder failure UI (2026-09-27)

- Verified the Campaign shortcut's error fallback with a file deliberately used
  as `-save-dir`: backend logged No usable save directory and the menu stayed
  usable. This fixture is under ignored build/test-saves; user saves unaffected.
- On PC, the no-device selection now reads Play Without Saving, hides meaningless
  free-space/profile zeroes, and explains an unavailable folder plus the lack of
  progress saving. If the folder is available, it instead explains how to choose
  Save Folder. Existing confirmation/launch behavior is unchanged.
- Release/Debug builds passed. Debug `pc_save_failure_ui` screenshot confirms the
  final layout and failure message (`build/shots/pc_save_failure_ui/latest.png`).
  No crashes/asserts/script/audio errors; the injected storage initialization error
  is expected. Startup stalls remain. This checks the fallback UI, not checkpoint
  write failures or the full no-save campaign flow. All test processes closed.

### Input handoff and profile verification (2026-09-27)

- `_MouseOtherInput` now clears the wrapper's cached same-frame hover, click,
  right-click, wheel and tick-drag state after keyboard/controller navigation.
  Previously it only hid the pointer, leaving `_MouseHoverSelect` able to apply
  cached mouse input after a key/stick selection. This is a source-confirmed
  conflict; it is not established as the cause of earlier test-route variations.
- Release and Debug builds passed. `pc_profile_direct` (muted, isolated settings
  and saves; keys `10:0x20,14:0x1B,22:0x20`) visibly reached Select Profile directly
  from Campaign, with proper space between Save Location and Save Folder.
  Screenshot: `build/shots/pc_profile_direct/latest.png`. No crash/assert/script/
  audio errors; startup stalls remain. This closes the profile-label visual check.
- Earlier `pc_profile_spacing` stopped at the main menu, so it is not profile
  verification. All runs ended and no game/test processes remain. Simultaneous
  physical mouse + controller interaction is still not exercised.

### Main-menu style follow-up (2026-09-27)

- User requested matching the existing label style. Generated Co-op and Quit labels
  now use the already loaded angular display font (slot 3), italic tilt, a blue
  outline and layered gold face/shadow. Selected labels reuse the retail green
  highlight mesh centered on the measured text bounds. Labels remain gold when
  selected. The font uses uppercase glyphs; these approximate the mesh lettering,
  rather than replacing or shipping extracted retail assets.
- Release/Debug builds passed. Muted `pc_menu_style` visually confirms the labels
  and selected Quit highlight in `build/shots/pc_menu_style/latest.png`; Quit
  exited by itself with code 0. No crashes/asserts/audio/script errors; startup
  and shutdown stalls remain. Test used isolated settings/saves and closed.
- Added and passed input checks for all four controller slots, separate keyboard
  routing, restoring configured layout, and invalid session-layout fallback.
  These validate mapping logic, not physical multi-controller gameplay.
- User is away: continue automated verification, record physical pad/audio checks
  as pending, and do not wait for user verification to advance independent work.

### Desktop exit follow-up (2026-09-27)

- Added PC-only Quit to Desktop to the main menu, using gameloop_ScheduleExit()
  for ordinary game-loop and application teardown. Mouse hit target and existing
  keyboard/controller navigation include the new fourth item.
- Main-menu drawing makes a temporary copy of the three retail mesh layouts and
  moves the two selection meshes up. Co-op and Quit fit below them; loaded retail
  layout data is not modified. No change to console menu entries.
- Release and Debug builds succeeded. Muted `pc_desktop_quit` with isolated saves
  and settings selected Quit and exited by itself with code 0 before the runner
  timeout. No crashes/asserts/audio/script errors; startup loading stalls remain.
  `build/shots/pc_desktop_quit/latest.png` visibly confirms all four menu entries
  and the selected Quit button. Physical controller and pointer acceptance were
  not exercised by this scripted-key run. No game/test processes remain.

### Controller-first co-op menu follow-up (2026-09-27)

- User requires controller-only co-op. Menu now defaults to Controllers, mapping
  pads 1-4 to P1-P4 (keyboard optional for P1), with a Keyboard + controllers
  alternative and Players 2-4 selector. No progress saving; networking inactive.
- CLI co-op no longer rejects shared layout or forces separate layout. Normal
  configured layout applies, shared by default. Use `-input-layout separate` for
  keyboard P1 + pads P2-P4. Temporary menu routing restores configured input at
  startup reset. `docs/coop-network-plan.md` describes current behavior.
- Added sparse main-menu navigation logs. `pc_menu_trace` proved co-op entry.
  `pc_coop_controls` proved Back, re-entry and a two-player controllers-layout
  launch into Mines. Split-screen rendered; the expected missing-P2-controller
  prompt appeared. No crashes/asserts/audio/script errors; loading stalls remain.
  The scripted route did not prove four-player launch or switching controls.
- All finite runs used mute, Discord off, isolated saves/settings and ended.
  Physical multi-pad play remains unverified; do not expand gameplay scope.
- Co-op page now uses standard Accept/Back prompts. Removed an attempted borrowed
  panel mesh after its capture showed rendering artifacts; use a dimmed animated
  background instead. Final dimming change builds, visual check pending.

### Co-op menu prototype (2026-09-27)

- PC main menu adds Co-op with a separate Start Local Co-op / Back screen. It
  clearly labels two-player local play as experimental, no progress saves, and
  network play as unavailable. Start uses campaign level 0 and virtual profiles.
- Keyboard/mouse owns player 1; first controller owns player 2. A temporary input
  routing override restores the configured layout at startup-menu reset.
- Release and Debug builds succeeded. Follow-up captures found the initial Co-op
  label offscreen: ftext Y uses a 0..0.75 coordinate range. Fixed the new menu's
  positions, explicit colors, and two-decimal scale escapes. Main-menu label now
  visibly renders (`build/shots/pc_coop_menu_checked/submenu-check.png` is actually
  a main-menu capture). Co-op submenu navigation/launch remains unverified: the
  scripted routes reached main/PvP join instead. Do not count these as co-op tests.
- Finite muted runs `pc_coop_menu_visual`, `pc_coop_menu_layout`,
  `pc_coop_menu_checked`, and `pc_menu_routes` ended without logged crashes,
  asserts, audio or script errors. Startup loading stalls remain. No personal
  saves/settings used. All runners and games closed.
- Source follow-up: reacquire controller selection after hotplug in the co-op
  submenu; initialize virtual profiles with InitNewProfile(TRUE), clear their
  storage metadata, and clear the prior profile device before the no-save launch.
  These changes build but still require runtime verification.
- Profile screen now positions the save-folder name after the measured label
  width (when the label is the final text row), fixing the character-count spacing
  estimate. Visual verification of this change is pending.
- Changes remain uncommitted with the earlier PC menu work.
- Read `docs/coop-network-plan.md` for source findings and staged network design.
  No networking is implemented. AI, cutscenes and campaign rules still have the
  limitations in `docs/coop-audit.md`; keep further gameplay work out of this pass.

### PC input polish follow-up (2026-09-27)

- Advanced Settings now has a **Mouse Sensitivity** row in the front end and pause settings.
  It shows a multiplier of the default 0.1 degrees/count, changes in 0.10x steps (0.10xÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¾ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¾ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€šÃ‚Â ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¾Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Â¦Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬ÃƒÂ¢Ã¢â‚¬Å¾Ã‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¡ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Â ÃƒÂ¢Ã¢â€šÂ¬Ã¢â€žÂ¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¦ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã‚Â¦ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã¢â‚¬Å“10.00x),
  and saves on Back/Accept under Local AppData alongside Button Prompts. The profile's existing
  Look Sensitivity still multiplies mouse look. Valid launch/environment overrides lock the row
  and do not overwrite the saved preference. Invalid settings fall back to the default.
- `pcinput_Sample` previously discarded scripted test keys whenever the window lacked focus,
  contradicting the documented test recipe. Scripted keys now bypass that gate; the physical
  controller sample is zeroed first so real background input remains blocked.
- Debug and Release builds succeeded. `ma_input_tests` passed, including isolated settings-file
  round trips, invalid values, override precedence/protection, and background scripted input.
  The Release `pc_mouse_verified` run navigated to the new row, changed it from 1.00x to 1.10x,
  and saved `MouseSensitivity=0.110000` on Escape. No crashes, asserts, script or audio errors;
  one 110 ms Present stall remains. All test processes and runners were confirmed stopped.
  Captures and logs are under `build/shots/pc_mouse_verified/` and `build/logs/pc_mouse_verified.log`.
  All private runs used `-mute` with separate test settings; no listening check was performed.
- Pause Advanced Settings and Audio Levels now use the localized Back footer, matching their
  save-on-exit behavior and the front-end label. Debug/Release builds passed; `pc_back_footer`
  visibly confirmed the new footer without crashes/asserts/script/audio errors (one Present stall).
- `menu_drive.py` now refuses pointer actions when the target window is unfocused or minimized,
  instead of reporting success for input the game discards. Its documentation now states the focus
  requirement. Use `-test-keys` for unattended background navigation.
- Front-end checks (`pc_front_settings`, `pc_front_existing`) created an isolated profile and loaded
  it again to the Launch screen. Advanced Settings there still needs a visual check; posted pointer
  input stopped working when focus changed. One capture showed a Reset confirmation; No was selected,
  and the existing profile loaded afterwards. Its trigger was not established. All tests were muted,
  used `build/test-saves/`, and their processes were closed. Retail front-end and pause layout tables
  have identical option-row positions, but this alone does not verify front-end runtime behavior.

### This session (2026-09-27, final pass), newest first

Commits `9a0268a`, `fcdaeb0`, `8241ffb`, `84cdfae`, all pushed.

1. **Mission sweep fixes** (`84cdfae`), from running all 42 missions and grouping their log errors:
   - **Crash opening Shady's shop in `wessstatn01`** (`BarterTypes.cpp`, `CBarterLevel::InitLevel`): the
     sale-chain array was sized to the candidate items alone, so a level listing more chains wrote past
     it, and chains left without an item were read. Now sized chains + candidates, the fill is bounded,
     and `m_nSaleChains` is the number filled. **Not yet confirmed in play by the user.**
   - **Audio assert in `WEWJjourn01`**: an emitter with an outer radius below DirectSound's minimum;
     `Create3D` clamps it to `DS3D_DEFAULTMINDISTANCE` (`fdx8audio.cpp`).
   - Budgets raised for the PC (`fang.cpp`, WINGC): collision impacts 4096, decals 400 / 6000 vertices
     (both ran out in busy fights).
   - Log noise removed where the retail game ignores the same data: retired items (Mil Translator,
     Antenna, EUK, Mission Briefing) in `ItemInst.cpp`; weight class `none` and the BotDie lookup in
     `bot.cpp`; sound group `None` in `fsound.cpp`; a missing intro movie in `level.cpp`.
   - `-mute`, `tools/mission_parallel.py` options (`--quiet`, `--test-keys`, `--no-audio`) and the new
     `tools/log_errors.py`.
2. **Campaign co-op** (`8241ffb`): start points (`CStartPtMgr::InitLevel`: 3 units beside or 4 behind
   player 1 instead of inside walls), `_CoopRespawnNearPartner` (`player.cpp`: a player who dies or falls
   out comes back beside a standing partner; the level checkpoint restore runs only when nobody is
   standing), `_ScriptPlayer()` / `_ScriptSetPlayersControl` (`MAScriptTypes.cpp`: scripts and cutscenes
   act on player 1 and freeze everyone). Split screen verified; respawn not yet exercised.
3. **AA gun aims with the mouse** (`fcdaeb0`, `botAAgun.cpp`): it only read the right stick; now adds
   `TakeMouseLookDelta` to heading and pitch. User confirmed.
4. **Gameplay audio after the intro movies** (`9a0268a`, `fdx8audio.cpp`): once Bink's heap allocation
   let the 110 MB intro play, emitters paused by it came back without voices:
   - `CFAudioEmitter::Pause(FALSE)` on a voiceless emitter now requests PLAY (it asked to unpause a voice
     it did not have);
   - `_ResumeStrandedEmitters()` (start of `faudio_Work`) resumes emitters left paused above the
     current pause level;
   - 3D voices are allocated for emitters already in range (listener state PRESENT/SWITCHED), not only
     on ENTERED.
   The `PORT-MIX` snapshot now counts voiced vs. voiceless emitters and names in-range voiceless ones.
5. **Q/R weapon lists no longer flicker** (`9a0268a`, `gamepad.cpp`): the weapon list pauses the game
   loop, and mouse look (hence menu mode) was keyed off any pause; now off the pause menu only
   (`pausescreen_IsActive()`).

Earlier this day (details in `docs/handoff-history.md` section 25): D3D9Ex and the dark-legs fix
(another model), the pointer in the pause menu and its settings screens, clickable settings, the
controller chart, flush-left prompts, Back keeps settings, the hitch fixes (async log, stream worker,
Bink read-ahead), the Release build, Discord, test tooling, and another model's pass (Xbox menu map on
PC, prompt settings, `-coop`, `-asset-log`, `mission_parallel.py`, checkpoint write hardening).

### Mission sweep

The last full sweep, `sweep3`, after all the fixes above:

SWEEP3_RESULT

Warnings that remain in the logs (all reproduce retail data the source does not handle; none crash):

- **`Unknown command 'shield'`** (hundreds of lines, 12 levels): retail grunts have a shield the source
  lacks. `main.dol` has `GruntShield`; the Titan's shield (`CEShield`, `bottitan.cpp`, "TitanShield")
  shows how a bot owns one. This is a missing retail feature, the most visible item left.
- Explosion `rocket`, texture `tfa1sawstr4`, a duplicate `laserl3` decal table, sounds `Door Locked` and
  `SOM_MOpen`, the `Vehicle` bank (absent from the GameCube data), `megawasher`, `debrisshakespersec`,
  `tripwirewho`, `AI_ATTACKWHO PLAYER`, `AI_RACE good`, barter `EUK` names and `Battery 2`-`6`.
  Check the retail tables (`tools/gamedata_dump.py`) and `main.dol` strings before mapping any of them.

## Diagnostics cookbook

Tests here keep Discord off. Audio is on by default; pass `--mute` only for intentionally silent runs.
Logs go to `build/logs/`.

**One private test run, summarized** (Release by default; `--config Debug` for asserts):

    python tools/port_run.py --mission wedmmines01 --seconds 60
    python tools/port_run.py --config Debug --mission wessstatn01 --seconds 90 --name shop_test

The summary lists `PORT-PERF` lines (fps, worst frame, work before Present), the longest hitches, each
`PORT-STALL` stack, crash/assert/run-time-check reports and audio errors.

**Screenshots** (engine captures of the back buffer; cost frame time, never in the user's session):

    python tools/port_run.py --mission WEDTtown_01 --seconds 40 --shots 120
    build\Release\ma_port.exe -data gamedata\files -mission wedmmines01 -discord-app-id off -shots build\shots\mines -shot-every 150

`port_run.py --shots N` clears `build/shots/NAME/`, and saves the newest frame as `latest.png`.
Read the PNG/BMP to look at it.

**Scripted keys** (no focus needed):

    python tools/port_run.py --mission wedmmines01 --seconds 30 --shots 60 --test-keys "g8:0x1B"
    python tools/port_run.py --seconds 60 --test-keys "10:0x20,14:0x20,30:0x0D"

The first pauses 8 s into gameplay (pause-menu tests); the second skips the logo movies and presses
Enter in the front end.

**Driving menus with the mouse** (posts window messages; never moves the desktop cursor):

    python tools/port_run.py --mission wedmmines01 --keep --shots 30 --test-keys "g8:0x1B" --name pause
    python tools/menu_drive.py --pid PID waitpause build/shots/pause 60
    python tools/menu_drive.py --pid PID click 630 296
    python tools/port_run.py --stop PID --name pause

The target game must be focused for posted pointer input; `-test-keys` works in the background.
Always pass `--pid`: without it the first game window found is used, which may be the user's. Fixed
coordinates are layout-dependent; confirm the screen in a capture.

**All missions in parallel** (up to 4 instances on the user's PC; use 3 while they play):

    python tools/mission_parallel.py --config Debug --seconds 90 --jobs 3 --run-name sweepN --quiet --test-keys "10:0x20,14:0x20,18:0x20,24:0x20,30:0x20,40:0x20" MISSIONS...
    python tools/log_errors.py "build/logs/sweepN_*.log"

The 42 missions: `WECDsneak01 WEDMmines01 WEDTtown_01 WEMCcity_01 WERRreactr1 WEWCcomm_01 WEWHchase01
WEWJjourn01 WEWZzombi01 webccolis01 webccolis02 webccolis03 webccolis04 wecdsneak02 wecffacty01
wecrruins01 wecrruins02 wediinvas01 wedmmines02 wedmmines03 wemccity_02 wemccity_03 wemccity_05
wermmorbot1 wermmorbot2 werrreactr2 wesccorros1 weshhangr01 wesrrepair1 wessstatn01 wessstatn02
wewccomm_02 wewccomm_03 wewchold_01 wewjjourn02 wewjjourn03 wewkrockt01 wewrresrch1 wewrresrch2
wewrresrch3 wewrresrch4 wewtrace_01`. A 42-mission sweep at 3 jobs takes about 25 minutes; run it with
a background shell or a second terminal and wait for it to finish rather than polling. `log_errors.py` lists
crashes/asserts per log first, then each distinct message with how many logs and lines have it.

**Co-op:** `python tools/port_run.py --mission WEDTtown_01 --coop 2 --shots 120` (split screen; player
1 on keyboard/mouse, players 2-4 on pads).

**Performance and stalls:** runs with `-port-diag -no-vsync` (what `port_run.py` passes) log
`PORT-PERF` every 10 s. A frame over `MA_PORT_STALL_MS` (default 100) makes the stall sampler suspend
the game thread and log its symbolized stack as `PORT-STALL`; `--stall-ms 50` lowers it.

**Audio:**

- `-port-diag` logs `PORT-MIX` snapshots every 2 s: active and voiced emitters, free voices, pause
  levels, each sound's level, and in-range playing emitters that have no voice ("voiceless"). Also
  `PORT-SND`, `PORT-TALK` (bot dialog), `PORT-DUCK`.
- Is a process audible? Test tools now enable audio by default; pass `--mute` only for intentional
  silence, then meter it:
  `powershell -ExecutionPolicy Bypass -File tools\audio_meter.ps1 -ProcessId PID -Seconds 30`.
  A `-mute` run meters 0 by design; so does `-no-audio`.

**Crashes and hangs:** crashes and the first of each assert log a symbolized stack (asserts are
rate-limited: first 10, then 100, 1000...). A run that looks hung is usually a dialog: check
`Get-Process -Name ma_port | select Id, MainWindowTitle`.

**Retail evidence** (settle schema questions from the retail data, not guesses; write outputs under
`build/`): `tools/mst_list.py` (list/extract the `.mst`), `tools/gamedata_dump.py` (binary `.csv` tables
as JSON), `tools/dol_vocab.py` (table vocabularies from `main.dol`), `tools/dol_xref.py` (PowerPC code
referencing an address), and a plain string search of `gamedata/sys/main.dol`.

**Workflow for a fix:** reproduce in a private Debug run (or a sweep) -> patch -> build Debug and
Release -> rerun the affected missions -> `log_errors.py` on the new logs -> `tools/eol.py check` ->
commit (trailer) -> check no retail data is tracked -> push. Tell the user what to try in their session.

## Working without Windows (cloud / Linux sessions)

No MSVC, no retail data, no game runs or logs. What works:

- `python3 tools/syntax_check.py [--changed REF | FILE...]`: clang against MinGW headers with the
  MSVC build's settings; all C/C++ files pass. Catches type errors and API misuse; not a substitute
  for an MSVC build. Needs `clang mingw-w64-i686-dev g++-mingw-w64-i686-win32`.
- `python3 tools/mathdiff/mathdiff.py`: runs the GC-layout math (`dx/fdx8gcmath_*.inl`, what this build
  uses) and the shipped SSE math on the same inputs and reports differences. Needs
  `clang gcc-multilib g++-multilib`.
- Everything a cloud session changes goes in `CLOUD_SESSION_LOG.md` with what to verify; the user
  builds, runs and reports back.

## Where things are (code map)

| Topic | Where |
|---|---|
| Options, logging, stall sampler, test keys, crash reports | `port/main_win.cpp` |
| Input mapping, menu mode, pointer, text input, prompt style | `port/pc_input.cpp`; `ma/App/ma/gamepad.cpp` (`gamepad_Sample`: control maps, `allowLook`) |
| Mouse in menus, settings clicks, controller chart | `ma/App/ma/wpr_system.cpp` ("mouse pointer (PC port)", `_MouseAdd*`, `_PcControllerMap`) |
| Prompt icons and key caps | `ma/App/ma/wpr_drawutils.cpp` (`wpr_drawutils_DrawButtonOverlay`, `_DrawKeyCap`) |
| Pause menu | `ma/App/ma/PauseScreen.cpp` (`CPauseScreen::Work`, `pausescreen_IsActive`) |
| In-game PC wording | `ma/App/ma/game.cpp` (`_aPcPhrases`, `game_PcPromptWork`), retail phrase maps `_anRetailPhraseField` / `_anRetailGamePhraseField` |
| Audio emitters, voices, GameCube volume chain, stream worker | `ma/Lib/Fang2/dx/fdx8audio.cpp` |
| Movies | `ma/Lib/Fang2/dx/fdx8movie2.cpp` (`_MovieAlloc`, `BINKIOSIZE`) |
| D3D8-on-D3D9Ex, screenshots | `port/compat/d3d8_compat.cpp` |
| Co-op | `launcher.cpp` (`-coop` init), `MultiplayerMgr.cpp` (`CStartPtMgr::InitLevel`), `player.cpp` (`_CoopRespawnNearPartner`), `MAScriptTypes.cpp` (`_ScriptPlayer`) |
| Budgets | `ma/Lib/Fang2/fang.cpp` (WINGC block) |

## How to work with this user (important)

- The user plays the game windows you launch, **while you work**, and reports by ear and eye; they
  cannot read logs. Their session is Release, audible, with Discord on, launched like:
  `build\Release\ma_port.exe -data gamedata\files -mission WORLD -port-diag -log build\logs\play_NAME.log`
  (`-port-diag` so you can read what happened in their session afterwards).
- **Test instances keep audio enabled by default** with `-discord-app-id off`; this user asked that
  play tests not be muted. The test tools now follow that preference. Pass `--mute` only for a run
  where silent playback is intentional, or `--no-audio` if testing without audio setup. Window titles
  identify the audio mode. Close test windows when their check is done.
- Never kill the user's game except to relink its exe: then stop that exact PID, rebuild, and relaunch
  it for them without being asked (they asked for this). Tools only ever close the PIDs they started.
- The user prefers less time on visual checks: capture a frame when a change is visual, don't iterate
  on pixel details unless they ask.
- Keep replies short and concrete: what changed, what to try in game.
- Edit CRLF sources with the Edit tool or a script that keeps line endings (`sed -i` in Git Bash strips
  CRs). Build C string edits with the Edit tool: shell heredocs mangle backslash escapes.

## Open work, roughly in priority order

1. **Vertex-buffer stalls**: the PC dynamic fdraw buffer is now 8,192 vertices. The old
   `build/logs/coop2.log` has a ~110 ms zipline `fdx8vb_Lock` sample, but fresh timing in
   `build/logs/pc_vb_town_coop_diag.log` shows 15-16 ms individual locks and a separate Present
   stall. Capture more startup samples before changing buffer allocation or lock flags. The
   `rel_front2.log` wait is a static mesh-load VB lock.
3. **Remaining manual checks**: Shady's shop in `wessstatn01` opens; controller hot-swapping;
   fullscreen/resize under D3D9Ex; the PC Button Prompts setting persists; additional liquid surfaces.
   Co-op save/reload, deaths/checkpoint animation, partner cutscene cameras and alt-tab are user
   verified as of 2026-09-30.
4. **Campaign playthrough**: verify progression without softlocks, including co-op barter,
   collectables, AI targeting, minigames and bosses. `docs/coop-audit.md` retains the source audit.
5. `L02_rslide3` playback is not confirmed broken; earlier `PORT-MIX` voiceless reports were
   diagnostic false positives for emitters whose listener had EXITED. The log condition is corrected.
6. The remaining retail-data warnings (see "Mission sweep" and latest pass above), including
   malformed `ColorRed` and `goodie` entries and LiquidMesh `dropfreq`. The `setdamageable`,
   `noliftblockchecking`, and `useby` world entries are handled.
7. The reported pause between front-end logo movies is not reproduced as a 1 s Bink open/close: the
   latest isolated timing measured opens at 47 ms and closes at 0 ms. Reproduce and profile the visible
   transition before changing movie startup.
8. Older items: the save flow from the menus; the laser's charged burst and other weapons' particle and
   sound fields; `Difficulty.csv` extra fields; failed-load teardown beyond `CLOUD_SESSION_LOG.md` 5/10;
   a Discord image asset; 64-bit, widescreen, rumble.

## Things that cost time before

- Don't re-investigate solved problems: fonts, skinned meshes, the world-origin bone, world collision,
  dark legs (normal sphere), gameplay audio after the intro (paused voiceless emitters).
- "No audio" reports: first check whether the process was launched with `-mute`/`-no-audio` (title), then
  the in-game volume (Back used to cancel settings changes), then `PORT-MIX` voiced vs. voiceless.
- The `we01multi01` restart loop was never explained (it is a multiplayer map launched through the
  debug path); test with campaign levels (`-mission`) instead.
- Sweeps with `-no-audio` cannot diagnose audio; normal runs and sweeps are audible by default.
- A backgrounded shell loop outlived its task and kept opening windows: use the tools, which close
  their own processes.
- Retail data drift is the usual cause of "wrong text/sound/value": the retail tables were reordered
  or extended after this source snapshot. Dump the retail table and compare with the source enum before
  changing code; check `main.dol` for names and tables.
- `PauseScreen.h` cannot be included from `gamepad.cpp` (it redefines `_PLATFORM_XB`); use the
  `pausescreen_IsActive()` accessor.
