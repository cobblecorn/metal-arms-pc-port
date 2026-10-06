#include "pc_cheats.h"
#include "player.h"
#include "bot.h"
#include "weapon.h"
#include "Item.h"
#include "ItemInst.h"
#include "collectable.h"
#include "MultiplayerMgr.h"

static u32 _nInfiniteAmmoPlayers;
static u32 _nInvulnerablePlayers;

static BOOL _ValidPlayer( s32 nPlayer ) {
	return nPlayer >= 0 && nPlayer < CPlayer::m_nPlayerCount && nPlayer < MAX_PLAYERS &&
		MultiplayerMgr.IsSinglePlayer();
}

void pccheats_Reset() {
	_nInfiniteAmmoPlayers = _nInvulnerablePlayers = 0;
}

BOOL pccheats_Enabled( s32 nPlayer, PcCheatAction_e eAction ) {
	if( !_ValidPlayer( nPlayer ) ) return FALSE;
	const u32 nBit = 1u << nPlayer;
	return (eAction == PC_CHEAT_INFINITE_AMMO && (_nInfiniteAmmoPlayers & nBit)) ||
		(eAction == PC_CHEAT_INVULNERABLE && (_nInvulnerablePlayers & nBit));
}

BOOL pccheats_InfiniteAmmo( const CWeapon *pWeapon ) {
	if( !_nInfiniteAmmoPlayers || !pWeapon || !MultiplayerMgr.IsSinglePlayer() ) return FALSE;
	const CItemInst *pItem = pWeapon->GetItemInst();
	const CBot *pOwner = pWeapon->GetOwner();
	for( s32 n=0; n < CPlayer::m_nPlayerCount && n < MAX_PLAYERS; ++n ) {
		if( !(_nInfiniteAmmoPlayers & (1u << n)) ) continue;
		const CPlayer &player = Player_aPlayer[n];
		if( (pItem && (pItem->m_pOwnerInventory == CPlayer::GetInventory(n) || pItem->m_pOwnerInventory == CPlayer::GetInventory(n,TRUE))) ||
			(pOwner && (pOwner == player.m_pEntityCurrent || pOwner == player.m_pEntityOrig)) ) return TRUE;
	}
	return FALSE;
}

BOOL pccheats_Invulnerable( const CEntity *pEntity ) {
	if( !_nInvulnerablePlayers || !pEntity || !MultiplayerMgr.IsSinglePlayer() ) return FALSE;
	for( s32 n=0; n < CPlayer::m_nPlayerCount && n < MAX_PLAYERS; ++n ) {
		if( (_nInvulnerablePlayers & (1u << n)) &&
			(pEntity == Player_aPlayer[n].m_pEntityCurrent || pEntity == Player_aPlayer[n].m_pEntityOrig) ) return TRUE;
	}
	return FALSE;
}

static void _RefillInventory( CInventory *pInventory ) {
	if( !pInventory ) return;
	for( u32 nSide=0; nSide < INV_INDEX_COUNT; ++nSide ) {
		for( s32 n=0; n < pInventory->m_auNumWeapons[nSide] && n < (s32)ItemInst_uMaxInventoryWeapons; ++n ) {
			CWeapon *pWeapon = pInventory->m_aoWeapons[nSide][n].m_pWeapon;
			if( !pWeapon || !pWeapon->IsCreated() ) continue;
			if( pWeapon->GetClipAmmo() != pWeapon->GetMaxClipAmmo() ) pWeapon->SetClipAmmo( pWeapon->GetMaxClipAmmo(), FALSE );
			if( pWeapon->GetReserveAmmo() != pWeapon->GetMaxReserveAmmo() ) pWeapon->SetReserveAmmo( pWeapon->GetMaxReserveAmmo(), FALSE );
		}
	}
}

static void _RefillPlayer( s32 nPlayer ) {
	CPlayer &player = Player_aPlayer[nPlayer];
	CInventory *pOriginalInventory = CPlayer::GetInventory( nPlayer );
	_RefillInventory( pOriginalInventory );
	CEntity *pEntity = player.m_pEntityCurrent;
	if( pEntity && (pEntity->TypeBits() & ENTITY_BIT_BOT) ) {
		CInventory *pInventory = ((CBot *)pEntity)->m_pInventory;
		if( pInventory != pOriginalInventory ) _RefillInventory( pInventory );
	}
}

void pccheats_Work() {
	if( !_nInfiniteAmmoPlayers || !MultiplayerMgr.IsSinglePlayer() ) return;
	for( s32 n=0; n < CPlayer::m_nPlayerCount && n < MAX_PLAYERS; ++n ) {
		if( _nInfiniteAmmoPlayers & (1u << n) ) _RefillPlayer( n );
	}
}

cwchar *pccheats_Apply( s32 nPlayer, PcCheatAction_e eAction ) {
	if( !_ValidPlayer( nPlayer ) ) return L"Cheats are available in campaign play.";
	CPlayer &player = Player_aPlayer[nPlayer];
	const u32 nBit = 1u << nPlayer;
	if( eAction == PC_CHEAT_INFINITE_AMMO ) {
		_nInfiniteAmmoPlayers ^= nBit;
		if( _nInfiniteAmmoPlayers & nBit ) _RefillPlayer( nPlayer );
		return (_nInfiniteAmmoPlayers & nBit) ? L"Infinite ammo enabled." : L"Infinite ammo disabled.";
	}
	if( eAction == PC_CHEAT_INVULNERABLE ) {
		_nInvulnerablePlayers ^= nBit;
		return (_nInvulnerablePlayers & nBit) ? L"Invulnerability enabled." : L"Invulnerability disabled.";
	}
	if( eAction == PC_CHEAT_REFILL_AMMO ) {
		_RefillPlayer( nPlayer );
		return L"Ammo refilled.";
	}
	if( eAction == PC_CHEAT_WASHERS ) {
		CInventory *pInventory = CPlayer::GetInventory( nPlayer );
		CItemInst *pWallet = pInventory ? pInventory->IsItemInInventory( ItemInst_apszItemNames[INVPOS_WASHER] ) : NULL;
		if( !pWallet ) return L"No washer wallet is available.";
		const s32 nBefore = FMATH_MAX( 0, (s32)pWallet->m_nClipAmmo );
		pWallet->m_nClipAmmo = (s16)FMATH_MIN( 32767, nBefore + 1000 );
		return pWallet->m_nClipAmmo == nBefore ? L"Washer wallet is full." : L"Washers added.";
	}
	if( eAction == PC_CHEAT_HEAL ) {
		CEntity *pEntity = player.m_pEntityCurrent;
		if( !pEntity || !pEntity->IsInWorld() ||
			((pEntity->TypeBits() & ENTITY_BIT_BOT) && ((CBot *)pEntity)->IsDeadOrDying()) ) return L"Player is down; wait for a checkpoint.";
		pEntity->SetMaxHealth();
		return L"Health restored.";
	}
	if( eAction != PC_CHEAT_WEAPONS && eAction != PC_CHEAT_UPGRADES ) return L"Unknown cheat.";
	CEntity *pEntity = player.m_pEntityOrig;
	if( !pEntity || pEntity != player.m_pEntityCurrent || !(pEntity->TypeBits() & ENTITY_BIT_BOTGLITCH) ||
		!pEntity->IsInWorld() || ((CBot *)pEntity)->IsDeadOrDying() ) return L"Use this while controlling Glitch.";
	CBot *pBot = (CBot *)pEntity;
	CInventory *pInventory = pBot->m_pInventory;
	if( !pInventory ) return L"No weapon inventory is available.";
	const s32 nPreviousPlayer = CPlayer::m_nCurrent;
	CPlayer::SetCurrent( nPlayer );
	BOOL bAllAvailable = TRUE;
	if( eAction == PC_CHEAT_WEAPONS ) {
		static cchar *apszWeapons[] = {
			"Blaster L1", "Rivet Gun L1", "Spew L1", "RLauncher L1", "Laser L1",
			"Ripper L1", "Tether L1", "Mortar L1", "Flamer L1", "Scope L1",
			"Coring Charge", "EMP Grenade", "Magma Bomb", "Recruiter Grenade", "Cleaner"
		};
		for( u32 n=0; n < sizeof(apszWeapons)/sizeof(apszWeapons[0]); ++n ) {
			// Existing weapons retain their upgrades; new grants use the
			// normal pickup pools rather than replacing the player's inventory.
			if( !pInventory->IsWeaponInInventory( apszWeapons[n] ) && !CCollectable::GiveWeaponToPlayer( pBot, apszWeapons[n] ) ) bAllAvailable = FALSE;
		}
	} else {
		for( u32 nSide=0; nSide < INV_INDEX_COUNT; ++nSide ) {
			for( s32 n=0; n < pInventory->m_auNumWeapons[nSide] && n < (s32)ItemInst_uMaxInventoryWeapons; ++n ) {
				CItemInst &item = pInventory->m_aoWeapons[nSide][n];
				CWeapon *pWeapon = item.m_pWeapon;
				if( !pWeapon || !pWeapon->IsCreated() || !item.m_pItemData || !item.m_pItemData->m_uCurLevel ) continue;
				pWeapon->SetUpgradeLevel( FMATH_MIN( pWeapon->GetMaxUpgradeLevel(), item.HighestUpgradeAvailable() ) );
			}
		}
	}
	_RefillPlayer( nPlayer );
	CPlayer::SetCurrent( nPreviousPlayer );
	if( eAction == PC_CHEAT_WEAPONS && !bAllAvailable ) return L"Available weapons granted; some aren't loaded in this level.";
	return eAction == PC_CHEAT_WEAPONS ? L"Weapons granted and ammo refilled." : L"Weapons upgraded and ammo refilled.";
}
