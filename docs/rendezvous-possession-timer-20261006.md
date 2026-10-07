# Secret Rendezvous: possession, countdown and ending

Level 29 (`WEMCcity_02`) reports: console interaction repeated vendor voices;
chamber Titan occasionally stayed inside; DET-pack countdown persisted after
detonation; Glitch's ending line was clipped; ending could use a possessed Titan;
an unidentified assertion appeared after completion.

## Changes

- PC DET packs clear their own countdown pointer from every player's HUD when
  exploding, being removed, or restoring a checkpoint. Cleanup checks ownership
  of that pointer, preserving a different pack or race timer. The planter's
  current possession index is no longer used to decide which HUD to clear.
- The co-op `xemc_shhh.sma` ending returns the active story player from the borrowed
  bot to Glitch, releases the console through its normal cleanup, then places
  Glitch at the scene position. The live cached AMX actor is replaced before the
  same callback's `Bot_GotoE`. This repair applies only to this mission's ending
  in local co-op; other mission possession and solo/PvP actor selection are unchanged.
- Natural cinematic completion now protects all undamaged finite 2D dialogue,
  including ordinary player dialogue. `GL_22m_020` has an authored duration of
  3.815 seconds but resolves to a 4.556-second sample (Level_22D, SFX 1087,
  sample 457). The previous protection required FORCE_2D_AUDIO, which this player
  line did not have. Explicit skip/termination, damaged voices, 3D ambient voices,
  infinite emitters and gameplay dialogue retain their existing behavior.
- Opt-in audio diagnostics log countdown registration/cleanup and natural voice
  completion. Test fixtures run only with `MA_PORT_TEST_COOP_POLISH=rendezvous-*`.

The countdown and cinematic audio corrections apply globally to the PC port;
the ending actor correction is restricted to campaign co-op.

## Validation

- Release build passed; source whitespace check passed.
- `python tools/test_detpack_hud_lifetime.py`: 70 production-method checks across
  one through four players, replaced countdowns, repeated removal, child packs
  and race timers.
- `python tools/test_dialogue_completion.py`: 462 production completion checks.
- `python tools/test_coop_cached_players.py`: 39 retail AMX elevator/player-binding
  checks. Its standalone factory-active stub was updated for an earlier dependency.
- Native two-player `rendezvous-ending` uses the actual chip insertion and console
  possession, then the authored ending tripwires. Glitch is restored, the 3.815-
  second line completes at 4.574 seconds, and the mission-complete screen loads
  without an assertion.
- Native `rendezvous-timer` uses the actual console, possessed Titan and authored
  DET pack, unplugs while counting down and verifies registration/cleanup of the
  same HUD pointer as well as removal of the pack. This pointer check is required:
  the draw flag alone did not reliably indicate a registered countdown. The
  fixture cancels its own NoGravity console back-jump to avoid flying out of bounds.
  See `build/logs/rendezvous-timer-1-after.log` and `rendezvous-timer-2-after.log`.
- Three/four-player native runs have not been validated for this repair.

## Reports still unconfirmed

Console chip insertion/possession did not reproduce vendor speech. The SPC effects
resolve to their console samples, with no vendor sample overlap in the audio
mapping audit. Authored console possession opens the Titan chamber in the test;
the intermittent trapped-actor report has not been reproduced. No speculative
vendor/AI changes were made.

The running user's root log starts in later missions and contains `fworld_coll`
callback assertions, not an attributable level-29 end assertion. The user does
not remember its expression. Passing this ending test does not identify or prove
a repair of that separate warning.

## Deployment

The candidate includes earlier co-op encounter and Goff-part completion fixes.
Install state and paired executable/PDB hashes are recorded in
`build/logs/rendezvous-install-20261006.json`. The root, Desktop and staging copies are
updated and verified against the immutable executable/PDB pair. The user's old
root process exited during verification, allowing installation immediately;
the earlier Goff installer completed before this installation.
