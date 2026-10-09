# Story playtest fixes (2026-10-07)

Found while watching the user's single-player run (levels 1-36). Evidence: `build/logs/story-playtest-20261007.log`
(the later part of the run; the earlier part was overwritten by a restart).

| Problem | Cause | Fix |
| --- | --- | --- |
| Cutscene skip froze the screen (music playing, pause locked) | Skip runs the scene at 10x with drawing off until its script ends the letterbox. Port dialogue held each line until its voice clip finished (`BotTalkInst.cpp`), and voice plays in real time, so scenes waiting on `BTI_IsBTIPlaying` never finished. | Lines no longer wait for the clip while a skip is running (`game_IsSkippingCutscene`). Watchdog in `game.cpp`: if a skip has not finished after 6 s, drawing and normal speed come back (`MA_PORT_SKIP_TIMEOUT` overrides for tests). Skips log start/finish. |
| Coliseum 3 won itself mid-fight | The arena's enemy list is taken when it starts; grunt5-8 and pred1 are out of the world then, so only grunt1-4 counted. | Coliseum 3 also lists the out-of-world Mil/zombie bots. If all arena bots are dead and the rest never appear for 60 s, the fight ends anyway. The list is logged at start. |
| RAT gun antenna: assertion popups every frame (Wasteland Thunder) | A zero-length frame after a streaming wait gave an infinite 1/dt and a NaN spring that never recovered. | `fAntenna.cpp` skips such frames and resets a non-finite spring. |
| They Live ripper bridge: wires gone after a checkpoint restore (softlock) | The restore brings back each anchor but not its CFWire; wires are rebuilt only while moving or entering scope. | `CFVerlet::PortRefreshWires()` after the entity is back in the world. |
| Wasteland Journey 2: dying again right after every continue | A checkpoint saved during the collapse (and the user's cheats). | After two deaths within 8 s of a continue, restore the level-start checkpoint (`player.cpp`). |
| Tank cannon did no blast damage | The "Sentinel" explosion names profile `SentinelCannon`, which no table defines. | The shell uses `SentinelCannonPC/NPC` (mp_ variants in versus). |
| Pools ran out (missing flashes, effects, decals, dropped collision contacts) | Console-era budgets. | All engine pools set far beyond any level's use in `port/main_win.cpp` (heap 256 MB). Note `nWorld_MaxMeshesPerDraw` x4 sizes a u16 list, so it must stay under 16384. Muzzle flashes x8, sound emitters 2048. |
| Previous log lost on restart | Log opened with "w". | The old log is kept as `ma_port.prev.log`. |

Also from this run: Advanced Settings layout, outward selection arrows, quick restart for display changes, Hor+ FOV.

Not fixed / for later:
- Wasteland Journey valve: Glitch is shoved when walking close to it (no log evidence; needs a position/collision trace).
- `I, Predator`: after the Predator fight the script plays the generator-interior ambience (`seb_mgenint`) outdoors; retail behaviour not confirmed.
- Scripted music stops (`level_StopStream`) cut instantly, as in the retail source; a short fade would be polish.
- Lift restore log prints an uninitialised "pickup" value (-431602080).
- One console's attract hum fails to allocate at load (Research levels, Comm Center 3).
- Rivet Gun handout on a fresh save in Into the Trenches: the "euk rivet gun" lookup fails; confirm the weapon itself is given.
