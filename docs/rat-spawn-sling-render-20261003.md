# RAT aim, scripted boss entry, slingshot and cutout rendering — 2026-10-03

Installed in `build/Release` after confirming the game process was closed. No game
launch, live-memory changes or profile/save changes. EXE/PDB/Bink match the staged
copies. The previously staged weapon-visibility correction is included too.

## Changes and evidence

- **RAT turret:** reverse the human gunner's combined vertical stick/mouse adjustment
  only for `BOTSUBCLASS_SITEWEAPON_RATGUN` on PC. Horizontal controls, other turret
  classes and NPC aiming retain their existing behavior.
- **ZombieBot King entry:** retail `xezz_jnk01.sma` moves the story Glitch from `start01`
  to `glitchgoto` during the intro, adds the gates, then plays the falling animation
  and saves a checkpoint. The load-time partner placement preceded that relocation,
  leaving P2 outside. The specific `WEWZzombi01`/`glitchgoto` story-bot snap now places
  partners beside the relocated lead. Scripted placement allows a longer floor probe
  for the falling intro, retains body/wall/lethal-floor checks, and checks occupancy
  after resolving the floor. Normal checkpoint placement keeps its strict height limits.
  Fixtures cover 2–4 players, every lead index, nonoverlap and unsafe-floor rejection.
- **Slingshot:** PC no longer requires a release velocity of at least 10 units/sec.
  That retail analog threshold silently discarded gradual releases and shots during
  frame stalls. Releases retain the measured draw strength and normal launch path.
  Temporary firing-stance gates retain a physically held primary slingshot trigger
  instead of synthesizing a release. Loading/stowed/abort modes remain intact.
- **Fence holes:** actual arena material uses `tewj_fnce01`, cutout lighting and
  `cBASE`. Its alpha is already intact (2,325 zero-alpha pixels in the 64×64 base mip).
  A hidden-window D3D9 test proved the production ColorMask shader rejects holes.
  The compatibility layer deliberately maps multipass EQUAL depth to biased LESSEQUAL
  to tolerate different shader compilation. The subsequent surface pass disabled alpha
  testing and consequently painted black texture pixels into those holes. PC cutout
  surface variants now repeat a >127 alpha test. This preserves the existing depth
  tolerance for opaque rendering. The two-pass GPU fixture reproduces black holes with
  the old behavior and preserves the background with the production alpha helper.
- **Black surfaces:** vertex-buffer changes now preserve the combined environment/
  lighting shader, including reflection coordinates. Fullbright materials use the
  baked instance color stream where present. Also corrected the simple diagnostic
  shader helper's enum/handle confusion; that helper is not the gameplay fast path.
  These are concrete renderer defects; coverage of every reported RAT black surface
  remains to be verified in a real run.
- **Texture decoding:** GC CMPR endpoint order now selects its hardware three-/four-
  color palette even for an alpha plane; RGB5A3 expands its three-bit alpha to the full
  0–255 range. These are independent correctness fixes. Comparing actual fence textures
  ruled CMPR decoding out as the cause of the arena fence report: `tewj_fnce01` and
  `tecc_fence` had unchanged alpha; `tewz_fence` changed some intermediate alpha values
  without changing the pixels rejected at the cutout threshold.

## Validation

- `tools/test_sling_texture.py`: 9,144 production-method input/texture/shader/aim checks.
- `tools/test_cutout_surface.py`: real compiled pixel shaders, legacy failure reproduced,
  fixed two-pass GPU output verified, all surface/pass alpha selections checked. Creates
  a hidden test window; does not start the game or access saves.
- `tools/test_coop_checkpoint_rat.py`: 293 checks including scripted partner placement.
- Existing selector/menu 6,147, waterfalls 6,901, scene/lift/liquid 434, cinematic/UI 851,
  weapon visibility 611 and playthrough 328 checks all pass.
- Final Debug and staged Release builds pass; whitespace check passes. Release output
  overrides were reset before installation.

Gameplay confirmation remains pending for the new spawn/aim/slingshot behavior and
full rendering coverage. The separate valve position correction remains unresolved.

## Installed binaries

Backup: `build/backups/pre-rat-spawn-sling-render-20261003-202116`.

- EXE SHA-256: `E24BE14A8FADD3E8B419B0A03F7A4EFEA9B1EB62A32A2A53659270741534F6A6`
- PDB SHA-256: `40B30BECE10D70A605C352EABFFC2BB9B112D1B3AC33B9ABDF7571571F50BE0F`
- Bink SHA-256: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`
