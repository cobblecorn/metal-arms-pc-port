#pragma once

#include "fang.h"

class CEntity;
class CWeapon;

enum PcCheatAction_e {
	PC_CHEAT_INFINITE_AMMO,
	PC_CHEAT_INVULNERABLE,
	PC_CHEAT_REFILL_AMMO,
	PC_CHEAT_WASHERS,
	PC_CHEAT_WEAPONS,
	PC_CHEAT_UPGRADES,
	PC_CHEAT_HEAL,
	PC_CHEAT_ACTION_COUNT
};

// Session toggles are per player and are never written to profiles/checkpoints.
void pccheats_Reset();
void pccheats_Work();
BOOL pccheats_Enabled( s32 nPlayer, PcCheatAction_e eAction );
BOOL pccheats_InfiniteAmmo( const CWeapon *pWeapon );
BOOL pccheats_Invulnerable( const CEntity *pEntity );
cwchar *pccheats_Apply( s32 nPlayer, PcCheatAction_e eAction );
