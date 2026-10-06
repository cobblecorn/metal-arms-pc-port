# Missing prison chip, 2026-10-03

The Mil Comm Centre prison chip was omitted during world parsing, before any combat or
co-op pickup handling. The saved gameplay log identifies `WEWCcomm_02`, entity `prison2`,
and reports `Error in 'goodie' table. Need 5 params.` Snapshot:
`build/logs/missing-prison-chip-20261003.log`.

The retail guard's properties contain `goodiebag 1 one` and `goodie1 chip 1 1 1 X safe`.
The nearby `cageopen` switch requires a chip. The source's four/five-field-only parser
rejected the six-field entry, so the guard never had a chip in its runtime loot bag.
This affects solo as well as co-op; the diagnosis does not depend on enemy kill credit.

PC parsing now accepts four through six fields. The known six-field retail form accepts
the `X` spin placeholder and validates the final `safe` string. Existing four/five-field
`none`/`random` forms retain their meaning. Unknown extra values/types/counts still fail
without installing partial loot. Type/resource notification, quantity/probability and the
normal bag cloning/drop path are retained. No free chip, unlocked switch, modified mission
asset or additional pickup physics policy is introduced. The suffix is accepted as retail
metadata; the existing collectable system supplies chip handling.

`tools/test_retail_goodie_chip.py` compiles the actual parser branch, goodie creation,
cloning, quantity and drop methods. Legacy/non-PC (19 checks) reproduces rejection/no
resource registration; PC (32 checks) loads the authored entry and releases one chip at
the guard, including multiple random rolls, legacy formats, malformed suffixes and full
bags. Existing campaign playthrough checks pass (328). Debug and Release builds pass.

The game was not launched and saves/profiles were untouched. Actual pickup and cage-panel
activation remain for gameplay verification. Reload the mission with the corrected build;
a world already loaded with the rejected loot entry cannot gain it from this parser fix.

Installed EXE/PDB/Bink in normal `build/Release` with no game process running. Hashes match
staging, and CMake Release output overrides are cleared. Previous files backed up to
`build/backups/pre-retail-prison-chip-20261003-225534`.

- EXE SHA256: `FCFD25540AE0EBF210167A9C1DF5D6B24D4F7C92127B829BFE59435E6B703E86`
- PDB SHA256: `E9900840A0FC86614BAF4885AFDD8E3CC69FA4EE80B01CEB5DF291B8C7092C02`
- Bink SHA256: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`
