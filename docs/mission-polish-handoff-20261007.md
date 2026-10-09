# Mission polish handoff — 2026-10-07

Stopped at the user's request to finish this pass promptly. No subagents or test games from this pass remain running. Existing work from the other chat was preserved.

## Installed

Release build succeeded. EXE/PDB installed and hash-verified in `D:/Desktop/MetalArmsPC`, `build/pending-update/Release`, and `build/pending-input/Release`. `build/Release/ma_port.exe` was running in a separate session (PID 2256), so that process and its files were preserved. Its replacement is staged, not active. Manifest, immutable pair and backups: `build/logs/mission-polish-install-20261007.json`.

Installed EXE SHA256: `1953BA90DDAECF23FB1E941DCD8D4C587C5DFE1BCCF2E26C8B44CF3A111E86FB`.

## Changes and evidence

- **32, Unwelcome Home:** `xedi_town.sma` requires both total deaths and player kill credit, but its original handler accepts only the story player. Partner kills now count for that script in this mission's local co-op only. Individual statistics and other event listeners retain the real killer. Native two-player test proved the original handler rejects P2, the fixed real `CreditKill` event increments its quota, and the first casualties transition advances. Log: `build/logs/invasion32-20261006/quota-2.log`. Later artificial wave-clear checks in that diagnostic were invalid because fresh enemies had spawned; those unsupported checks were removed. This does not retroactively recover kills omitted from an old checkpoint.
- **36, Coliseum boss:** surviving players can become Corrosive's target after P1 dies; whole-team defeat is required for the defeat reaction. Any living Glitch can complete the wrench escape, and the winning actor is used in the ending. Hut occupancy tracks players separately. Added a readiness guard after the native test exposed an uninitialized combat object. The retest passed P1 death, P2 retargeting, and rejection of an assembled player's false hut win: `build/logs/mission-polish-20261006/corrosive36-2-e770525d/native.log`. The next test step tried firing the wrench before its deployment finished; the diagnostic now waits for deployment. The complete wrench/ending sequence has **not** been verified after this final adjustment. Retail boss invulnerability is intentional; the wrench escape is the mechanic.
- **41 collision assertion:** Corrosive's hand collision callback spawned debris and applied damage while traversing world trackers. It now records bounded copied impacts and applies effects/damage after traversal. This fixes a concrete unsafe callback pattern without suppressing collision assertions. The user's exact level-41 occurrence was not reproduced, so its precise cause remains unconfirmed.
- **Earlier fixes:** the combined source retains the other chat's Coliseum shield escort and friendly-list corruption fixes, plus the previous fabricator release fix. Existing offline scene/restoration checks (434) and player/elevator checks (39) passed. Earlier in this pass the DET HUD, dialogue completion and native cutout rendering checks also passed. These checks are not a complete campaign or PvP playthrough.

## Unchanged / still open

- **32 disappearing friendly droids:** native diagnostic reproduced their existing lethal damage/removal path. Miner part-manager creation is disabled and their authored dialogue bank lacks a death animation. No new friendly-fire policy was added.
- **38, One Small Step:** the button moves the suspended magnet's slider, not the doors directly. The script opens the doors when the magnet reaches a field-post trigger and destroys its post. Native test confirmed switch/slider movement but did not complete that contact. Cable slicing was briefly suggested during investigation, then disproved: this asset disables anchor slicing. No production patch or verified player walkthrough for this puzzle was produced. The opt-in `hangar-magnet` diagnostic's contact expectation is unresolved; do not treat it as a passing regression test.
- **40, Bring It Down:** user confirmed orange/green supports return to normal after blasts/shots stop. No rendering change was applied. An artificial light cycle ran without assertions, but did not establish a defect in the exact pictured supports.

## Next work, only if requested

Finish the bounded native Corrosive wrench/ending test, investigate the real level-38 magnet interaction, and reproduce the specific level-41 assertion with a matching EXE/PDB stack. Do not restart broad campaign searches or repeat the established level-32 quota diagnosis.
