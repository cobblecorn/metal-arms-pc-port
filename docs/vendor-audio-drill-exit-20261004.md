# Co-op vendor music and You Know the Drill exit (2026-10-04)

The user heard a vendor greeting followed by persistent music while progressing
through `WEWRresrch4`. They confirmed no shop menu opened. The live log shows
the authored `add_titan1` encounter moving vendors to `barter2` and starting
`Barter_Attr`; that greeting is intended. Both players share audio, and the
authored attract radius is 135 world units. The track eventually stopped in
the user's log, so it was not proven permanently stuck in that particular run.

Confirmed co-op audio defects corrected:

- Idle vendor listener selection considered every barter location, including
  inactive ones. It now selects the living player nearest the actual occupied
  vendor point. An absent/dead listener cannot keep attraction playing.
- Vendor stop failed to release its mission-music pause, and used a generic
  speech stop that could kill a transmission that replaced the attract loop.
  Co-op stops only named vendor tracks, releases its pause, and waits for
  unrelated speech to finish before starting attraction again.
- Volume/pan and fades now address the named vendor stream, preserving radio
  speech that takes over the shared speech slot.
- Co-op shop entry pauses the mission track and uses the speech slot for its
  tune; the retail `StopAllStreams` path destroyed the mission track. Leaving
  the vendor resumes mission music rather than relying on a later script cue.

All changed behavior requires `IsLocalCoop()`; solo/PvP retain the old logic.
Greeting, shop action/input, prices, mission script data, and saves are unchanged.
`level_GetStreamByName` is a read-only lookup used for stream ownership.

Validation: production-method fixture 19 checks (2–4 player listener selection,
owned stream cleanup, pause release, leaving radius, transmission takeover,
fade isolation, shop mission preservation, and solo/PvP exclusion). Existing
cinema, cached-player, and playthrough fixtures pass 1,141, 39, and 328 checks.
Debug/Release builds and scoped whitespace checks pass. Native co-op shop test
passes all ten checks including actual departure and resumed mission music;
`build/logs/coop-vendor-audio-departure-20261004.log`. The first departure fixture
returned to the mission start, still inside the 135-unit radius, so its track
correctly remained audible at low volume. The corrected test uses the safe pipe
lift landing outside the radius. No assertions or script errors in the retest.

The user also reported no obvious exit after the drill explosion and Predator
fight. The preserved live log records `xewr_res401`'s `PLAT FALL` and later combat
triggers, but no `levelend` entry and no script error indicating a broken exit.
The authored objective is to get up and out. `xewr4winlvl` wins on entry into
`levelend`, with no additional boss/kill requirement in that script.

World positions: `pipe_lift` lower landing (-46.557, -213.227, -298.695), moves
48.3 units upward; `levelend` (-44.156, -101.492, -439.523). A fresh native co-op
test successfully rides the pipe lift to player height -164.729, log
`build/logs/drill-exit-pipe-live-20261004.log`, snapshot
`build/shots/drill-exit-pipe-live-20261004/shot_007.bmp`. This verifies that lift
in a fresh level; it does not reproduce the user's post-explosion checkpoint or
prove the complete route clear in their previous run. No progression bypass
was added based on this evidence. The quit log contains resource cleanup warnings
and earlier renderer limits, but those have no demonstrated connection to exit
triggering. The current isolated tests use native commands/snapshots and separate
saves, as requested; user profiles are untouched.

Installed after two closed-process checks; EXE/PDB/Bink hashes match staging.
Backup `build/backups/pre-vendor-audio-20261004-042256`. Output overrides
cleared. EXE SHA256:
`EB08BFB0DF113B4AC66581DFEAA9D3A3A4B7D27427D9108518FA377BE007BF88`.
