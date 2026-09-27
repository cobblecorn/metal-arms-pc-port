# Campaign co-op and network direction

## Current scope (2026-09-27)

PC main menu includes **Co-op**, with Start Local Co-op, Players (2-4),
Controls, and Back. Controllers is the default: pads 1-4 control players 1-4,
with keyboard/mouse optional for player 1. Keyboard + controllers instead reserves
player 1 for keyboard/mouse and maps the first three pads to players 2-4.
Accept/click cycles the settings; left/right adjusts them. Starting launches
campaign level 0 with campaign rules and virtual profiles. No progress is saved.
The temporary routing override clears on return to the startup menu.

CLI `-mission WORLD -coop 2..4` now accepts either `-input-layout shared` (pads
1-4, keyboard optional for P1) or `separate` (keyboard P1, pads P2-4). Without an
explicit layout, normal input configuration applies (shared by default).
Network play is planned and unavailable; the menu cannot host or connect.

Implementation: `wpr_system.cpp` main-menu work/draw/exit functions and
`_PrepareToLoad_Work`; `pc_input.cpp` temporary session routing. Retail menu data
is unchanged. The new Co-op label uses the game's font rather than a retail mesh.
Virtual profiles keep wrapper completion screens supplied with profile objects,
without enabling disk saves. Campaign completion and subsequent levels still
need runtime verification; this does not claim a fully working co-op campaign.

Release and Debug builds succeeded. `pc_menu_trace` verified entry to the co-op
page. `pc_coop_controls` exercised Back and launched campaign level 0 with two
players using controllers routing; split-screen rendered, followed by the expected
player-2 reconnect prompt with no second pad connected. No crashes, asserts,
audio or script errors were logged. Physical 2/4-pad gameplay, four-player menu
launch, and switching layouts still need verification. Further cutscene/AI work
remains outside the basic prototype scope. See HANDOFF.md for later polish.

## Findings from source

- `GameInitInfo_t` and `CPlayer` already support multiple players. Current controller
  indices, cameras, HUDs, listeners, and viewports assume these players are local.
- `splitscreen.cpp` sizes views using the total player count. A remote player must
  remain in the world without receiving a local viewport or audio listener.
- Script/cutscene changes already select the story player and freeze all players,
  but only player 1 gets the scripted camera. A remote client's view needs an
  explicit cinematic policy.
- Some AI and mission logic explicitly uses player 0. Examples and remaining
  work are in `coop-audit.md`; existing PvP support does not establish campaign
  enemy targeting correctness for player 2.
- No Winsock startup or datagram send/receive implementation was found in the
  inspected port, Fang2, and game sources. Existing local multiplayer is not a
  ready-made network session implementation.

## Proposed architecture (not implemented)

Use a host-authoritative campaign simulation. The host runs scripts, AI, damage,
physics decisions, pickups, checkpoints, and mission transitions. Clients send
bounded, sequenced player commands and receive world snapshots and events.
Do not run independent campaign scripts on every client and assume input replay
will keep them synchronized: deterministic timing, random streams, and simulation
ordering have not been established in this engine.

Separate three identities before connecting machines:

1. Stable session player ID: who exists in the campaign.
2. Owning peer ID: who can submit that player's commands.
3. Local view/input slot: which player this machine displays and controls.

Keep single-player and local co-op using the same session description with local
ownership. One remote-client viewport should use its owned player across camera,
HUD, reticle, listener, mouse look, and menus, without removing other players from
the world. Do not use a controller index as a network identity.

Snapshots need stable entity IDs plus generations, spawn/despawn messages, player
transforms and velocities, health, inventory and possession, and relevant vehicle,
projectile, door, and objective state. Use reliable ordered events for mission
transitions and one-time actions, and sequenced snapshots for changing state.
Client movement prediction/reconciliation and interpolation follow a working
host simulation; a transform-only demo is insufficient for campaign play.

Before accepting traffic: bound message sizes/counts, validate entity ownership,
reject invalid values and stale session IDs, and negotiate protocol/build/content
versions. Transmit identifiers and values, never native pointers or raw C++ object
memory. Keep retail assets on each machine. The host owns progress and save writes.

## Small milestones

1. Introduce session ownership and local-view selection while retaining current
   local play behavior. Audit every player-count-based rendering/audio decision.
2. Build a two-process LAN experiment in one ordinary campaign map: one host,
   one remote player, one full-screen view per machine, authoritative spawn,
   movement and disconnect cleanup. No matchmaking or internet exposure yet.
3. Replicate combat, enemy state, pickups, vehicles and mission events. Handle
   loading barriers, all-player death, checkpoints and reconnect policy.
4. Give cutscenes a shared start/end state and camera policy; audit bosses,
   minigames, barter, transitions and saving before exposing a network campaign.
5. Only then choose internet discovery, authentication, NAT traversal/relay and
   distribution integrations. Transport/library choice is deliberately deferred.

Networking is a substantial engine feature spanning these systems. The menu entry
reserves a product path; it does not supply replication infrastructure or imply
online campaign support. Gameplay expansion remains outside this basic menu pass.
