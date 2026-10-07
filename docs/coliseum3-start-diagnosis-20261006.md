# Coliseum 3 battle-start diagnosis (2026-10-06)

The first fight hangs because the authored `outerdoor6` has no `behavetype=other` property. `outerdoor3` has the same omission. Every other outer gate in this map, and all six corresponding gates in Coliseum 1, 2 and 4, explicitly use `other`.

`CEDoorBuilder::SetDefaults` defaults an unspecified behavior to `DOOR_BEHAVIOR_DOOR` (automatic proximity opening/closing). `DoBehaviorDoor` closes a fully open gate immediately when `_ValidBotNearby` is false. It does not use the auto-close timer. In contrast, `DOOR_BEHAVIOR_OTHER` with zero control flags stays at the position requested by the script.

The runtime trace in the fixes checkout (`build/logs/colis3_gap.log`) shows:

- t=30.19: `xebcbttle03` asks outerdoor6 to open and delays work for 4 seconds.
- t=31.19: gate reaches unit position 1.
- t=31.20: gate begins closing.
- t=33.20: gate is shut.
- t=34.20 onward: battle State 1 polls `Door_GetUnitPos == 1`, receiving 0 forever.

The first Grunts' Goto/Attack commands are inside that successful-open condition (script source lines 505-512), so the combat never starts. The guard cannot hold the gate open in advance: those commands have not been issued yet.

Read-only PDB inspection of fresh solo and two-player runs of `D:/Documents/metal-arms-fixes/build/Release/ma_port.exe` confirmed identical values: outerdoor6 and outerdoor3 behavior=0 (DOOR), flags=0; the other four gates behavior=3 (OTHER), flags=0. There is no enabled AUTOCLOSE flag. This is an asset/default-behavior mismatch present in both modes, not an all-partners trigger requirement. A full solo fight was not played.

The earlier handoff's explanation that AutoClose accumulates time while shut is incorrect: its only caller is the open-state branch of DoBehaviorOther, and these two gates are using DoBehaviorDoor instead. Adjusting the timer would not repair this stall.

Repair applied in `CDoorEntity::ClassHierarchyBuild`: restore OTHER behavior only for `outerdoor6` and `outerdoor3` in WEBCcolis03, equivalent to the missing authored properties. The Windows port guard and exact level/name checks leave global proximity doors and elevator behavior untouched. The repair applies to solo as well as co-op. The same source correction was applied to the fixes checkout while preserving its existing changes and temporary diagnostics.

Generated evidence in this workspace: `build/logs/colis3-shapes.json`, `build/logs/colis3-battle-disassembly.txt`, and `build/probe_colis3_properties.py`. Each probe launched and terminated only its own diagnostic process. Lava was outside the scope of this investigation.

## Native verification

`tools/test_coliseum_gates_native.py --players 1,2` passed against the Release candidate. Its opt-in fixture loads the authored world and battle scripts with isolated saves, resets the tested gates to shut, moves players away from both gates, and dispatches the real `trig_doorclose` callback. Ordinary play never runs the fixture.

The retail `xebcbttle03` script reached line 504, `Door Open`, after its four-second delay, passing the previously stalled condition before the first Grunts' Goto/Attack orders. Both gates were still fully open at 6.5 seconds; explicit scripted close commands shut both again. No FANG assertion occurred in either successful run.

Logs: `build/logs/coliseum-gates-native/1-710a054f.log` and `build/logs/coliseum-gates-native/2-dd626ea8.log`. This is a focused battle-start and gate-lifetime test, not a full mission playthrough. The later gate3 wave was not played; gate3's same stay-open and close-on-command behavior was exercised directly. A four-player launch never reached fixture readiness and supplies no verification evidence (`4-0700efa8.log`).

## Deployment

The tested EXE/PDB pair is staged in `build/coliseum3-install-20261006` and hash-verified in `D:/Desktop/MetalArmsPC`, `build/pending-update/Release`, and `build/pending-input/Release`. The running Unwelcome Home diagnostic keeps `build/Release/ma_port.exe` locked; `build/install_coliseum3_when_idle.ps1` will install there on game exit and will not overwrite a newer concurrent deployment. Current completion and backups are recorded in `build/logs/coliseum3-install-20261006.json`.

EXE SHA256: `E79C6FC0C0043AA4223A6E099122CB52115F5EF9397282042EEC2FC8E5B6A0DC`. PDB SHA256: `BC2973734933CDA116D22B597E0A472C8AF1B9D47D604656E4B5569A464FD9A3`.
