# Rivet Gun mouse charge follow-up

The existing L3 Rivet Gun charges its embedded explosive rivet while primary fire remains held, then detonates it on release. A native mouse run confirmed this before the change. There was no right-click charge mapping; the prior review had tested the existing primary-hold behavior without changing Rivet gameplay code.

The new mouse mapping uses the same default 0.75-second tap/hold boundary as the Mining Laser. Hold right-click to fire the L3 rivet and keep its existing charge active after impact; release to detonate. A short right-click still operates the selected secondary item. Normal left-click firing and its existing hold/release behavior remain available. L1 and L2 retain their existing firing mechanics; this does not invent a charge mode for the timed explosive L2 rivet.

## Scope and implementation

- `weapon_rivet.cpp/.h`: per-weapon gesture state, enabled only for a player-controlled Glitch with the remote-explosive Rivet type and mouse weapon input. It drives the existing primary trigger and projectile logic. Damage, steam-charge rate, impact and explosion code are unchanged.
- `botglitch.cpp`: passes physical secondary history to the charged primary, forwards only a short tap to the grenade/scope, and cancels pending input on a blocked firing stance. The existing Mining Laser path is retained.
- State resets, weapon removal and upgrade changes cancel a pending mouse gesture so a held button cannot become a later grenade throw.
- Controllers continue through the existing primary-trigger path; NPC and possessed-MIL firing have no new mouse charge mapping. Physical controller testing is not claimed.
- `pc_input.cpp`: only the opt-in automated test key schedule capacity changes, from 32/512 bytes to 128/2048 bytes. A test with two long holds exceeded the previous schedule buffer and produced no injected input. Normal player bindings/routing are unchanged.

## Embedded projectile cleanup fix

An intermittent native failure was traced to `CEntity::_DetachAndRemoveAllChildrenAndRecurse`: an arrow attached to a temporary `CMeshEntity` inherited its removal. The rivet was detached and taken out of the world while still marked active/launched in its pool, so its steam charge and life timer stopped. This also explained why some earlier primary-hold probes varied by what they hit.

Native arrows now use the existing `SetRemovedOnParentsDetach(FALSE)` contract: they detach at their last position and keep ticking when the parent is removed. Bot panic notifications use the current parent pointer rather than the cached hit entity, avoiding a stale pointer after removal. This applies to embedded arrows in solo/co-op and preserves their damage, lifetime and explosion settings. Temporary trace assertions were removed from both entity.cpp and eproj.cpp after diagnosis.

## Verification

`tools/test_rivet_mouse_native.py` drives actual synthetic mouse input in a rendered, muted native client with isolated saves. It checks one grenade per short tap, full charge and release of a real embedded rivet, one round spent per charge, a working scope tap, and no scope toggle during a held charge. `tools/test_coop_batch_native.py --case rivet --players 1` checks the existing left-click behavior. The final build receipt records the completed solo/co-op and Mining Laser regression checks and the exact executable hash.

The original randomizer option tests were also completed on the preceding runtime; their separate receipt retains that binary's hash. Final smoke checks ensure the added mouse path does not prevent modded co-op or vanilla loading. Main and modded saves are retained.
