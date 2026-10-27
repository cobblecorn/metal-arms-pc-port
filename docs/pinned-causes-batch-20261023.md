# Pinned-cause batch — 23 October 2026

Six reports whose causes another chat traced in source. Built on 1.0.9 (`1d8ca67`). Release
`ma_port` builds; resource version still 1.0.9.0; no patcher zip built.

Installed for playtest (EXE + PDB only) in `D:\Documents\Metal Arms PC Port`: EXE SHA256
`FBDA02CB8080BE208E4CA2D0861351E06C64BB5201FE27A377F07513274CBEFB` (second install, with the shop
follow-up below). `build/backups/pinned-causes-20261023/` holds the pre-batch pair (`original/`,
`FD318C…`), the first install (`first-install/`, `DA9654…`), the source diff and `installed.json`.

## Changes

| Report | Cause | Fix |
| --- | --- | --- |
| Tank machine gun far too strong; enemy Loader gun very lethal | Retail vehicle tables carry four weapon profiles (primary player/NPC, secondary player/NPC: `b_vsentinel` SentinelCannonPC/NPC, SentinelMGunPC/NPC; `b_vloader` LoaderGun_PC/NPC, LoaderPincers x2). The source read two, so the tank MG used `SentinelCannonNPC`, NPC Loader drivers used `LoaderGun_PC` (6 base vs 2) and the pincers used `LoaderGun_NPC`. | `vehicle.h/.cpp`: four-field vocabulary; `CVehicle::GetWeaponDamageProfile( bSecondary, shooter )` picks by player index (NPC = -1). Loader gun/crush and tank MG use it. The tank shell's blast now uses it too: its own lookup chose `mp_` names in versus, but `CDamage::SwapMultiplayerData` already swaps the mp_ data in under the plain names, so that picked campaign values in PvP. |
| Co-op batteries vanish after the first player | Personal (per-player) pickups covered only weapons/upgrades; numbered batteries were turned into Mega Energy from P1's inventory alone. | `collectable.cpp`: `_IsPersonalPickup()` (renamed from `_IsPersonalWeapon`) includes campaign batteries; per-player claims, view hiding and checkpoint state reuse the existing mask. A numbered battery becomes Mega Energy at level start only if every player owns it; otherwise an owner gets the energy refill (Mega Energy HUD/sound), others the battery. Owners or full sets at full health don't block a partner. Purchases/gifts (`m_nRecipientPlayer`) unchanged. |
| Walking while choosing a weapon | W/S (and the left stick) both scroll the full selector and move; only fire was suppressed. Solo hides it by pausing the world; co-op/versus can't. | `player.cpp`: while `IsWSActive()`, the selecting player's forward/strafe are zeroed in every mode (user's call: global). The HUD still reads the raw axis; partners move; quick taps never open the selector. |
| Flamethrower reticle wrong | `tfh_cross02` is one corner; retail draws it mirrored four ways. | Retail dispatch `801cc478` sends TYPE_FLAMER to the mirror renderer `801cfa70` with 0.05/0.05 offsets (Quad Laser 0/0). `_Draw_MirrorXY` takes the offsets; each quad's inner corner sits at origin ± offset, as retail's arithmetic does. |
| Lingering loop ("zipline") after cabinet exit | `CFAudioEmitter::Destroy()` stopped/released the DirectSound buffer only for logical PLAYING. Stop()/Pause() change the state at once but the buffer on the next update, and paused emitters keep theirs, so a destroyed emitter could keep playing on a recycled real emitter with its voice still counted. | `fdx8audio.cpp`: release whenever a real emitter is owned (it always counts in `uPlaying`); end-of-play callback still only for PLAYING. **The exact cabinet sound (Zipping / ZipBrake / SPC_Attract) is not reproduced or identified.** |
| Rivet ammo purchase not delivered | `CBarterItem::Purchase` dropped a pickup and charged regardless of the result; stock was consumed too. | `BarterTypes.cpp`: ammo for an owned weapon goes straight into that weapon (reserve/clip, upgrade level untouched, `CCollectable::GiveOwnedWeaponAmmo`) for the shopper; owned-but-full refuses. Every other item is placed for the shopper and collected for them on the spot by the normal pickup code (`CCollectable::GiveToPlayerAndCollect`: same HUD, sounds, EUK/battery/servo rules); one they can't take (full health, nothing to upgrade, already owned and full) is removed instead of left lying there. No delivery → no charge, no stock change, vendors' failed-purchase response. Logged as `Port: shop purchase ...`. |

## Verification

Offline, all pass: `test_shop_purchase` 66 (new: the real purchase/grant/collect methods; wallet,
reserve, tier, stock, P2 shopper, refusals, nothing left in the world; a "charge anyway" mutant fails it),
`test_slosh_pickups` (PC 127, including 9 new production co-op battery checks),
`test_checkpoint_euk` 2,043, `test_collectable_skiplist` 180,016, `test_reddit_gameplay` 16,048,
`test_reported_systems` 963, `test_weapon_select_menu` 6,147, `test_vehicle_math` 118,
`test_vendor_audio` 19, `test_retail_blaster_damage` 208. Logs: `build/logs/offline-*-20261023.log`.
Line endings preserved (CRLF sources); `git -c core.whitespace=cr-at-eol diff --check` clean.

Not verified natively: tank/Loader damage feel (player and NPC), co-op battery runs (different
starting counts, P2 first, 4P, checkpoint rollback), selector lock on keyboard and pad, the
reticle's appearance and split-screen, the cabinet sound, shop sequences (Rivet L1–3, repeats,
near-full reserve, each co-op shopper).
