# Krunk asylum exit: ten kills, fourteen bots (2026-10-04)

User explicitly requests lowering the exit requirement in F&!?ing Krunked
(`WEWRresrch3`) from fourteen kills to ten, while keeping all fourteen spawns.
Their two players were controlling bots and the retail exit script remained at
thirteen recorded deaths. This is an intentional gameplay adjustment in solo
and co-op, not a claim that vanilla has no way to dispose of a controlled bot.
Log snapshot: `build/logs/krunked-progression-20261004.log`.

The authored `xewrasycam1.sma` registers fourteen named bots and waits for a
death count of fourteen before starting the asylum-exit speech/camera sequence.
That sequence opens `exit_asylum`, ends the cutscene, and saves a checkpoint.

## Change

`port/pc_script_goals.h` applies a narrow PC runtime override after `amx_Init`
expands and relocates the program, before any OnInit execution. It matches the
script name and validates the exact retail comparison instruction window:
`LOAD_PRI 560; EQ_C_PRI 14; JZER ...`, at comparison offset `0x4b0`.
It changes only the comparison operand to ten, idempotently. Other script names,
short headers/code, or a different instruction layout are left untouched.

The fourteen-bot roster, spawn scripts/assets, actual death counter, and normal
exit work sequence are unchanged. Further deaths after ten do not start a second
exit sequence. Retail developer text `CAMERA GO 14 DEAD BOTS` is retained and now
refers to the original marker, not the requirement; the added port diagnostic
reports the actual ten-kill goal.

This applies on fresh mission initialization in both solo and campaign co-op.
An already-running script is not patched in memory. **Restart the mission from
its beginning with the updated EXE**, rather than restoring a checkpoint with
more than ten deaths: the authored comparison remains equality and its death
counter is checkpointed.

## Validation and installation

- `tools/test_asylum_kill_goal.py`: 68 checks using the production SmallAMX VM
  and actual retail compact script. The production helper changes exactly one
  byte in the expanded program. All fourteen bot references still register;
  kills one through nine leave the exit locked; kill ten runs the authored
  camera/door/checkpoint sequence; kills eleven through fourteen do not repeat it.
  Native game calls are spies, so physics/cameras and real gameplay are untested.
- Debug and Release builds pass, scoped whitespace check passes. CMake staging
  overrides reset. No game launched or live process manipulated.
- Game had closed by build completion. Two process checks bracketed the backup;
  normal Release EXE/PDB/Bink installed and all hashes verified against staging.
- Backup: `build/backups/pre-asylum-goal-20261004-025122`.
- Installed EXE SHA256:
  `B747A5ADCBF889A3DBBE3B3A76F404BBAD0162366CD90A96A527CC5098E83FE6`.

The user subsequently confirmed: "the 10 bot cap fixed it nicely."
The ten-kill exit is confirmed in their co-op gameplay; all fourteen original
spawns remain retained by the runtime override.
