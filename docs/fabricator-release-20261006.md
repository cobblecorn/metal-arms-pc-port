# Night Sneak fabricator release

The upper authored fabricator reproduced the reported trap in campaign play.
With a known nearby intruder, the newly constructed Jumper switched to ATTACK
inside its shielded chamber and remained at `(-183.38, 0.86, 676.31)`. The
unpatched native run failed its exit check; the bot remained in ATTACK at that
position even after the players retreated. This is evidence of combat interrupting the release
sequence, rather than proof that a temporary noclip interval expired.

The production repair is restricted to campaign fabricator release. Local
co-op uses campaign rules, so the same release correction applies to solo.
PvP fabricator behavior bypasses the campaign repair.

`tools/test_fabricator_native.py` runs the actual lower or upper machine in
`WECDsneak02`, with isolated saves and a fresh log. It requires observed
construction, a completed powered bot, movement more than 12 feet from the
construction position before the players retreat, the retail exit PASS, and
resumed ATTACK after departure. A missing final check, fixture failure, or FANG
assertion fails the run. Each run has a 50-second observation limit and closes
only its own process.

```powershell
python tools/test_fabricator_native.py --machine lower --players 2
python tools/test_fabricator_native.py --machine upper --players 1,2
```

The native fixture temporarily relocates and protects test players. Those
actions are opt-in diagnostics and do not run in ordinary play. The runner
provides focused release and combat checks; it does not certify an entire
mission playthrough or the separate report of a rocket enemy inside a wall.

Run native games sequentially: concurrent launches share the candidate's Fang
resource log and can interfere with fixture readiness. Four-player launches
also require a ready controller/join setup; a run that never observes bot
construction provides no evidence about the release behavior.

Verified candidate results: upper fabricator in solo and two-player co-op,
and lower fabricator in two-player co-op, completed all release and
resumed-combat checks without a FANG assertion. Logs:
`build/logs/fabricator-native/upper-1-5b8a3726.log`,
`build/logs/fabricator-native/upper-2-d12268d9.log`, and
`build/logs/fabricator-native/lower-2-24f69abc.log`.

Installed verified EXE/PDB pairs in `build/Release`, `D:/Desktop/MetalArmsPC`,
`build/pending-update/Release`, and `build/pending-input/Release`. The user game
was preserved until it exited. Installation hashes, prior-pair backup, and target
completion are recorded in `build/logs/fabricator-install-20261006.json`.
