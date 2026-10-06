# Mil Comm Centre assertions (2026-10-04)

Three reports from the same live campaign session have separate, confirmed causes.
No game was launched, stopped, or interacted with during diagnosis/builds.

## Titan loot collision list overflow

Saved evidence: `build/logs/mil-collectable-skiplist-20261004.log`. World
`WEWCcomm_03` (Destroy Comm Arrays), stack:
`CCollectable::_HandleCollision -> CBotTitan::AppendTrackerSkipList ->
CWeaponChaingun::AppendTrackerSkipList` (line 900, 100-entry capacity assertion).
This is a pickup collision query, not a chaingun firing failure.

The pickup never reset the shared tracker skip count before appending its spawning
entity while rising. A Titan adds its body, shield, main and duplicate weapon, and
data port. Repeated queries accumulate these meshes, inherit exclusions from other
queries, and overflow the fixed array. `fcoll_Clear` only clears impact records.

PC `_HandleCollision` now resets the skip count before constructing each moving
pickup's owner exclusion list. Falling and ownerless pickups get an empty list;
the existing falling-owner release, gravity, world collisions, and impact response
remain unchanged. Stationary/floating pickups retain their early return. Capacity
and assertions are retained; no blanket collision suppression or enlarged buffer.

`tools/test_collectable_skiplist.py` compiles the production pickup collision,
Titan append, and chaingun append methods with instrumented collision fixtures.
Legacy mode reproduces overflow after 20 five-mesh queries (1 check). PC mode
passes 180,016 checks including 30,000 moving queries, a full preceding scratch
list, changing owners, ownerless/falling pickups, no-query states, and movement /
impact response. Collision geometry itself is mocked; gameplay still needs a
fresh build to confirm the assertion no longer occurs.

## Hold Your Ground co-op turret allocation

Saved evidence: `build/logs/hold-ground-allocator-20261004.log`. Loading
`WEWChold_01` asserts in `botAAgun.h:387`, directly called by
`_CreateCoopDefenseGuns` at `MG_HoldYourGround.cpp:270`. The added turret helper
incorrectly used plain `new` / `delete` on a Fang aligned resource-stack class.
Its debug/test allocator explicitly rejects these in favor of `fnew` / `fdelete`.
Continuing the assertion allowed loading; the user then confirmed both turrets
work cleanly with the mission. This confirms actual two-player gameplay, not
three/four-player positioning or complete mission/retry coverage.

The helper now uses Fang allocation and matching deletion on every creation-error
rollback path, and handles a null allocation before dereferencing it. Successful
turret layout, boarding, controls, and wave logic are unchanged. The subsequent
mission-end report exposed a separate missing world-ownership flag, repaired below.

`tools/test_coop_defense.py` now models Fang's rejection of plain `new`, enforces
paired Fang deletion, and checks creation and allocation failure at all three
partner indices. Its former fixture used unrestricted C++ allocation, which
missed this assertion. All 215 offline defense checks pass (1–4 players, seating,
independent cameras, readiness, retry, and rollback).

## Hold Your Ground mission-end crash

Saved evidence: `build/logs/hold-ground-teardown-20261004.log`. At mission completion,
`level_Unload -> fres_Debug_ReleaseFrame -> aibrainman_UninitSys` asserts that active
brains remain. Continuing logs undeleted brain, mover, weapon-control, animation,
and bot-part resources; the level-complete screen then crashes reading
`0xCDCDCE49` in `CEntity::ResolveEntityPointerFixups` (released memory).

`CEntity::_DestroyAll` only destroys/deletes entities flagged `AUTODELETE` before
the resource frame is released. Authored world entities receive that flag in the
world builder; the added runtime gun calls `Create` directly and never set it.
Neither the mission unload hook nor frame release can substitute for the entity's
normal world removal / brain destruction. Changing only `new` to `fnew` would not
have fixed this ownership omission.

`CBotAAGun::PortCreateDefenseGun` now marks each successfully created clone with
`SetAutoDelete(TRUE)`. The normal world destruction loop removes it, detaches
occupants, destroys its brain and dependent resources, and performs Fang deletion
before AI/resource teardown. Original authored gun, player ownership, checkpoint
restore, and wave logic are unchanged. The assertion and fixup checks remain.

New `tools/test_defense_teardown.py` compiles production `PortCreateDefenseGun`,
ownership setter, entity `Destroy`, world `_DestroyAll`, and gun destructor against
instrumented entity/brain/resource fixtures. It reproduces unowned clones surviving
teardown, then passes 6,008 checks across 100 repeated cycles for 1–4 guns, in/out
of world clones, seated occupant detachment, empty lists/brains before shutdown,
balanced allocation/deletion, and repeated cleanup. Full engine AI/resource
implementations are mocked; actual mission completion still needs gameplay checking.

## Build and delivery

- Debug and staged Release builds pass; retail loot parser/clone/drop 19 legacy /
  32 PC checks and campaign playthrough 328 checks pass.
- Scoped whitespace checks pass. Build logs are under `build/logs/mil-assertions-*`.
- User reports game fully crashed and requests installation. Process checks confirmed
  it closed; installed all three fixes together in normal `build/Release` with
  matching staged EXE/PDB/Bink hashes. No game launch or save modification.
- Backup: `build/backups/pre-mil-assertions-20261004-012201`.
- Installed/staged EXE SHA256:
  `A65212082F85E3BB5002E53A8973763744870C2FE257628A1D03765614670D1D`.
- Installed/staged PDB SHA256:
  `CFB64346A3B1500190FBE5442A048F9A6805FABDC12BB93E310FD389BBD00E0E`.
- Bink SHA256:
  `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`.
- CMake Release output overrides were reset after the build. Latest build logs:
  `build/logs/defense-teardown-*`. User's no-game-launch constraint remains in force.
