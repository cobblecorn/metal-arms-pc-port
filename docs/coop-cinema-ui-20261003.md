# Co-op cinematic views and join panels (2026-10-03)

The co-op join menu used retail panel meshes scaled by screen width. Their height grew with
the aspect ratio, overlapping the player rows in widescreen/fullscreen. Co-op now draws dark
panels in screen fractions with one-to-three-pixel borders. Retail text, profile selection,
controller joining and PvP's original panel meshes remain in use.

Partners previously used a separate camera aimed at the story actor during scripted scenes.
They now borrow the active cinematic camera's live transform, shake, vertical field of view
and clipping planes. Each viewport keeps its own screen rectangle; horizontal coverage adapts
to its aspect ratio. This renders the cinematic independently in each existing split rather
than replacing the display with one player's screen.

Cutscene/manual script cameras use P1's camera. Scenes holding the actor's normal camera use
the current story actor, including the living substitute when P1 is dead. Partners retain
their own camera controllers and projection underneath the borrowed view, so returning to
normal play restores vehicle, turret and on-foot cameras without replacing their state.
Camera links and cached player perspective viewports reset on pause and level teardown.
The change does not move or freeze player bodies. Fullscreen movie playback is unchanged.

## Verification

- `tools/test_coop_cinema_ui.py`: 851 offline checks using production camera/coordinator/panel
  methods. Covers two through four players, live pose updates, source changes, dead partners,
  pause/resume/cleanup, cycle rejection, lens/frustum restoration and 640x480 through 3840x2160
  plus ultrawide panel/viewport proportions.
- Existing fixtures: scene restore 434, campaign playthrough 328, PvP join routing 145,
  waterfall rendering 6,901.
- Debug and staged Release builds passed. Installed EXE/PDB/Bink in normal `build/Release`
  while the game was closed, with matching staged/live hashes. Backup:
  `build/backups/pre-coop-cinema-ui-20261003-173526`. Hashes are recorded in HANDOFF.md.
- No game was launched. Actual cinematic composition and menu appearance need user confirmation.
