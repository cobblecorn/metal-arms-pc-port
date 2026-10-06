# They Live cage allies, 2026-10-03

The two authored bots `pipezom1` and `pipezom2` in `WEWJjourn01` now join the normal
ally-follow system after release in solo and campaign co-op.

Retail entity data gives both bots a `wait` job. The `xewj_wastes` release path removes
their respective physical cage and calls `Bot_Recruit` for each. However, `CBotZomBuilder`
does not set the recruitable-class flag, and these entities have no open data port.
`CBot::Recruit_CanBeRecruited` therefore rejects the script's recruitment. The earlier
neutral-friendship correction did not change recruitment or assign a follow target.

The exception applies only to campaign, these two names in this exact world, and a call
without a grenade epicenter from a living, controlled player bot. Normal recruiter grenades,
other zombies, PvP, reserved/shocked/possessed/dead bots, player-owned bodies and the explicit
instance prohibition retain their rules. Successful recruitment still uses the existing
recruiter system, team/race changes, shock, icons and checkpoint state.

After recruitment shock ends, bot work assigns an actual follow thought instead of waiting
for AI visibility to discover a friendly player. P1 is preferred; a surviving partner leads
while P1 is dead. Allies follow the currently controlled body during possession. Healthy
formations are left running; absent follow state after restore/failure is reacquired. The
normal ally movement, formation, navigation and combat systems do the work. Still-caged bots
are not recruited or moved by the follow helper.

Validation: `tools/test_caged_zombie_follow.py` passes 132 production-helper checks for
solo and 2–4 players, release/shock gating, repeated frames, checkpoint reconstruction,
P1 death/revival, possession, allocation retry and exclusions. Existing campaign playthrough
fixture passes 328 checks. No game was launched or save/profile data changed. Actual route
navigation remains for gameplay verification.

The user also confirmed the earlier fence transparency correction in gameplay.

Debug and Release builds pass. With no game process running, installed the combined update
including the previously staged checkpoint/RAT runtime corrections in `build/Release`.
All three installed files match staging; CMake output overrides are reset. Backup:
`build/backups/pre-cage-follow-rat-runtime-20261003-224426`.

- EXE SHA256: `56BB2FFB49A8F34624BAE8FA2A0604DB8EAC70ADE09D9A3723D3FCF552D6E329`
- PDB SHA256: `343CD8FA9A0451A1592E2035BAC68EDA30015C11B4D9232CAC4B4E950783B239`
- Bink SHA256: `8E4B8E032A52CD42796A35E062511E26641AE2247548AE69AFD49E38DB3856C5`
