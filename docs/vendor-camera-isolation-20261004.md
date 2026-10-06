# Private vendor cameras in co-op (2026-10-04)

User reports P1 shopping at Shady/Slim now also forces the vendor camera onto
P2's split view. Before cinematic-camera sharing, P2 could roam independently.

`game_EnterBarterMode` disables only the shopper's controls and sets that
player's mode to `CONTROLMODE_BARTERSYSTEM`. The barter camera uses a manual
camera on the shopper's slot. `_CoopWatchCamerasWork` treated an alive story
player without entity control as a cinematic fallback, incorrectly selecting
that private vendor view as the camera source for every partner.

The sharing helper now rejects a selected source in barter mode after clearing
old source links. Every player's own transform/projection is restored as usual,
while the vendor retains the shopper's existing camera and controls. It also
excludes P1's vendor camera if a script-camera flag is still set. Actual
scripted/letterbox cinematics retain sharing; no barter input, purchases,
introduction history, or global cutscene state is modified.

Validation: existing production-method cinematic/menu fixture strengthened to
1,141 checks, covering two through four players and each possible shopper,
cinematic-to-shop transitions, stale script flags, independent owned views and
perspective pointers, partner control state, shop exit, and the next real
scripted camera. Debug/Release and scoped whitespace checks pass. Camera and
control objects are fixture mocks. Subsequent native vendor gameplay checks
verify independent views, partner entity controls and visibility, shopper-only
hiding, separate purchase funds, and normal exit (eight checks).

The initial build was staged while the game was open. The user later authorized
live testing; the update is now installed together with cached-player recovery.
CMake output overrides are reset. Tests used native game commands and back-buffer
snapshots with isolated saves. See `docs/coop-cached-player-recovery-20261004.md`
for rollout and logs. The first shop fixture disabled the partner before moving
them, so its mesh stayed at the old position; removing that test-only disable
restored normal model updates. No production visibility workaround was added.

The user's prior message also confirms the ten-kill asylum exit works in actual
co-op gameplay. Fourteen original spawns remain retained by that update.
