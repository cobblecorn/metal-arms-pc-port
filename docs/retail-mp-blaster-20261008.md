# Retail multiplayer maps and Scatter Blaster — 2026-10-08

The source Xbox MP world order disagreed with the retail GameCube menu IDs. For
example, Last Minute Effort (ID 45) selected WE04multi04, the Coliseum, instead of
WE05multi05. The FANG_WINGC level list now follows the 14 retail DOL records,
including each world, script, material table and the Coliseum callbacks. The map
count and chip thresholds use the same 14-map order; the fake missing-map
threshold was removed. Other platform lists remain unchanged.

The user requested vanilla shotgun behavior. The old Scatter Blaster submitted
a whole damage form for every pellet. Retail keeps one representative hit per
entity per burst, choosing the pellet most aligned with the shot direction.
The FANG_WINGC path now uses that grouping, a 16-entity limit, centrality
clamp(200 * (alignment - 0.995), 0, 1), and cubic range falloff
max(1 - distance / maxRange, 0)^3. Existing damage/armor profiles, barrel timing,
ammo and per-pellet visual effects are retained.

Evidence from the user's retail main.dol:
- MP level array: file offset 0x3c6b04, 14 records of 44 bytes, level IDs 42-55.
- Scatter hit collector: 0x801edce4; deduplicates entities and keeps maximum dot.
- Damage calculation: 0x801ef954-0x801efa8c; centrality and cubic range falloff.
- Constants at 0x804bf5b0 = 200 and 0x804bf5b4 = 0.995 (float).

Verification:
- tools/test_retail_mp_selection.py: 114 production map/unlock checks. Expected
  records come from main.dol and menu/chip values from the retail CSV assets.
- tools/test_retail_blaster_damage.py: 208 production impact/damage checks,
  including repeat pellets, all player slots, multiple targets, range, glancing
  hits, misses and burst limits.
- tools/test_blaster_ammo.py: 8,347 existing ammo/fire checks.
- Full Release build succeeded. build/Release/ma_port.exe is the updated EXE
  used by tools/package_release.py. The previous EXE/PDB were backed up.
- Live gameplay has not been re-tested. No game or player-save operations.

The workspace moved from D:/metal arms source port back to its usual
D:/Documents/metal arms source port during this turn. The final Release build was
made at that usual location, using the existing build directory. Other working
changes were retained; no source reset or old binary was applied.
