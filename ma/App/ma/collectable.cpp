//////////////////////////////////////////////////////////////////////////////////////
// collectable.cpp - 
//
// Author: Nathan Miller   
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2003
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 02/24/03 Miller      Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "fanim.h"
#include "fclib.h"
#include "fresload.h"
#include "fmeshpool.h"
#include "fworld_coll.h"
#include "fstringtable.h"

#include "collectable.h"
#include "entity.h"
#include "bot.h"
#include "Item.h"
#include "botfx.h"
#include "weapon.h"
#include "Player.h"
#include "botglitch.h"
#include "MeshTypes.h"
#include "ItemRepository.h"
#include "weapon.h"
#include "MultiplayerMgr.h"
#include "sas_user.h"

#define _COLLECTABLE_CSV					( "goodies.csv" )
#define _COLLECTABLE_SOUNDS					( "powerups" )
#define _COLLECTABLE_POOL_SIZE				( 100 )
#define _COLLECTABLE_GRAVITY				( -44.0f )
#define _COLLECTABLE_SPECIAL_PICKUP			( "collect_spc" )
#define _CORING_CHARGE_LDS					( "coringcharge" )
#define _CORING_CHARGE_COLLECT				( "coring charge" )
#define _PICKUP_SPECIAL_SOUND				( "SpecialSound" )
#define _COLLECTABLE_ARM_SERVO				( "Arm Servo" )
#define _REST_HEIGHT						( 2.5f )
#define _PUSH_VEL							( 1.0f )
#define _SPAWN_POOL_SIZE					( 10 )
#define _MAX_SPEED							( 30.0f )
#define _MAX_SPEED_SQ						( ( _MAX_SPEED * _MAX_SPEED ) )
#define _MESH_COL_SPHERE_SIZE				( 1.5f )
#define _HUD_MESSAGE_TIME					( 3.5f )
#define _PICKUP_RADIUS_MULTIPLY				( 0.75f ) // Was 2.0f, then was 1.5f
#define _MAX_BATTERIES						( 6 )
#define _EUK_MESH_ID						( 50 )

#define ITEM_UNKNOWN						( -1 )

typedef struct {
	cchar *pszName;
	CollectableType_e eType;
} NameType_t;

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// DO NOT MESS WITH THESE NAMES - IT WILL BREAK THE ENTIRE COLLECTABLE SYSTEM.
// THEY MUST MATCH THE NAMES IN itemlist.csv and goodies.csv
//
// ORDER NOW MATTERS IN THIS TABLE - IT MUST FOLLOW THE ORDER OF THE ENUM IN 
// COLLECTABLE.H
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
static const NameType_t _collectableTypes[] = {
	{ "Washer",				COLLECTABLE_WASHER },
	{ "Chip",				COLLECTABLE_CHIP },
	{ "Pup_Energy",			COLLECTABLE_PUP_ENERGY },
	{ "Pup_Hijump",			COLLECTABLE_PUP_HIJUMP },
	{ "Pup_Armor",			COLLECTABLE_PUP_ARMOR },
	{ "Pup_Speed",			COLLECTABLE_PUP_SPEED },
	{ "Pup_Weapon",			COLLECTABLE_PUP_WEAPON },

	{ "Det Pack",			COLLECTABLE_DET_PACK },
	{ "Battery",			COLLECTABLE_BATTERY },
	{ "Mega Health",		COLLECTABLE_MEGA_HEALTH },
	{ "Secret Chip",		COLLECTABLE_SECRET_CHIP },

	{ "Arm Servo L1",		COLLECTABLE_ARM_SERVO_L1 },
	{ "Arm Servo L2",		COLLECTABLE_ARM_SERVO_L2 },
	{ "Arm Servo L3",		COLLECTABLE_ARM_SERVO_L3 },

	// Weapons
	{ "Laser L1",			COLLECTABLE_WEAPON_LASER_L1 },
	{ "Laser L2",			COLLECTABLE_WEAPON_LASER_L2 },
	{ "Laser L3",			COLLECTABLE_WEAPON_LASER_L3 },
	{ "Ripper L1",			COLLECTABLE_WEAPON_RIPPER_L1 },
	{ "Ripper L2",			COLLECTABLE_WEAPON_RIPPER_L2 },
	{ "Ripper L3",			COLLECTABLE_WEAPON_RIPPER_L3 },
	{ "Rivet Gun L1",		COLLECTABLE_WEAPON_RIVET_GUN_L1 },
	{ "Rivet Gun L2",		COLLECTABLE_WEAPON_RIVET_GUN_L2 },
	{ "Rivet Gun L3",		COLLECTABLE_WEAPON_RIVET_GUN_L3 },
	{ "RLauncher L1",		COLLECTABLE_WEAPON_ROCKET_L1 },
	{ "RLauncher L2",		COLLECTABLE_WEAPON_ROCKET_L2 },
	{ "RLauncher L3",		COLLECTABLE_WEAPON_ROCKET_L3 },
	{ "Blaster L1",			COLLECTABLE_WEAPON_BLASTER_L1 },
	{ "Blaster L2",			COLLECTABLE_WEAPON_BLASTER_L2 },
	{ "Blaster L3",			COLLECTABLE_WEAPON_BLASTER_L3 },
	{ "Tether L1",			COLLECTABLE_WEAPON_TETHER_L1 },
	{ "Tether L2",			COLLECTABLE_WEAPON_TETHER_L2 },
	{ "Tether L3",			COLLECTABLE_WEAPON_TETHER_L3 },
	{ "Spew L1",			COLLECTABLE_WEAPON_SPEW_L1 },
	{ "Spew L2",			COLLECTABLE_WEAPON_SPEW_L2 },
	{ "Spew L3",			COLLECTABLE_WEAPON_SPEW_L3 },
	{ "Scope L1",			COLLECTABLE_WEAPON_SCOPE_L1 },
	{ "Scope L2",			COLLECTABLE_WEAPON_SCOPE_L2 },
	{ "Mortar L1",			COLLECTABLE_WEAPON_MORTAR_L1 },
	{ "Flamer L1",			COLLECTABLE_WEAPON_FLAMER_L1 },
	
	{ "Coring Charge",		COLLECTABLE_WEAPON_CORING_CHARGE },
	{ "EMP Grenade",		COLLECTABLE_WEAPON_EMP },
	{ "Magma Bomb",			COLLECTABLE_WEAPON_MAGMA_BOMB },
	{ "Cleaner",			COLLECTABLE_WEAPON_CLEANER },
	{ "Wrench",				COLLECTABLE_WEAPON_WRENCH },
	{ "Recruiter Grenade",	COLLECTABLE_WEAPON_RECRUITER },
#if FANG_WINGC
	{ "megawasher",		COLLECTABLE_MEGA_WASHER },
#endif
};

static const u32 _uNumCollectableTypes		= sizeof( _collectableTypes ) / sizeof( NameType_t );

static CCollectableBuilder _CollectableBuilder;

typedef struct {
	cchar *pszMeshName;
	u32 uPoolSize;
	s32 nAmmoCount;
	CFSoundGroup *pPickupSound;
	CFSoundGroup *pPickupSoundWeapon;
	CFSoundGroup *pRespawnSound;
	cchar *pszParticleName;
	cchar *pszRespawnParticleName;
	cchar *pszEUKMeshName;
	cchar *pszMeshAnimName;
	f32 fHUDScale;
	f32 fHUDScaleSpecialEUK;
} CollectableUserProp_t;

#if FANG_WINGC
// Retail goodies.csv layout, from the retail vocabulary in main.dol (tools/dol_vocab.py
// --scan 12). It adds an s32 after nAmmoCount and a u32 at the end, keeps three strings
// between the sounds and the HUD scale, and has no separate special-EUK HUD scale. The
// middle string is "none" in every retail table, so it cannot tell the respawn particle
// from the EUK mesh; both are NULL either way. The two new values are kept but unused.
typedef struct {
	cchar *pszMeshName;
	u32 uPoolSize;
	s32 nAmmoCount;
	s32 nRetailCount2;				// e.g. 100 where nAmmoCount is 50 (ripper); meaning unconfirmed
	CFSoundGroup *pPickupSound;
	CFSoundGroup *pPickupSoundWeapon;
	CFSoundGroup *pRespawnSound;
	cchar *pszParticleName;
	cchar *pszRespawnParticleName;
	cchar *pszMeshAnimName;
	f32 fHUDScale;
	u32 uRetailFlag;				// 1 for energy, mega health, battery and arm servos; meaning unconfirmed
} CollectableRetailUserProp_t;

static const FGameData_TableEntry_t _aRetailGameDataVocab[] = {
	FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, sizeof( cchar * ), F32_DATATABLE_0, F32_DATATABLE_0,
	FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, sizeof( u32 ), F32_DATATABLE_0, F32_DATATABLE_10000,
	FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_CONVERT_TO_S32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, sizeof( s32 ), F32_DATATABLE_Neg1, F32_DATATABLE_10000,
	FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_CONVERT_TO_S32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, sizeof( s32 ), F32_DATATABLE_Neg1, F32_DATATABLE_10000,
	FGAMEDATA_VOCAB_SOUND_GROUP,
	FGAMEDATA_VOCAB_SOUND_GROUP,
	FGAMEDATA_VOCAB_SOUND_GROUP,
	FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, sizeof( cchar * ), F32_DATATABLE_0, F32_DATATABLE_0,
	FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, sizeof( cchar * ), F32_DATATABLE_0, F32_DATATABLE_0,
	FGAMEDATA_VAR_TYPE_STRING | FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL, sizeof( cchar * ), F32_DATATABLE_0, F32_DATATABLE_0,
	FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, sizeof( f32 ), F32_DATATABLE_POS_EPSILON, F32_DATATABLE_10000,
	FGAMEDATA_VAR_TYPE_FLOAT | FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO, sizeof( u32 ), F32_DATATABLE_0, F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

// Read one retail goodies table into the source layout.
static BOOL _GetRetailCollectableTableData( FGameDataTableHandle_t hTable, CollectableUserProp_t *pUserProp ) {
	CollectableRetailUserProp_t Retail;

	if( fgamedata_GetNumFields( hTable ) != 12 ||
		!fgamedata_GetTableData( hTable, _aRetailGameDataVocab, &Retail, sizeof( Retail ) ) ) {
		return FALSE;
	}

	pUserProp->pszMeshName = Retail.pszMeshName;
	pUserProp->uPoolSize = Retail.uPoolSize;
	pUserProp->nAmmoCount = Retail.nAmmoCount;
	pUserProp->pPickupSound = Retail.pPickupSound;
	pUserProp->pPickupSoundWeapon = Retail.pPickupSoundWeapon;
	pUserProp->pRespawnSound = Retail.pRespawnSound;
	pUserProp->pszParticleName = Retail.pszParticleName;
	pUserProp->pszRespawnParticleName = Retail.pszRespawnParticleName;
	pUserProp->pszEUKMeshName = NULL;
	pUserProp->pszMeshAnimName = Retail.pszMeshAnimName;
	pUserProp->fHUDScale = Retail.fHUDScale;
	pUserProp->fHUDScaleSpecialEUK = Retail.fHUDScale;
	return TRUE;
}
#endif

const FGameData_TableEntry_t CCollectableType::m_aGameDataVocab[] = {
	// pszMeshName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// uPoolSize
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// nAmmoCount
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_S32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( s32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_10000,

	// pPickupSound
	FGAMEDATA_VOCAB_SOUND_GROUP,

	// pPickupSoundWeapon
	FGAMEDATA_VOCAB_SOUND_GROUP,

	// pRespawnSound
	FGAMEDATA_VOCAB_SOUND_GROUP,

	// pszParticleName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszParticleRespawnName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszEUKMeshName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// pszMeshAnimName
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL | FGAMEDATA_FLAGS_STRING_NONE_TO_NULL,
	sizeof( cchar * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fHUDScale
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_POS_EPSILON,
	F32_DATATABLE_10000,

	// fHUDScaleSpecialEUK
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_POS_EPSILON,
	F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

FMeshTexLayerHandle_t CCollectableType::m_hEUKTexLayer = -1;

typedef struct {
	u32 uPoolSize;
	s32 nAmmoCount;
} CollectableOverrideUserProp_t;

const FGameData_TableEntry_t CCollectable::m_aGameDataVocabOverride[] = {
	// uPoolSize
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_10000,

	// nAmmoCount
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_S32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( s32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_10000,

	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};

BOOL CCollectableType::Setup( void ) {
	FResFrame_t Res = fres_GetFrame();

	// If the pools have been allocated, return
	if( m_bSetup ) {
		return TRUE;
	}

#if FANG_WINGC
	// World pickups can remain for partners while each HUD and the shop borrow display meshes.
	if( MultiplayerMgr.IsSinglePlayer() && CPlayer::m_nPlayerCount > 1 ) {
		const u32 nMinimum = 2 * CPlayer::m_nPlayerCount + 3;
		if( m_uMeshPoolSize < nMinimum ) m_uMeshPoolSize = nMinimum;
	}
#endif

	// _SetupAnimation() mush happen before the normal mesh pool is created
	if( !_SetupAnimation() ) {
		goto _ExitWithError;
	}

	// Do this before the mesh pool since it sets up the postering
	switch( m_eType ) {
		case COLLECTABLE_BATTERY:
		case COLLECTABLE_PUP_ENERGY:
		case COLLECTABLE_ARM_SERVO_L1:
		case COLLECTABLE_ARM_SERVO_L2:
		case COLLECTABLE_ARM_SERVO_L3:
			m_bShouldPoster = TRUE;
		break;
		default:
			m_bShouldPoster = FALSE;
		break;
	}

	if( !_SetupMeshPool( m_uMeshPoolSize, &m_pMeshPool, &m_pMesh, m_pszMeshName, TRUE ) ) {
		goto _ExitWithError;
	}

	// Only load the special EUK if we need it
	if( m_bSpecialEUK ) {
		if( m_pszEUKMeshName ) {
			if( !_SetupMeshPool( m_uMeshPoolSize, &m_pEUKMeshPool, &m_pEUKMesh, m_pszEUKMeshName, FALSE, TRUE ) ) {
				goto _ExitWithError;
			}
		} else {
			// We didn't get a mesh name, don't bother thinking it is special
			m_bSpecialEUK = FALSE;
		}
	}

	// Load particles
	// In world particles
	if( m_pszParticleName != NULL ) { 
		m_hParticleDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_pszParticleName );

		if( m_hParticleDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CCollectableType::Setup(): Failed to load particle '%s'.\n", m_pszParticleName );
		}
	}

	// Respawn particles
	if( m_pszParticleRespawnName != NULL ) { 
		m_hParticleRespawnDef = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, m_pszParticleRespawnName );

		if( m_hParticleRespawnDef == FPARTICLE_INVALID_HANDLE ) {
			DEVPRINTF( "CCollectableType::Setup(): Failed to load particle '%s'.\n", m_pszParticleRespawnName );
		}
	}

	m_bSetup = TRUE;
	m_bNeedSetup = FALSE;

	return TRUE;

_ExitWithError:
	if( m_pMeshPool ) {
		CFMeshPool::DestroyPool( m_pMeshPool );
		m_pMeshPool = NULL;
	}

	if( m_pEUKMeshPool ) {
		CFMeshPool::DestroyPool( m_pEUKMeshPool );
		m_pEUKMeshPool = NULL;
	}

	if( m_pAnimInst ) {
		fdelete( m_pAnimInst );
		m_pAnimInst = NULL;
	}

	if( m_paAnimCombiners ) {
		for( u32 i = 0; i < m_uMeshPoolSize; ++i ) {
			fdelete( m_paAnimCombiners[ i ] );
		}

		m_paAnimCombiners = NULL;
	}

	fres_ReleaseFrame( Res );

	return FALSE;
}

void CCollectableType::Reset( void ) {
	if( m_pMeshPool ) {
		CFMeshPool::DestroyPool( m_pMeshPool );
	}

	if( m_pEUKMeshPool ) {
		CFMeshPool::DestroyPool( m_pEUKMeshPool );
	}

	if( m_pAnimInst ) {
		fdelete( m_pAnimInst );
	}

	if( m_paAnimCombiners ) {
		for( u32 i = 0; i < m_uMeshPoolSize; ++i ) {
			fdelete( m_paAnimCombiners[ i ] );
		}

		m_paAnimCombiners = NULL;
	}

	m_pMesh = NULL;
	m_pMeshPool = NULL;

	m_pEUKMeshPool = NULL;
	m_pEUKMesh = NULL;

	m_bSetup = FALSE;
	m_bNeedSetup = FALSE;
	m_bSpecialEUK = FALSE;
	m_bShouldPoster = FALSE;

	m_pAnimInst = NULL;

	m_hEUKTexLayer = -1;
}

CFWorldMesh *CCollectableType::GetWorldMesh( BOOL *pbIsEUKMesh/* = NULL*/ ) {
	CFWorldMesh *pMesh = NULL;

	if( !m_bSpecialEUK ) {
		if( !m_pMeshPool ) {
			return NULL;
		}

		pMesh = m_pMeshPool->GetFromFreePool();

		if( pMesh ) {
			pMesh->m_pUser = m_pMeshPool;
		}

		if( pbIsEUKMesh ) {
			(*pbIsEUKMesh) = FALSE;
		}
	} else {
		if( !m_pEUKMeshPool ) {
			return NULL;
		}

		pMesh = m_pEUKMeshPool->GetFromFreePool();

		if( pMesh ) {
			pMesh->m_pUser = m_pEUKMeshPool;
		}

		if( pbIsEUKMesh ) {
			(*pbIsEUKMesh) = TRUE;
		}
	}

#if FANG_WINGC
	// Checkpoint restore returns the whole pool at once; a new HUD/shop borrower must not
	// inherit the previous world pickup's per-player visibility predicate.
	if( pMesh ) pMesh->SetDrawFilter( NULL, NULL );
#endif
	return pMesh;
}

void CCollectableType::ReturnWorldMesh( CFWorldMesh *pMesh ) {
	if( !pMesh ) {
		return;
	}

	if( pMesh->m_pUser == m_pMeshPool ) {
		if( !m_pMeshPool ) {
			return;
		}

		m_pMeshPool->ReturnToFreePool( (CFWorldMeshItem *) pMesh );
	} else {
		if( !m_pEUKMeshPool ) {
			return;
		}

		m_pEUKMeshPool->ReturnToFreePool( (CFWorldMeshItem *) pMesh );
	}
}

void CCollectableType::ForceReturnAllMeshes( void ) {
	if( m_pMeshPool ) {
		m_pMeshPool->ReturnAllToFreePool();
	}

	if( m_pEUKMeshPool ) {
		m_pEUKMeshPool->ReturnAllToFreePool();
	}
}

void CCollectableType::_ClearDataMembers( void ) {
	m_pszName = NULL;
	m_pszMeshName = NULL;

	m_pMesh = NULL;
	m_pEUKMesh = NULL;

	m_pMeshPool = NULL;
	m_pEUKMeshPool = NULL;
	
	m_uMeshPoolSize = 0;

	m_eType = COLLECTABLE_UNKNOWN;
	m_uItemRepositoryIndex = ITEM_UNKNOWN;
    m_nAmmoCount = 0;

	m_pszParticleName = NULL;
	m_pszParticleRespawnName = NULL;
	m_pszEUKMeshName = NULL;
	m_pszMeshAnimName = NULL;

	m_pPickupSound			= NULL;
	m_pPickupWeaponSound	= NULL;
	m_pRespawnSound			= NULL;

	m_hParticleDef			= FPARTICLE_INVALID_HANDLE;
	m_hParticleRespawnDef	= FPARTICLE_INVALID_HANDLE;

	m_bSetup = FALSE;
	m_bNeedSetup = FALSE;
	m_bSpecialEUK = FALSE;
	m_bShouldPoster = FALSE;

	m_pAnimInst = NULL;
	m_paAnimCombiners = NULL;

	m_fHUDScale = 0.0f;
	m_fHUDScaleSpecialEUK = 0.0f;
}

BOOL CCollectableType::_SetupMeshPool( u32 uPoolSize, CFMeshPool **pMeshPool, FMesh_t **pMesh, cchar *pszMeshName, BOOL bUseAnimation/* = FALSE*/, BOOL bIsSpecialEUK/* = FALSE*/ ) {
	CFWorldMeshItem **paMeshes = NULL;
	FMeshInit_t  MeshInit;
	CFWorldMeshItem *pWorldMesh;
	u32 i;
	FResFrame_t Res = fres_GetFrame();
	FMemFrame_t Mem = fmem_GetFrame();

	*pMeshPool = NULL;
	*pMesh = NULL;

	*pMeshPool = CFMeshPool::CreatePool( uPoolSize );

	if( !(*pMeshPool) ) {
		DEVPRINTF( "CCollectable::Setup(): Not enough memory for CFMeshPool of size %d.\n", uPoolSize );
		goto _ExitWithError;
	}

	*pMesh = (FMesh_t *) fresload_Load( FMESH_RESTYPE, pszMeshName );

	if( !(*pMesh) ) {
		DEVPRINTF( "CCollectable::Setup(): Could not find mesh '%s'\n", pszMeshName );
		goto _ExitWithError;
	}

	paMeshes = (CFWorldMeshItem **) fmem_Alloc( sizeof( CFWorldMeshItem * ) * uPoolSize );

	if( paMeshes == NULL ) {
		DEVPRINTF( "CCollectable::Setup(): Not enough memory for array of CFWorldMeshes.\n" );
		goto _ExitWithError;
	}

	MeshInit.pMesh		= *pMesh;
	MeshInit.nFlags		= 0;
	MeshInit.fCullDist	= FMATH_MAX_FLOAT;
	MeshInit.Mtx.Identity();

	for( i = 0; i < m_uMeshPoolSize; ++i ) {
		pWorldMesh = (*pMeshPool)->GetFromFreePool();

		// We should ALWAYS have a valid item
		FASSERT( pWorldMesh );

		pWorldMesh->Init( &MeshInit );
		pWorldMesh->m_nUser = MESHTYPES_UNKNOWN;
		pWorldMesh->m_pUser = NULL;
		pWorldMesh->SetUserTypeBits( 0 );
		pWorldMesh->m_nFlags &= ~FMESHINST_FLAG_DONT_DRAW;
		pWorldMesh->m_nFlags |= FMESHINST_FLAG_NOCOLLIDE;
		pWorldMesh->SetCollisionFlag( FALSE );
		pWorldMesh->SetLineOfSightFlag( FALSE );
		pWorldMesh->RemoveFromWorld();

		if( m_bShouldPoster ) {
			pWorldMesh->m_nFlags |= ( FMESHINST_FLAG_POSTER_X | FMESHINST_FLAG_POSTER_Y | FMESHINST_FLAG_POSTER_BONES ); 
		}

		if( bUseAnimation && m_paAnimCombiners ) {
			m_paAnimCombiners[ i ]->CreateSimple( m_pAnimInst, pWorldMesh );
		}

		// We do this only once since we assume that all special EUKs use the same mesh
		// The exception to this is the scope, which has its own mesh
		if( bIsSpecialEUK && m_hEUKTexLayer == -1 && 
			m_eType != COLLECTABLE_WEAPON_SCOPE_L1 && m_eType != COLLECTABLE_WEAPON_SCOPE_L2 ) {

			m_hEUKTexLayer = pWorldMesh->GetTexLayerHandle( _EUK_MESH_ID );
		}

		// VERY IMPORTANT NOTE
		//
		// We assume that if we have a special EUK mesh that they will all be the same outside of the scope.
		if( bIsSpecialEUK ) {
			RemapTCs( pWorldMesh, m_eType );
		}

		paMeshes[i] = pWorldMesh;
	}

	for( i = 0; i < m_uMeshPoolSize; ++i ) {
		(*pMeshPool)->ReturnToFreePool( paMeshes[i] );
	}

	fmem_ReleaseFrame( Mem );

	return TRUE;

_ExitWithError:
	if( (*pMeshPool) != NULL ) {
		CFMeshPool::DestroyPool( (*pMeshPool) );
		*pMeshPool = NULL;
	}

	fmem_ReleaseFrame( Mem );
	fres_ReleaseFrame( Res );

	return FALSE;
}

BOOL CCollectableType::_SetupAnimation( void ) {
	// If we have a special EUK mesh, don't allow animation
	if( m_pszEUKMeshName && m_bSpecialEUK ) {
		return TRUE;
	}

	FResFrame_t Res = fres_GetFrame();

	if( m_pszMeshAnimName ) {
		FAnim_t *pAnimRes;
		
		pAnimRes = (FAnim_t *)fresload_Load( FANIM_RESNAME, m_pszMeshAnimName );

		if( !pAnimRes ) {
			DEVPRINTF( "CCollectableType::Setup(): Failed to load animation '%s'.\n", m_pszMeshAnimName );
			goto _ExitWithError;
		}

		m_pAnimInst = fnew CFAnimInst;

		if( m_pAnimInst == NULL ) {
			DEVPRINTF( "CCollectableType::Setup(): No memory for anim inst.\n" );
			goto _ExitWithError;
		}

		if( !m_pAnimInst->Create( pAnimRes ) ) {
			DEVPRINTF( "CCollectableType::Setup(): Failed to create anim inst.\n" );
			goto _ExitWithError;
		}

		m_paAnimCombiners = (CFAnimCombiner **) fres_Alloc( sizeof( CFAnimCombiner * ) * m_uMeshPoolSize );

		if( !m_paAnimCombiners ) {
			DEVPRINTF( "CCollectableType::Setup(): No memory for anim combiners.\n" );
			goto _ExitWithError;
		}

		for( u32 i = 0; i < m_uMeshPoolSize; ++i ) {
			m_paAnimCombiners[ i ] = fnew CFAnimCombiner();

			if( !m_paAnimCombiners[ i ] ) {
				DEVPRINTF( "CCollectableType::Setup(): No memory for anim combiners.\n" );
				goto _ExitWithError;
			}
		}

		// Note - The combiner will be setup later when the mesh pool is created.  We don't get the
		//		  CFWorldMeshes until that point.
	}

	return TRUE;

_ExitWithError:
	if( m_pAnimInst ) {
		fdelete( m_pAnimInst );
		m_pAnimInst = NULL;
	}

	if( m_paAnimCombiners ) {
		for( u32 i = 0; i < m_uMeshPoolSize; ++i ) {
			fdelete( m_paAnimCombiners[ i ] );
		}

		m_paAnimCombiners = NULL;
	}

	return FALSE;
}

// This is an artist value
// If the image thf2wpnsp01.tga changes you will need to change this function
#define _TC_MOVE ( 64.0f / 256.0f )

void CCollectableType::RemapTCs( CFWorldMesh *pWorldMesh, CollectableType_e eType ) {
	FASSERT( pWorldMesh );
	f32 fX = 0.0f;
	f32 fY = 0.0f;

	switch( eType ) {
		case COLLECTABLE_WEAPON_SCOPE_L1:
		case COLLECTABLE_WEAPON_SCOPE_L2:
			// The scope does nothing	
			return;
		break;

		case COLLECTABLE_WEAPON_LASER_L1:
		case COLLECTABLE_WEAPON_LASER_L2:
		case COLLECTABLE_WEAPON_LASER_L3:
			fX = _TC_MOVE;
		break;
		case COLLECTABLE_WEAPON_RIPPER_L1:
		case COLLECTABLE_WEAPON_RIPPER_L2:
		case COLLECTABLE_WEAPON_RIPPER_L3:
			fX = _TC_MOVE * 3.0f;
			fY = _TC_MOVE;
		break;
		case COLLECTABLE_WEAPON_RIVET_GUN_L1:
		case COLLECTABLE_WEAPON_RIVET_GUN_L2:
		case COLLECTABLE_WEAPON_RIVET_GUN_L3:
			// nothing to do
		break;
		case COLLECTABLE_WEAPON_ROCKET_L1:
		case COLLECTABLE_WEAPON_ROCKET_L2:
		case COLLECTABLE_WEAPON_ROCKET_L3:
			fX = _TC_MOVE;
			fY = _TC_MOVE;
		break;
		case COLLECTABLE_WEAPON_BLASTER_L1:
		case COLLECTABLE_WEAPON_BLASTER_L2:
		case COLLECTABLE_WEAPON_BLASTER_L3:
			fX = 2.0f * _TC_MOVE;
		break;
		case COLLECTABLE_WEAPON_TETHER_L1:
		case COLLECTABLE_WEAPON_TETHER_L2:
		case COLLECTABLE_WEAPON_TETHER_L3:
			fX = _TC_MOVE * 3.0f;
			fY = _TC_MOVE * 2.0f;
		break;
		case COLLECTABLE_WEAPON_SPEW_L1:
		case COLLECTABLE_WEAPON_SPEW_L2:
		case COLLECTABLE_WEAPON_SPEW_L3:
			fX = _TC_MOVE * 2.0f;
			fY = _TC_MOVE;
		break;
		case COLLECTABLE_WEAPON_FLAMER_L1:
			fY = _TC_MOVE * 2.0f;
		break;

		default:
			// If we don't have anything above, leave it alone
			return;
		break;
	}

	CFVec2 Rate;
	CFVec2 ST;

	Rate.Zero();
	ST.Set( fX, fY );

	// If this assert is hit a bad EUK mesh was used
	FASSERT( pWorldMesh->m_pMesh->nTexLayerIDCount > 0 );

	// !!Nate - THIS IS A TEMPORARY FIX FOR THE TIME BEING
	FMeshTexLayerHandle_t hHandle = pWorldMesh->GetTexLayerHandle( _EUK_MESH_ID );

	pWorldMesh->AnimTC_SetScrollRate( hHandle, Rate );
	pWorldMesh->AnimTC_SetScrollST( hHandle, ST );
}




BOOL CCollectable::m_bSystemInitialized = FALSE;

FLinkRoot_t CCollectable::m_CollectablePoolAlive;
FLinkRoot_t CCollectable::m_CollectablePoolDead;
FLinkRoot_t CCollectable::m_CollectableTypeList;
FLinkRoot_t CCollectable::m_CollectableSpawnPoolAlive;
FLinkRoot_t CCollectable::m_CollectableSpawnPoolDead;

f32 CCollectable::m_fRotY = 0.0f;
f32 CCollectable::m_fTransY = 0.0f;

CFCollInfo CCollectable::m_CollInfo;
CFTrackerCollideProjSphereInfo CCollectable::CollProjSphereInfo;

CPlayer *CCollectable::m_pPlayer = NULL;
CBot *CCollectable::m_pCollectBot = NULL;
CHud2 *CCollectable::m_pCollectHud = NULL;

CollectableCallback_t *CCollectable::m_pCollectionCallback = NULL;

FParticle_DefHandle_t CCollectable::m_hSpecialPickup = FPARTICLE_INVALID_HANDLE;
CFSoundGroup *CCollectable::m_pSpecialGroup = NULL;

CFCheckPoint::ObjectDataHandle_t CCollectable::m_hSaveData[ FCHECKPOINT_MAX_CHECKPOINTS ];

CFDebrisGroup *CCollectable::m_pGlassGroup = NULL;

BOOL CCollectable::InitSystem( void ) {
	FASSERT( !IsSystemInitialized() );
	
	FResFrame_t hResFrame = fres_GetFrame();
	CCollectable *pCollectable = NULL;
	CCollectableSpawnPoint *pSpawnPoint = NULL;

	m_bSystemInitialized = TRUE;

	flinklist_InitRoot( &m_CollectablePoolAlive, FANG_OFFSETOF( CCollectable, m_Link ) );
	flinklist_InitRoot( &m_CollectablePoolDead, FANG_OFFSETOF( CCollectable, m_Link ) );

	for( u32 i = 0; i < _COLLECTABLE_POOL_SIZE; ++i ) {
		pCollectable = fnew CCollectable;

		if( pCollectable == NULL ) {
			DEVPRINTF( "CCollectable::InitSystem(): Failed to allocate memory for collectable.\n" );
			goto _ExitInitSystemWithError;
		}

		pCollectable->_ClearDataMembers();
		pCollectable->m_bInActiveList = FALSE;
		pCollectable->m_bOverPoolAlloc = FALSE;
		pCollectable->m_bUsedInScript = FALSE;

		flinklist_AddTail( &m_CollectablePoolDead, pCollectable );
	}

	// Init the spawn point list
	flinklist_InitRoot( &m_CollectableSpawnPoolAlive, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );
	flinklist_InitRoot( &m_CollectableSpawnPoolDead, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );

	for( u32 i = 0; i < _SPAWN_POOL_SIZE; ++i ) {
		pSpawnPoint = fnew CCollectableSpawnPoint;

		if( pSpawnPoint == NULL ) {
			DEVPRINTF( "CCollectable::InitSystem(): Failed to allocate memory for spawn point.\n" );
			goto _ExitInitSystemWithError;
		}

		flinklist_AddTail( &m_CollectableSpawnPoolDead, pSpawnPoint );
	}

	flinklist_InitRoot( &m_CollectableTypeList, FANG_OFFSETOF( CCollectableType, m_Link ) );

	// Make sure to load sound bank before CSV
	if( !fresload_Load( FSNDFX_RESTYPE, _COLLECTABLE_SOUNDS ) ) {
		DEVPRINTF( "CCollectable::InitSystem(): Could not load sound effect bank '%s'\n", _COLLECTABLE_SOUNDS );
	}

	m_pSpecialGroup = CFSoundGroup::RegisterGroup( _PICKUP_SPECIAL_SOUND );

	if( !_LoadCollectableCSV() ) {
		goto _ExitInitSystemWithError;
	}

	m_hSpecialPickup = (FParticle_DefHandle_t) fresload_Load( FPARTICLE_RESTYPE, _COLLECTABLE_SPECIAL_PICKUP );

	if( m_hSpecialPickup == FPARTICLE_INVALID_HANDLE ) {
		DEVPRINTF( "CCollectable::InitSystem(): Failed to load particle '%s'.\n", _COLLECTABLE_SPECIAL_PICKUP );
	}

	m_fRotY = 0.0f;
	m_fTransY = 0.0f;

	m_CollInfo.nCollTestType = FMESH_COLLTESTTYPE_PROJSPHERE;
	m_CollInfo.bFindClosestImpactOnly = FALSE;
	m_CollInfo.nStopOnFirstOfCollMask = FCOLL_MASK_NONE;
	m_CollInfo.bCullBacksideCollisions = TRUE;
	m_CollInfo.bCalculateImpactData = TRUE;
	m_CollInfo.nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	m_CollInfo.nResultsLOD = FCOLL_LOD_HIGHEST;
	m_CollInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;

	CollProjSphereInfo.pProjSphere = &m_CollInfo.ProjSphere;
	CollProjSphereInfo.bIgnoreCollisionFlag = FALSE;
	CollProjSphereInfo.nTrackerUserTypeBitsMask = FCOLL_USER_TYPE_BITS_ALL;
	CollProjSphereInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	CollProjSphereInfo.pCallback = _TrackersCallbackProjSphere;
	CollProjSphereInfo.nTrackerSkipCount = 0;
	CollProjSphereInfo.ppTrackerSkipList = NULL;

	m_pGlassGroup = CFDebrisGroup::Find( "GlassMed" );

	return TRUE;

_ExitInitSystemWithError:
	fres_ReleaseFrame( hResFrame );

	return FALSE;
}

void CCollectable::UninitSystem( void ) {
	if( m_bSystemInitialized ) {
		CCollectable *pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );
		CCollectable *pNext = NULL;

		// Free the active ones
		while( pCollectable ) {
			pNext = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCollectable );
			fdelete( pCollectable );
			pCollectable = pNext;
		}

		// Free the dead ones
		pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolDead );
		pNext = NULL;

		while( pCollectable ) {
			pNext = (CCollectable *) flinklist_GetNext( &m_CollectablePoolDead, pCollectable );
			fdelete( pCollectable );
			pCollectable = pNext;
		}

		// Release the pool of collectables
		flinklist_InitRoot( &m_CollectablePoolAlive, FANG_OFFSETOF( CCollectable, m_Link ) );
		flinklist_InitRoot( &m_CollectablePoolDead, FANG_OFFSETOF( CCollectable, m_Link ) );

		// Release our collectable types
		CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );
		CCollectableType *pCNext = NULL;

		while( pType ) {
			pCNext = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
			fdelete( pType );
			pType = pCNext;
		}

		flinklist_InitRoot( &m_CollectableTypeList, FANG_OFFSETOF( CCollectableType, m_Link ) );

		// Release any spawn points that we may have 
		CCollectableSpawnPoint *pPoint = NULL;
		CCollectableSpawnPoint *pPointNext = NULL;

		pPoint = (CCollectableSpawnPoint *) flinklist_GetHead( &m_CollectableSpawnPoolAlive );

		while( pPoint ) {
			pPointNext = (CCollectableSpawnPoint *) flinklist_GetNext( &m_CollectableSpawnPoolAlive, pPoint );
			fdelete( pPoint );
			pPoint = pPointNext;
		}

		pPoint = (CCollectableSpawnPoint *) flinklist_GetHead( &m_CollectableSpawnPoolDead );

		while( pPoint ) {
			pPointNext = (CCollectableSpawnPoint *) flinklist_GetNext( &m_CollectableSpawnPoolDead, pPoint );
			fdelete( pPoint );
			pPoint = pPointNext;
		}

		flinklist_InitRoot( &m_CollectableSpawnPoolDead, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );
		flinklist_InitRoot( &m_CollectableSpawnPoolAlive, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );

		m_pGlassGroup = NULL;
		m_bSystemInitialized = FALSE;
	}
}

BOOL CCollectable::InitLevel( void ) {
	CCollectable *pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolDead );

	// Create anything that is in the dead pool and not created
	while( pCollectable ) {
		if( !pCollectable->IsCreated() ) {
			if( !pCollectable->Create() ) {
				DEVPRINTF( "CCollectable::InitLevel(): Failed to create collectable.\n" );
				return FALSE;
			}			
		}

		pCollectable->RemoveFromWorld( TRUE );
		pCollectable = (CCollectable *) flinklist_GetNext( &m_CollectablePoolDead, pCollectable );
	}

#if 0
	if( Level_aInfo[ Level_nLoadedIndex ].nLevel == LEVEL_RACE_TO_THE_ROCKET ) {
		if( !_RaceToTheRocketSetup() ) {
			return FALSE;
		}
	}
#endif

	return TRUE;
}

void CCollectable::UnInitLevel( void ) {
	CCollectable *pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );
	CCollectable *pNext = NULL;

	while( pCollectable ) {
		pNext = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCollectable );
		ReturnCollectable( pCollectable );
		pCollectable = pNext;
	}

	// Delete any collectables we created at level init time
	pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolDead );

	while( pCollectable ) {
		// At end of level, all script collectables are free game again
		pCollectable->m_bUsedInScript = FALSE;

		// NKM - Make sure we Destroy() all entities that we have created so they get rebuilt again.
		pCollectable->Destroy();

		pNext = (CCollectable *) flinklist_GetNext( &m_CollectablePoolDead, pCollectable );

		if( pCollectable->m_bOverPoolAlloc ) {
			flinklist_Remove( &m_CollectablePoolDead, pCollectable );
			fdelete( pCollectable );
		}

		pCollectable = pNext;
	}

	flinklist_InitRoot( &m_CollectablePoolAlive, FANG_OFFSETOF( CCollectable, m_Link ) );

	// Reset all of the types
	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );
	CCollectableType *pCNext = NULL;

	while( pType ) {
		pCNext = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
		
		pType->Reset();
		pType = pCNext;
	}

	// Put the spawn points back
	CCollectableSpawnPoint *pPoint = NULL;
	CCollectableSpawnPoint *pPointNext = NULL;

	pPoint = (CCollectableSpawnPoint *) flinklist_GetHead( &m_CollectableSpawnPoolAlive );

	while( pPoint ) {
		pPointNext = (CCollectableSpawnPoint *) flinklist_GetNext( &m_CollectableSpawnPoolAlive, pPoint );
		flinklist_AddTail( &m_CollectableSpawnPoolDead, pPoint );
		pPoint = pPointNext;
	}

	flinklist_InitRoot( &m_CollectableSpawnPoolAlive, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );

	m_pCollectionCallback = NULL;
}

BOOL CCollectable::LoadOverrideCSV( cchar *pszCSVName ) {
	FASSERT( IsSystemInitialized() );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	
	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( pszCSVName );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CCollectable::LoadOverrideCSV(): Could not load collectable override CSV file '%s.csv'.\n", pszCSVName );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		_LoadFromOverrideGameData( hTable );
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );

	return FALSE;
}

void CCollectable::CheckpointSaveGlobal( s32 nCheckpointID ) {
	m_hSaveData[ nCheckpointID ] = CFCheckPoint::CreateObjectDataHandle();

	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	while( pType ) {
		CFCheckPoint::SaveData( (BOOL) pType->m_bSpecialEUK );
		pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
	}
}

// This is so that if we are waiting on any spawns, they get nuked
void CCollectable::CheckpointRestoreGlobal( s32 nCheckpointID ) {
	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );
	BOOL bSpecial;

	CFCheckPoint::SetObjectDataHandle( m_hSaveData[ nCheckpointID ] );

	while( pType ) {
		CFCheckPoint::LoadData( bSpecial );
		pType->m_bSpecialEUK = bSpecial;
		pType->ForceReturnAllMeshes();

		pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
	}

	// Put the spawn points back
	CCollectableSpawnPoint *pPoint = NULL;
	CCollectableSpawnPoint *pPointNext = NULL;

	pPoint = (CCollectableSpawnPoint *) flinklist_GetHead( &m_CollectableSpawnPoolAlive );

	while( pPoint ) {
		pPointNext = (CCollectableSpawnPoint *) flinklist_GetNext( &m_CollectableSpawnPoolAlive, pPoint );
		flinklist_AddTail( &m_CollectableSpawnPoolDead, pPoint );
		pPoint = pPointNext;
	}

	flinklist_InitRoot( &m_CollectableSpawnPoolAlive, FANG_OFFSETOF( CCollectableSpawnPoint, m_Link ) );
}

CCollectable *CCollectable::GetCollectable( BOOL bAllocateNewIfEmpty/* = FALSE*/ ) {
	CCollectable *pCollectable = NULL;

	if( m_CollectablePoolDead.nCount == 0 ) {
		if( bAllocateNewIfEmpty ) {
			// Allocate a new collectable since we have no dead ones to use
			// Collectables allocated in this way will be destroyed when the level is unloaded
			FResFrame_t Frame = fres_GetFrame();

			pCollectable = fnew CCollectable;

			if( pCollectable ) {
				pCollectable->_ClearDataMembers();
				pCollectable->m_bInActiveList = TRUE;
				pCollectable->m_bOverPoolAlloc = TRUE;
				pCollectable->m_bUsedInScript = FALSE;
				flinklist_AddTail( &m_CollectablePoolAlive, pCollectable );
			} else {
				fres_ReleaseFrame( Frame );
			}

			return pCollectable;
		} else {
			if( m_CollectablePoolAlive.nCount != 0 ) {
				pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );

				while( pCollectable ) {	
					if( !pCollectable->m_bUsedInScript &&
#if FANG_WINGC
						!pCollectable->_IsPersonalWeapon() &&
#endif
						( pCollectable->m_pCollectableType->m_eType != COLLECTABLE_CHIP ) &&
						( pCollectable->m_pCollectableType->m_eType != COLLECTABLE_SECRET_CHIP ) ) {
						ReturnCollectable( pCollectable );
						break;
					}

//					if( pCollectable->m_pCollectableType->m_eType == COLLECTABLE_WASHER ) {
//						ReturnCollectable( pCollectable );
//						break;
//					}
					
					pCollectable = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCollectable );
				}
		
				// !!Nate - TODO don't pull anything that is really needed, eg chips, etc
				// We didn't find any washers to take, so take whatever is at the head
				if( pCollectable == NULL ) {
#if FANG_WINGC
					if( MultiplayerMgr.IsSinglePlayer() && CPlayer::m_nPlayerCount > 1 ) {
						pCollectable = GetCollectable( TRUE );
						if( pCollectable && !pCollectable->Create() ) {
							flinklist_Remove( &m_CollectablePoolAlive, pCollectable );
							fdelete( pCollectable );
							return NULL;
						}
						return pCollectable;
					}
#endif
					pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );

					// We can't re-use anything that is used in a script, it will wreak havok
					if( pCollectable->m_bUsedInScript ) {
						return NULL;
					}
					
					ReturnCollectable( pCollectable );
				}

				return GetCollectable();
			} else {
				DEVPRINTF( "CCollectable::GetCollectable(): Collectable pool is empty!  This should not happen!!!\n" );
				return NULL;
			}
		}
	}

	// Changed this so that we never remove a m_bUsedInScript collectable.
	pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolDead );

	while( pCollectable && pCollectable->m_bUsedInScript ) {
		pCollectable = (CCollectable *) flinklist_GetNext( &m_CollectablePoolDead, pCollectable );
	}

	FASSERT( pCollectable );
    flinklist_Remove( &m_CollectablePoolDead, pCollectable );
	
//	pCollectable = (CCollectable *) flinklist_RemoveHead( &m_CollectablePoolDead );
//	FASSERT( pCollectable );
	flinklist_AddTail( &m_CollectablePoolAlive, pCollectable );
	pCollectable->m_bInActiveList = TRUE;
	pCollectable->m_bOverPoolAlloc = FALSE;
	pCollectable->m_bUsedInScript = FALSE;
#if FANG_WINGC
	pCollectable->m_nCoopCollectedMask = 0;
	pCollectable->m_nRecipientPlayer = -1;
#endif

	return pCollectable;
}

void CCollectable::ReturnCollectable( CCollectable *pCollectable ) {
	if( pCollectable == NULL ) {
		return;
	}

	// Do this first since there is now a check in RemoveFromWorld() for list membership
	pCollectable->m_bInActiveList = FALSE;

	// Remove from active list first since we are a member of that list already
	flinklist_Remove( &m_CollectablePoolAlive, pCollectable );
	flinklist_AddTail( &m_CollectablePoolDead, pCollectable );

	// In some cases we can get here when the entity has not been created
	if( pCollectable->IsCreated() && pCollectable->m_pCollectableType ) {
		pCollectable->_ReturnWorldMesh();
		pCollectable->RemoveFromWorld( TRUE );
	}
}

CCollectableType *CCollectable::GetCollectableTypeObjectFromString( cchar *pszName ) {
	return _FindCollectableType( pszName );
}

CollectableType_e CCollectable::GetCollectableTypeFromString( cchar *pszName ) {
	return _ClassifyCollectableType( pszName );
}

CCollectableType *CCollectable::GetCollectableTypeObjectFromType( CollectableType_e eName ) {
	return _FindCollectableType( eName );
}
// !!!!!!!!!!!!!!
// IMPORTANT NOTE
// !!!!!!!!!!!!!!
// If you change the way this function works in the case when uEUKLevel is 0,
// you will need to change the way NotifyCollectableUsedInWorld() works in the case when bFromBot is TRUE.
CollectableType_e CCollectable::ClassifyBotWeapon( CWeapon *pBotWeapon, u32 uEUKLevel/* = 0*/ ) {
	if( pBotWeapon == NULL ) {
		return COLLECTABLE_UNKNOWN;
	}

	CWeapon::WeaponType_e eWeapType = pBotWeapon->Type();
	CollectableType_e eType = COLLECTABLE_UNKNOWN;

	// All of this taken from the information in weapon.h for the EUKs
	switch( eWeapType ) {
		case CWeapon::WEAPON_TYPE_SPEW:
			FASSERT( uEUKLevel >= 0 && uEUKLevel <= 2 );
			eType = (CollectableType_e) ( COLLECTABLE_WEAPON_SPEW_L1 + uEUKLevel );
		break;
//		case CWeapon::WEAPON_TYPE_FLAMER:
//			FASSERT( uEUKLevel == 0 );
//			eType = COLLECTABLE_WEAPON_FLAMER_L1;
//		break;
//		case CWeapon::WEAPON_TYPE_RIVET_GUN:
//			FASSERT( uEUKLevel >= 0 && uEUKLevel <= 2 );
//			eType = (CollectableType_e) ( COLLECTABLE_WEAPON_RIVET_GUN_L1 + uEUKLevel );
//		break;
//		case CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER:
//			FASSERT( uEUKLevel >= 0 && uEUKLevel <= 2 );
//			eType = (CollectableType_e) ( COLLECTABLE_WEAPON_ROCKET_L1 + uEUKLevel );
//		break;
	}

	return eType;
}

CollectableType_e CCollectable::ClassifyPlayerWeapon( CWeapon *pWeap ) {
	if( pWeap == NULL ) {
		return COLLECTABLE_UNKNOWN;
	}

	CWeapon::WeaponType_e eWeapType = pWeap->Type();
	u32 uUpgrade = pWeap->GetUpgradeLevel();

	// All of Glitch's weapons are EUKs 0 through 2, anything higher is bad
	if( uUpgrade >= 3 ) {
		return COLLECTABLE_UNKNOWN;
	}

	// All of this was taken from weapon.h and the comments in there.  If weapon.h changes, make sure to change this
	switch( eWeapType ) {
		case CWeapon::WEAPON_TYPE_LASER:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_LASER_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_RIVET_GUN:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_RIVET_GUN_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_GRENADE:
			return COLLECTABLE_WEAPON_CORING_CHARGE;
		break;
		case CWeapon::WEAPON_TYPE_BLASTER:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_BLASTER_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_ROCKET_LAUNCHER:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_ROCKET_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_TETHER:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_TETHER_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_SPEW:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_SPEW_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_FLAMER:
			return COLLECTABLE_WEAPON_FLAMER_L1;
		break;
		case CWeapon::WEAPON_TYPE_RIPPER:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_RIPPER_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_CLEANER:
			return COLLECTABLE_WEAPON_CLEANER;
		break;
		case CWeapon::WEAPON_TYPE_SCOPE:
			return (CollectableType_e) ( COLLECTABLE_WEAPON_SCOPE_L1 + uUpgrade );
		break;
		case CWeapon::WEAPON_TYPE_EMP:
			return COLLECTABLE_WEAPON_EMP;
		break;
		case CWeapon::WEAPON_TYPE_MAGMABOMB:
			return COLLECTABLE_WEAPON_MAGMA_BOMB;
		break;
		case CWeapon::WEAPON_TYPE_MORTAR:
			return COLLECTABLE_WEAPON_MORTAR_L1;
		break;
		case CWeapon::WEAPON_TYPE_WRENCH:
			return COLLECTABLE_WEAPON_WRENCH;
		break;
		case CWeapon::WEAPON_TYPE_RECRUITER:
			return COLLECTABLE_WEAPON_RECRUITER;
		break;
	}

	return COLLECTABLE_UNKNOWN;
}

void CCollectable::NotifyCollectableUsedInWorld( cchar *pszName, BOOL bFromBot/* = FALSE*/ ) {
	CCollectableType *pType = NULL;
	cchar *pszLookName = pszName;

	// NKM - This is a hack since we want "coring charge" but the LDs have "coringcharge" in all the worlds.
	if( !fclib_stricmp( _CORING_CHARGE_LDS, pszName ) ) {
		pszLookName = _CORING_CHARGE_COLLECT;
	}

	pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	for( ; pType != NULL; pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType ) ) {
		if( !fclib_stricmp( pType->m_pszName, pszLookName ) ) {
			if( !pType->m_bSetup ) {
				pType->m_bNeedSetup = TRUE;
			}

			// If we know that this came from a bot, we know that the bots return the base EUK level.
			// We need to load all EUK levels since when a bot drops a weapon, he drops the level that
			// Glitch currently has.
			if( bFromBot && _IsWeapon( pType ) && IsPrimaryWeaponType( pType ) ) {
				_ForceNotifyHigherEUK( pType );
			}

			break;
		}
	}

	if( pType == NULL ) {
		DEVPRINTF( "CCollectable::NotifyCollectableUsedInWorld(): Bad collectable name '%s'.\n", pszLookName );
	}
}

void CCollectable::NotifyCollectableUsedInWorld( CollectableType_e eType, BOOL bFromBot/* = FALSE*/ ) {
	// Can't classify unknown, since we don't know what it is!
	if( eType == COLLECTABLE_UNKNOWN ) {
		return;
	}

	CCollectableType *pType = NULL;

	pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	for( ; pType != NULL; pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType ) ) {
		if( pType->m_eType == eType ) {
			if( !pType->m_bSetup ) {
				pType->m_bNeedSetup = TRUE;
			}

			// If we know that this came from a bot, we know that the bots return the base EUK level.
			// We need to load all EUK levels since when a bot drops a weapon, he drops the level that
			// Glitch currently has.
			if( bFromBot && _IsWeapon( pType ) && IsPrimaryWeaponType( pType ) ) {
				_ForceNotifyHigherEUK( pType );
			}

			break;
		}
	}

	if( pType == NULL ) {
		DEVPRINTF( "CCollectable::NotifyCollectableUsedInWorld(): Bad collectable type.\n" );
	}
}

BOOL CCollectable::PostPlayerCreationSetup( void ) {
	FResFrame_t Frame = fres_GetFrame();
	CCollectableType *pType = NULL;

	pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	// Go through and setup any pools that need to be setup.
	// We get notified of these by NotifyCollectableUsedInWorld()
	for( ; pType != NULL; pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType ) ) {

		if( pType->m_bNeedSetup && !pType->m_bSetup ) {
			// If we have a primary weapon, see if Glitch has it.  If he doesn't, we need to flag this weapon
			// as our special EUK type weapon.  This process must happen before Setup() is called on the collectable type.
			//
			// Extra Note: All EUK level 1 weapons will be drawn in the world as normal.
			if( MultiplayerMgr.IsSinglePlayer() ) {
				if( _IsWeapon( pType ) && IsPrimaryWeaponType( pType ) ) {
					if( Player_aPlayer[ 0 ].m_pEntityOrig && ( Player_aPlayer[ 0 ].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
						CBot *pBot = (CBot *) Player_aPlayer[ 0 ].m_pEntityOrig;

						if( !pBot->m_pInventory ) {
							continue;
						}

						CItemInst *pItemInst = pBot->m_pInventory->IsWeaponInInventory( pType->m_pszName );

						// See if we have this weapon
						if( pItemInst ) {
							// If we have it, but we don't have the higher EUK, make the special EUK TRUE
							if( ( pItemInst->m_nUpgradeLevel != 0 ) && ( ( pItemInst->m_nUpgradeLevel - 1 ) < _GetEUKForWeapon( pType ) ) ) {
								pType->m_bSpecialEUK = TRUE;
							}
						} else {
							// Don't have the weapon and the EUK is greater than 0, make it special
							// In this case we MIGHT draw the weapon as a normal weapon in the world, but the
							// special pool needs to be allocated regardless to make sure that we have the meshes
							// just in case.
							if( _GetEUKForWeapon( pType ) > 0 ) {
								pType->m_bSpecialEUK = TRUE;
							}
						}
					}
				}
			}

			if( !pType->Setup() ) {
				DEVPRINTF( "CCollectable::PostPlayerCreationSetup(): Failed to setup pools.\n" );
				goto _ExitWithError;
			}
		}
	}

	// Now walk all players and check to see which weapons we need to create.
	// If the player doesn't have a weapon in their inventory, we need to create it
	for( s32 i = 0; i < CPlayer::m_nPlayerCount; ++i) {
		u32 uNumMissing = 0;

		// Don't create weapons if we aren't Glitch, he is the only one who can pick them up.
		if( Player_aPlayer[i].m_pEntityOrig == NULL || !( Player_aPlayer[i].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
			continue;
		}

		pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

		for( ; pType != NULL; pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType ) ) {
			// If it isn't setup, we don't need it
			if( !pType->m_bSetup ) {
				continue;
			}

			if( IsWeapon( pType->m_eType ) ) {
				// Make sure we have a bot
				if( ( Player_aPlayer[i].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOT ) ) {
					CBot *pBot = (CBot *) Player_aPlayer[i].m_pEntityOrig;

					if( !pBot->m_pInventory ) {
						continue;
					}

					// We don't have this weapon, we better create it
					if( !pBot->m_pInventory->IsWeaponInInventory( pType->m_pszName ) ) {
						CInventory *pInv = pBot->m_pInventory;
						FASSERT( pType->m_uItemRepositoryIndex != ITEM_UNKNOWN);
						CItem *pItem = CItemRepository::RetrieveEntry( pType->m_uItemRepositoryIndex );
						CWeapon *pWeapon = NULL;

						if( pItem == NULL ) {
							DEVPRINTF( "CCollectable::PostPlayerCreationSetup(): Failed to find CItem for '%s'.\n", pType->m_pszName );
							goto _ExitWithError;	
						}
						
						// If we are set to use a special EUK, set it to FALSE since we don't have the weapon and we don't want to be
						// drawing the special mesh in the world.
						//
						// It will be set back to TRUE later. 
						//
						// We can set to FALSE here since if we needed the mesh, it will already be created.
						pType->m_bSpecialEUK = FALSE;

						// Don't create weapons that we don't need
						u32 uPickupNdx;
                        for( uPickupNdx = 0; uPickupNdx < pInv->m_uNumPickupWeapons; ++uPickupNdx ) {
							if( !fclib_stricmp( pInv->m_PickupWeaponInfo[ uPickupNdx ].pItem->m_pszCodeName, pItem->m_pszCodeName ) ) {
								// We have this one already
								break;
							}
						}

						if( uPickupNdx != pInv->m_uNumPickupWeapons ) {
							continue;
						}

						pWeapon = pItem->MakeWeapon();

						if( pWeapon == NULL ) {
							DEVPRINTF( "CCollectable::PostPlayerCreationSetup(): Failed to allocate memory for weapon.\n" );
							goto _ExitWithError;	
						}

						// NKM - Empty the thing
						pWeapon->SetClipAmmo( 0, FALSE );
						pWeapon->SetReserveAmmo( 0, FALSE );

						pWeapon->EnableAutoWork( FALSE );
						pWeapon->RemoveFromWorld( TRUE );
						pWeapon->SetOwner( pBot );

						// Set the bUsed flag to FALSE since it hasn't been used yet
						// Once the weapon is used by the collectable system it will be set to TRUE
						// and will be deleted by the bot.
						FASSERT( pInv->m_uNumPickupWeapons < ItemInst_uMaxInventoryWeapons );
						pInv->m_PickupWeaponInfo[pInv->m_uNumPickupWeapons].pWeapon = pWeapon;
						pInv->m_PickupWeaponInfo[pInv->m_uNumPickupWeapons].pItem = pItem;
						pInv->m_PickupWeaponInfo[pInv->m_uNumPickupWeapons].bUsed = FALSE;

						++pInv->m_uNumPickupWeapons;
					}
				}
			}

			
		}
	}

	// Finally, go through all of the collectable objects that are in the alive list and do the battery check.
	// If we find a battery type and it has its battery number set, check and see if we need to swap it to the super health
	//
	// The rule for swapping to the mega health is as follows:
	//		If the number on the battery is less than or equal to the current number of batteries, draw a mega health
	//		Otherwise, draw the battery
	if( MultiplayerMgr.IsSinglePlayer() ) {
		CCollectable *pCurr = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );

		while( pCurr ) {
			if( pCurr->m_pCollectableType && pCurr->m_pCollectableType->m_eType == COLLECTABLE_BATTERY ) {
				if( Player_aPlayer[ 0 ].m_pEntityOrig && ( Player_aPlayer[ 0 ].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
					CBot *pBot = (CBot *) Player_aPlayer[ 0 ].m_pEntityOrig;

					if( ( pCurr->m_nCurAmmoCount != -1 ) && ( pCurr->m_nCurAmmoCount <= pBot->m_pInventory->m_uNumBatteries ) ) {
						CCollectableType *pType = GetCollectableTypeObjectFromType( COLLECTABLE_MEGA_HEALTH );

						if( pType ) {
							pCurr->_ReturnWorldMesh();

							pCurr->m_pCollectableType = pType;
							pCurr->_Setup( pCurr->MtxToWorld(), 1.0f );
						}
					}
				}
			}

			pCurr = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCurr );
		}
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( Frame );
	return FALSE;
}

#if FANG_WINGC
static s32 _CoopGrantRecipient( const CBot *pBot ) {
	for( s32 n=0; n < CPlayer::m_nPlayerCount; ++n ) {
		if( Player_aPlayer[n].m_pEntityCurrent == pBot || Player_aPlayer[n].m_pEntityOrig == pBot ) return n;
	}
	return -1;
}
#endif

BOOL CCollectable::GiveToPlayer( CBot *pBot, CollectableType_e eType, f32 fScale ) {
	FASSERT( pBot );
	FASSERT( fScale > 0.0f );

	CFMtx43A Mtx;

	Mtx.Identity();
	Mtx.m_vPos = pBot->MtxToWorld()->m_vPos;
	Mtx.m_vPos.Add( CFVec3A::m_UnitAxisY );

	#if FANG_WINGC
	return _PlaceIntoWorld( _FindCollectableType( eType ), &Mtx, NULL, fScale, 0.0f, -1, NULL, _CoopGrantRecipient( pBot ) );
#else
	return PlaceIntoWorld( eType, &Mtx, NULL, fScale );
#endif
}

BOOL CCollectable::GiveToPlayer( CBot *pBot, cchar *pszCollectableName, f32 fScale ) {
	FASSERT( pBot );
	FASSERT( pszCollectableName );
	FASSERT( fScale > 0.0f );

	CFMtx43A Mtx;

	Mtx.Identity();
	Mtx.m_vPos = pBot->MtxToWorld()->m_vPos;
	Mtx.m_vPos.Add( CFVec3A::m_UnitAxisY );

	#if FANG_WINGC
	return _PlaceIntoWorld( _FindCollectableType( pszCollectableName ), &Mtx, NULL, fScale, 0.0f, -1, NULL, _CoopGrantRecipient( pBot ) );
#else
	return PlaceIntoWorld( pszCollectableName, &Mtx, NULL, fScale );
#endif
}

BOOL CCollectable::GiveWeaponToPlayer( CBot *pBot, cchar *pszWeaponName, s32 nAmmoCount ) {
#if defined(MA_PC_INPUT)
	// Direct grants also use HUD/special-pickup helpers. Supply their recipient
	// context instead of inheriting whoever last touched a world collectable.
	if( !pBot || !pszWeaponName ) return FALSE;
	CBot *pPreviousBot = m_pCollectBot;
	CHud2 *pPreviousHud = m_pCollectHud;
	CPlayer *pPreviousPlayer = m_pPlayer;
	const s32 nPlayer = _CoopGrantRecipient( pBot );
	m_pCollectBot = pBot;
	m_pCollectHud = nPlayer >= 0 ? CHud2::GetHudForPlayer( nPlayer ) : NULL;
	m_pPlayer = nPlayer >= 0 ? &Player_aPlayer[nPlayer] : NULL;
	const BOOL bResult = _GiveWeaponToPlayerScoped( pBot, pszWeaponName, nAmmoCount );
	m_pCollectBot = pPreviousBot;
	m_pCollectHud = pPreviousHud;
	m_pPlayer = pPreviousPlayer;
	return bResult;
}

BOOL CCollectable::_GiveWeaponToPlayerScoped( CBot *pBot, cchar *pszWeaponName, s32 nAmmoCount ) {
#endif
	FASSERT( pBot );
	FASSERT( pszWeaponName );

	CCollectableType *pCollect = _FindCollectableType( pszWeaponName );

	if( pCollect == NULL ) {
		DEVPRINTF( "CCollectable::GiveWeaponToPlayer(): Failed to find collectable type '%s'.\n", pszWeaponName );
		return FALSE;
	}

	if( pBot->m_pInventory == NULL ) {
		return FALSE;
	}

	// Get the ammo count. If we are given an ammo count of -1, that means
	// we use the default for this weapon type.
	s32 nAmmo = (nAmmoCount < 0) ? pCollect->m_nAmmoCount : nAmmoCount;
	CInventory *pInv = pBot->m_pInventory;
	CItemInst *pItemInst = NULL;
	u32 uEUK = 0;

	pItemInst = pInv->IsWeaponInInventory( pszWeaponName );

	// We have it already
	if( pItemInst ) {
		BOOL bSpecial = FALSE;

		// Now check to see if we have the EUK for this weapon that is higher than what we have now
		if( IsPrimaryWeaponType( pCollect ) ) {
			if( ( pItemInst->m_nUpgradeLevel != 0 ) && ( ( pItemInst->m_nUpgradeLevel - 1 ) < _GetEUKForWeapon( pCollect ) ) ) {
				_PickupSpecialItem( pCollect );
				bSpecial = TRUE;
			}
		}

		BOOL bRes = _GiveWeaponAmmoToPlayer( pBot, pItemInst, pInv, pszWeaponName, nAmmo );

		if( bRes ) {
			if( !bSpecial ) {
				// Play a tone for pickup of ammo
				CFSoundGroup::PlaySound( pCollect->m_pPickupSound, TRUE );
			}
		} else {
			return FALSE;
		}

		if( pItemInst && pItemInst->m_nUpgradeLevel != 0 ) {
			uEUK = pItemInst->m_nUpgradeLevel - 1;
		}
	} else {
#if defined(MA_PC_INPUT)
		// A direct grant can request a gun whose resources/pickup pool weren't
		// prepared for this level. Reject it before the retail grant indexes the pool.
		CItem *pItem = CItemRepository::RetrieveEntry( pszWeaponName, NULL );
		if( !pItem ) return FALSE;
		const u32 nSide = pItem->IsSecondaryOnly() ? INV_INDEX_SECONDARY : INV_INDEX_PRIMARY;
		if( pInv->m_auNumWeapons[nSide] >= ItemInst_uMaxInventoryWeapons ) return FALSE;
		u32 nPickup;
		for( nPickup=0; nPickup < pInv->m_uNumPickupWeapons; ++nPickup ) {
			const CInventory::WeaponPickupInfo_t &pickup = pInv->m_PickupWeaponInfo[nPickup];
			if( pickup.pItem && pickup.pWeapon && !pickup.bUsed && pickup.pWeapon->IsCreated() &&
				!fclib_stricmp( pItem->m_pszCodeName, pickup.pItem->m_pszCodeName ) ) break;
		}
		if( nPickup == pInv->m_uNumPickupWeapons ) return FALSE;
#endif
		if( IsPrimaryWeaponType( pCollect ) ) {
			_PickupSpecialItem( pCollect );
		} else {
			CFSoundGroup::PlaySound( pCollect->m_pPickupWeaponSound, TRUE );
		}

		_GiveWeaponToPlayer( pBot, pInv, pszWeaponName, nAmmo );	

		if( IsPrimaryWeaponType( pCollect ) ) {
			uEUK = _GetEUKForWeapon( pCollect->m_eType );
		}
	}

	_EUKMeshSwap( pCollect, uEUK );

	return TRUE;
}

#define _ROTATION_RATE		( FMATH_PI * 0.75f )//( FMATH_PI )
#define _TRANSLATION_RATE	( FMATH_2PI * 0.75f )

void CCollectable::Work( void ) {
	m_fRotY += ( _ROTATION_RATE * FLoop_fPreviousLoopSecs );

	if( m_fRotY >= FMATH_2PI ) {
		m_fRotY -= FMATH_2PI;
	}

	m_fTransY += ( _TRANSLATION_RATE * FLoop_fPreviousLoopSecs );

	if( m_fTransY >= FMATH_2PI ) {
		m_fTransY -= FMATH_2PI;
	}

	CCollectableSpawnPoint *pPoint = (CCollectableSpawnPoint *) flinklist_GetHead( &m_CollectableSpawnPoolAlive );
	CCollectableSpawnPoint *pPointNext;

	while( pPoint ) {
		pPointNext = (CCollectableSpawnPoint *) flinklist_GetNext( &m_CollectableSpawnPoolAlive, pPoint );

		pPoint->m_fSpawnTime -= FLoop_fPreviousLoopSecs;

		// Spawn it
		if( pPoint->m_fSpawnTime <= 0.0f ) {
			// _RespawnItem will NOT remove the item from the list, we need to do that here
			_RespawnItem( pPoint );
			flinklist_Remove( &m_CollectableSpawnPoolAlive, pPoint );
			flinklist_AddTail( &m_CollectableSpawnPoolDead, pPoint );
		}

		pPoint = pPointNext;
	}	

	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	while( pType ) {
		// Only update the animation if we have them
		if( pType->m_bSetup && pType->m_pAnimInst ) {
			pType->m_pAnimInst->DeltaTime( FLoop_fPreviousLoopSecs );

			for( u32 i = 0; i < pType->m_uMeshPoolSize; ++i ) {
				pType->m_paAnimCombiners[ i ]->ComputeMtxPalette();
			}	
		}

		pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
	}
}

// If called with COLLECTABLE_UNKNOWN, nothing will be put into the world
BOOL CCollectable::PlaceIntoWorld(	CollectableType_e eType, 
									const CFMtx43A *pMtx, 
									const CFVec3A *pVelWS/* = NULL*/, 
									f32 fScale/* = 1.0f*/, 
									s32 nAmmoCount/* = -1*/, 
									CEntity *pSpawnIgnoreEntity/* = NULL*/ ) {
	FASSERT( pMtx );
	FASSERT( fScale > 0.0f );

 	if( eType == COLLECTABLE_UNKNOWN ) {
		DEVPRINTF( "CCollectable::PlaceIntoWorld(): Trying to place an unknown collectable type into world.\n" );
		return FALSE;
	}

	return _PlaceIntoWorld( _FindCollectableType( eType ), pMtx, pVelWS, fScale, 0.0f, nAmmoCount, pSpawnIgnoreEntity );
}

BOOL CCollectable::PlaceIntoWorld(	cchar *pszCollectableName, 
									const CFMtx43A *pMtx, 
									const CFVec3A *pVelWS/* = NULL*/, 
									f32 fScale/* = 1.0f*/, 
									s32 nAmmoCount/* = -1*/, 
									CEntity *pSpawnIgnoreEntity/* = NULL*/ ) {
	FASSERT( pszCollectableName );
	FASSERT( pMtx );
	FASSERT( fScale > 0.0f );

	CCollectableType *pCollectableType = _FindCollectableType( pszCollectableName );

	if( pCollectableType == NULL ) {
		DEVPRINTF( "CCollectable::PlaceIntoWorld(): Invalid collectable type '%s'.\n", pszCollectableName );
		return FALSE;
	}

	return _PlaceIntoWorld( pCollectableType, pMtx, pVelWS, fScale, 0.0f, nAmmoCount, pSpawnIgnoreEntity );
}

CCollectable::~CCollectable() {
	if(IsSystemInitialized() && IsCreated()) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}

void CCollectable::CheckpointSaveSelect( s32 nCheckpoint ) {
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}

BOOL CCollectable::CheckpointSave( void ) {
	FASSERT( IsCreated() );
	CEntity::CheckpointSave();

	// Save everything that is important since we may be re-used
	CFCheckPoint::SaveData( (u32 &) m_pCollectableType );
	CFCheckPoint::SaveData( IsInWorld() && !IsMarkedForWorldRemove() );
	CFCheckPoint::SaveData( m_fScale );
	CFCheckPoint::SaveData( m_fSpawnTime );
	CFCheckPoint::SaveData( m_nCurAmmoCount );
	CFCheckPoint::SaveData( m_bUsedInScript );
#if FANG_WINGC
	CFCheckPoint::SaveData( m_nCoopCollectedMask );
	CFCheckPoint::SaveData( m_nRecipientPlayer );
#endif

	return TRUE;
}

void CCollectable::CheckpointRestore( void ) {
	FASSERT( IsCreated() );
	BOOL bInWorld = FALSE;

	CEntity::CheckpointRestore();

	CFCheckPoint::LoadData( (u32 &) m_pCollectableType );
 	CFCheckPoint::LoadData( bInWorld );
	CFCheckPoint::LoadData( m_fScale );
	CFCheckPoint::LoadData( m_fSpawnTime );
	CFCheckPoint::LoadData( m_nCurAmmoCount );
	CFCheckPoint::LoadData( m_bUsedInScript );
#if FANG_WINGC
	CFCheckPoint::LoadData( m_nCoopCollectedMask );
	CFCheckPoint::LoadData( m_nRecipientPlayer );
#endif

	if( !m_bInActiveList && bInWorld ) {
		flinklist_Remove( &m_CollectablePoolDead, this );
		flinklist_AddTail( &m_CollectablePoolAlive, this );		
		// NKM - MUST SET THIS TO TRUE, was the cause of the list assert
		m_bInActiveList = TRUE;
	}

	if( m_bInActiveList && !bInWorld ) {
		flinklist_Remove( &m_CollectablePoolAlive, this );
		flinklist_AddTail( &m_CollectablePoolDead, this );
		m_bInActiveList = FALSE;
	}

	_Setup( MtxToWorld(), m_fScale, m_fSpawnTime, m_nCurAmmoCount );

	// Force the collectable to get a mesh from the pool
	// Don't need to return mesh since we restore the mesh pool on CheckpointRestoreGlobal()
	m_pWorldMesh = NULL; 

	m_eState = STATE_PLACED;
}

BOOL CCollectable::Create( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );

	// Get pointer to the leaf class's builder object...
	CCollectableBuilder *pBuilder = (CCollectableBuilder *) GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_CollectableBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_GOODIE );
	}

	// Put the goodies in a weird place so I don't get blamed for (0 0 0) bugs.
	CFMtx43A::m_Temp.Identity();
	CFMtx43A::m_Temp.m_vPos.Set( 13.0f, 666.0f, 69.0f );

	return CEntity::Create( "Goodie", &CFMtx43A::m_Temp, NULL, 0 );
}

void CCollectable::ClassHierarchyDestroy( void ) {
	FASSERT( IsCreated() );
	CEntity::ClassHierarchyDestroy();
}

BOOL CCollectable::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	FResFrame_t hResFrame = fres_GetFrame();

	CCollectableBuilder *pBuilder = (CCollectableBuilder *)(GetLeafClassBuilder());

	if( !CEntity::ClassHierarchyBuild() ) {
		goto _ExitWithError;
	}

	_ClearDataMembers();

	m_pCollectableType = pBuilder->m_pCollectableType;

	if( _CheckRemapItem( pBuilder ) ) {
		// If we got here, we re-mapped to something that shouldn't be in the world
		return TRUE;
	}

//	 Took this out since we now have the special EUK mesh and we don't find out if we need a
//	 special EUK mesh or not until AFTER the player's inventory has been setup.
//	 The collectable type is still setup in PostPlayerCreationSetup() which is always called.
//
//	if( m_pCollectableType && !m_pCollectableType->Setup( ) ) {
//		goto _ExitWithError;
//	}
	m_eState = STATE_PLACED;

	if( !_Setup( MtxToWorld(), pBuilder->m_fScale, pBuilder->m_fSpawnTime, pBuilder->m_nAmmo ) ) {
		DEVPRINTF( "CCollectable::ClassHierarchyBuild(): Failed to setup collectable.\n" );
		goto _ExitWithError;
	}

	if( m_pCollectableType ) {
		// Make sure this is done
		m_pCollectableType->m_bNeedSetup = TRUE;

		if( m_pCollectableType->m_eType == COLLECTABLE_BATTERY ) {
			if( pBuilder->m_uBatteryNumber != -1 ) {
				// We may need to use the super health
				CCollectable::NotifyCollectableUsedInWorld( COLLECTABLE_MEGA_HEALTH );

				// This is a hack, but it saves space and makes my life easier
				m_nCurAmmoCount = pBuilder->m_uBatteryNumber;
			}
		}
	}

	// Make sure to set these up AFTER _Setup()
	//m_fSpawnTime = pBuilder->m_fSpawnTime;
	//m_nCurAmmoCount = pBuilder->m_nAmmo;

	EnableAutoWork( TRUE );

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( hResFrame );
	return FALSE;
}

BOOL CCollectable::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	if( !CEntity::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	return FALSE;
}

CEntityBuilder *CCollectable::GetLeafClassBuilder() {
	return &_CollectableBuilder;
}

void CCollectable::ClassHierarchyAddToWorld( void ) {
	CEntity::ClassHierarchyAddToWorld();

	if( !m_bInActiveList ) { 
		if( m_bUsedInScript ) {
			flinklist_Remove( &m_CollectablePoolDead, this );
			flinklist_AddTail( &m_CollectablePoolAlive, this );	
			m_bInActiveList = TRUE;
		}
	}
}

void CCollectable::ClassHierarchyRemoveFromWorld( void ) {
	if( m_bInActiveList ) {
		m_bInActiveList = FALSE;
		// Remove from active list first since we are a member of that list already
		flinklist_Remove( &m_CollectablePoolAlive, this );
		flinklist_AddTail( &m_CollectablePoolDead, this );
	}

	if( IsCreated() && m_pCollectableType ) {
		_ReturnWorldMesh();
	}

	if( m_hParticleEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hParticleEmitter );
		m_hParticleEmitter = FPARTICLE_INVALID_HANDLE;
	}

	_ClearDataMembers();

	CEntity::ClassHierarchyRemoveFromWorld();
}

void CCollectable::ClassHierarchyRelocated( void *pIdentifier ) {
	if( IsInWorld() && m_pWorldMesh ) {
		m_pWorldMesh->m_Xfm.BuildFromMtx( m_MtxToWorld, m_fScaleToWorld );
		m_pWorldMesh->UpdateTracker();
	}

	CEntity::ClassHierarchyRelocated( pIdentifier );
}

void CCollectable::ClassHierarchyWork( void ) {
	FASSERT( IsCreated() );
//	FASSERT( m_pCollectableType );
	
	if( !m_pCollectableType ) {
		RemoveFromWorld( TRUE );
		return;
	}

	if( m_pVolume ) {
		if( FVis_anVolumeFlags[ m_pVolume->nVolumeID ] & FVIS_VOLUME_IN_ACTIVE_LIST ) {
			_ActiveListGetResources();
		} else {
			_NonActiveListReleaseResources();
			return;
		}
	}

	CEntity::ClassHierarchyWork();

	if( !IsOurWorkBitSet() ) {
		return;
	}

	// So we don't have any lingering values
	m_pPlayer = NULL;
	m_pCollectBot = NULL;
	m_pCollectHud = NULL;

	s32 sPlayerIndex = _CanPickup();

	if( sPlayerIndex > -1 && _PlayerPickupCollectable() ) {
		return;
	}

	if( m_eState == STATE_FLOAT_UP ) {
		f32 fDisY = m_MtxToWorld.m_vPos.y - m_PushPoint.y;

		if( fDisY >= _REST_HEIGHT ) {
			m_MtxToWorld.m_vPos.y += ( _REST_HEIGHT - fDisY );
			m_eState = STATE_STOPPED;
		} else {
			CFVec3A TempMove = CFVec3A::m_UnitAxisY;

			TempMove.Mul( CFVec3A::m_UnitAxisY, _PUSH_VEL * FLoop_fPreviousLoopSecs );
			m_MtxToWorld.m_vPos.Add( TempMove );
		}
	}

	if( m_pWorldMesh ) {
		f32 fScale;
		CFVec3A Center = m_pWorldMesh->m_BoundSphere_MS.m_Pos;
		Center.Mul( -1.0f );

		fScale = m_pCollectableType->m_bSpecialEUK ? 1.0f : m_fScale;
		FASSERT( fScale > 0.0f );

		CFMtx43A::m_XlatRotY.m_vPos.Zero();

		if( !m_pCollectableType->m_bShouldPoster ) {
			CFMtx43A::m_XlatRotY.SetRotationY( m_fRotY + FMATH_DEG2RAD( m_uRotOffset ) );
		} else {
			CFMtx43A::m_XlatRotY.Identity();
		}

		CFMtx43A::m_XlatRotY.MulPoint( Center, Center );
		Center.Mul( fScale );
        CFMtx43A::m_XlatRotY.m_vPos = m_MtxToWorld.m_vPos;
		CFMtx43A::m_XlatRotY.m_vPos.Add( Center );

		if( m_eState == STATE_STOPPED || m_eState == STATE_FLOAT_UP ) {
			CFMtx43A::m_XlatRotY.m_vPos.y += ( 0.65f * fmath_Sin( m_fTransY + FMATH_DEG2RAD( m_uBobOffset ) ) );
		}

		m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_XlatRotY, fScale );
		m_pWorldMesh->UpdateTracker();
	}

	_HandleCollision();

	if( m_eState != STATE_STOPPED ) {
		fworld_GetVolumeContainingPoint( &m_MtxToWorld.m_vPos, &m_pVolume );
	}

//#if SAS_ACTIVE_USER == SAS_USER_NATHAN
//	if( m_pWorldMesh ) {
//		fdraw_DevSphere( &m_MtxToWorld.m_vPos.v3, _MESH_COL_SPHERE_SIZE );///*m_fScale **/ m_pWorldMesh->GetBoundingSphere().m_fRadius );
//	}
//#endif
}

BOOL CCollectable::_LoadCollectableCSV( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( m_CollectableTypeList.nCount == 0 );

	FGameDataFileHandle_t hFile;
	FGameDataWalker_t DataWalker;
	FGameDataTableHandle_t hTable;
	
	FMemFrame_t MemFrame = fmem_GetFrame();

	hFile = fgamedata_LoadFileToFMem( _COLLECTABLE_CSV );
	if( hFile == FGAMEDATA_INVALID_FILE_HANDLE ) {
		DEVPRINTF( "CCollectable::_LoadCollectableCSV(): Could not load collectable CSV file '%s.csv'.\n", _COLLECTABLE_CSV );
		goto _ExitWithError;
	}

	for( hTable = fgamedata_GetFirstTable( hFile, DataWalker ); hTable != FGAMEDATA_INVALID_TABLE_HANDLE; hTable = fgamedata_GetNextTable( DataWalker ) ) {
		_LoadFromGameData( hTable );
	}

	fmem_ReleaseFrame( MemFrame );

	return TRUE;

_ExitWithError:
	fmem_ReleaseFrame( MemFrame );

	return FALSE;
}

void CCollectable::_LoadFromGameData( FGameDataTableHandle_t hTable ) {
	FASSERT( IsSystemInitialized() );

	cchar *pszTableName;
	CollectableUserProp_t UserProp;
	CCollectableType *pType = NULL;

	pszTableName = fgamedata_GetTableName( hTable );

	if( _FindCollectableType( pszTableName ) ) {
		return;
	}

	FResFrame_t ResFrame = fres_GetFrame();

#if FANG_WINGC
	if( !_GetRetailCollectableTableData( hTable, &UserProp ) ) {
#else
	if( !fgamedata_GetTableData( hTable, CCollectableType::m_aGameDataVocab, &UserProp, sizeof( CollectableUserProp_t ) ) ) {
#endif
		DEVPRINTF( "CCollectable::_LoadFromGameData(): Trouble parsing data for '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	pType = fnew CCollectableType;
	
	if( pType == NULL ) {
		DEVPRINTF( "CCollectable::_LoadFromGameData(): Not enough memory for new CCollectableType.\n" );
		goto _ExitWithError;
	}

	pType->_ClearDataMembers();

	pType->m_pszName = CFStringTable::AddString( NULL, pszTableName );
	
	if( pType->m_pszName == NULL ) {
		DEVPRINTF( "CCollectable::_LoadFromGameData(): Not enough memory to store name for goodie type '%s'.\n", pszTableName );
		goto _ExitWithError;
	}

	CItem *pItem = CItemRepository::RetrieveEntry( pType->m_pszName, &pType->m_uItemRepositoryIndex );

	if( !pItem ) {
		pType->m_uItemRepositoryIndex = ITEM_UNKNOWN;
	}

	// Make sure this is set
	pType->m_uMeshPoolSize				= UserProp.uPoolSize;
	pType->m_pMesh						= NULL;
	pType->m_pszMeshName				= UserProp.pszMeshName;
	pType->m_nAmmoCount					= UserProp.nAmmoCount;
	pType->m_eType						= _ClassifyCollectableType( pType->m_pszName );
	pType->m_pPickupSound				= UserProp.pPickupSound;
	pType->m_pPickupWeaponSound			= UserProp.pPickupSoundWeapon;
	pType->m_pRespawnSound				= UserProp.pRespawnSound;
    pType->m_pszParticleName			= UserProp.pszParticleName;
	pType->m_pszParticleRespawnName		= UserProp.pszRespawnParticleName;
	pType->m_pszEUKMeshName				= UserProp.pszEUKMeshName;
	pType->m_pszMeshAnimName			= UserProp.pszMeshAnimName;
	pType->m_fHUDScale					= UserProp.fHUDScale;
	pType->m_fHUDScaleSpecialEUK		= UserProp.fHUDScaleSpecialEUK;

	flinklist_AddTail( &m_CollectableTypeList, pType );

	return;

_ExitWithError:
	fdelete( pType );
	fres_ReleaseFrame( ResFrame );
}

void CCollectable::_LoadFromOverrideGameData( FGameDataTableHandle_t hTable ) {
	FASSERT( IsSystemInitialized() );

	cchar *pszTableName;
	CollectableOverrideUserProp_t UserProp;
	CCollectableType *pType = NULL;

	pszTableName = fgamedata_GetTableName( hTable );

	pType = _FindCollectableType( pszTableName );

	if( !pType ) {
		// This collectable type doesn't exist, so we can't override
		return;
	}

	FASSERT( !pType->m_bSetup );

	if( !fgamedata_GetTableData( hTable, CCollectable::m_aGameDataVocabOverride, &UserProp, sizeof( CollectableOverrideUserProp_t ) ) ) {
		DEVPRINTF( "CCollectable::_LoadFromOverrideGameData(): Trouble parsing data for '%s'.\n", pszTableName );
		return;
	}

	if( UserProp.nAmmoCount > -1 ) {
		pType->m_nAmmoCount = UserProp.nAmmoCount;
	}

	if( UserProp.uPoolSize > 0 ) {
		pType->m_uMeshPoolSize	= UserProp.uPoolSize;
	}

	return;
}

CCollectableType * CCollectable::_FindCollectableType( cchar *pszName ) {
	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );
	const cchar *pszLookName = pszName;

	// NKM - This is a hack since we want "coring charge" but the LDs have "coringcharge" in all the worlds.
	if( !fclib_stricmp( _CORING_CHARGE_LDS, pszName ) ) {
		pszLookName = _CORING_CHARGE_COLLECT;
	}

	while( pType ) {
		if( !stricmp( pszLookName, pType->m_pszName ) ) {
			return pType;
		}

		pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
	}

	return NULL;
}

CCollectableType *CCollectable::_FindCollectableType( CollectableType_e eType ) {
	CCollectableType *pType = (CCollectableType *) flinklist_GetHead( &m_CollectableTypeList );

	while( pType ) {
		if( pType->m_eType == eType ) {
			return pType;
		}

		pType = (CCollectableType *) flinklist_GetNext( &m_CollectableTypeList, pType );
	}

	return NULL;
}

CollectableType_e CCollectable::_ClassifyCollectableType( cchar *pszTypeName ) {
	FASSERT( pszTypeName );
	const cchar *pszLookName = pszTypeName;

	// NKM - This is a hack since we want "coring charge" but the LDs have "coringcharge" in all the worlds.
	if( !fclib_stricmp( _CORING_CHARGE_LDS, pszTypeName ) ) {
		pszLookName = _CORING_CHARGE_COLLECT;
	}

	for( u32 i = 0; i < _uNumCollectableTypes; ++i ) {
		if( !fclib_stricmp( pszLookName, _collectableTypes[i].pszName ) ) {
			return _collectableTypes[i].eType;
		}
	}

	return COLLECTABLE_UNKNOWN;
}

// This should be called when you DON'T have the weapon
void CCollectable::_GiveWeaponToPlayer( CBot *pBot, CInventory *pInv, cchar *pszWeaponName, s32 nAmmoCount ) {
	FASSERT( pBot );
	FASSERT( pInv );

	CItemInst *pItemInst;
	CItem *pItem = CItemRepository::RetrieveEntry( pszWeaponName, NULL );
	u32 uNdx, uWeapNdx;

	FASSERT( pItem );

	uNdx = INV_INDEX_PRIMARY;
	
	// NKM - Two handed weapons are primary
	if( pItem->IsSecondaryOnly() ) {
		uNdx = INV_INDEX_SECONDARY;
	}

	uWeapNdx = pInv->m_auNumWeapons[ uNdx ];
	
	if( uWeapNdx >= ItemInst_uMaxInventoryWeapons ) {
		DEVPRINTF( "CCollectable::_GiveWeaponToPlayer(): Weapon inventory is full!\n" );
		return;
	}
	
	pItemInst = &pBot->m_pInventory->m_aoWeapons[ uNdx ][ uWeapNdx ];

	// Find the weapon in our temp array of weapons
	u32 i;
	for( i = 0; i < pInv->m_uNumPickupWeapons; ++i ) {
		//if( !fclib_stricmp( pszWeaponName, pInv->m_PickupWeaponInfo[i].pItem->m_pszTag ) ) {
		// Check the code name of the item since the tag name won't match.
		if( !fclib_stricmp( pItem->m_pszCodeName, pInv->m_PickupWeaponInfo[i].pItem->m_pszCodeName ) ) {
			break;
		}
	}
	
	// If we hit this, we didn't allocate the temporary weapons
	FASSERT( i < pInv->m_uNumPickupWeapons );
	FASSERT( pInv->m_PickupWeaponInfo[i].pItem != NULL );

	CWeapon *pWeap = pInv->m_PickupWeaponInfo[i].pWeapon;

	// This weapon is now used
	pInv->m_PickupWeaponInfo[i].bUsed = TRUE;

	pWeap->AddToClipThenReserve( nAmmoCount );

	pItemInst->m_pItemData = pInv->m_PickupWeaponInfo[i].pItem;
	pItemInst->m_nClipAmmo = pWeap->GetClipAmmo();
	pItemInst->m_nReserveAmmo = pWeap->GetReserveAmmo();
	pItemInst->m_pOwnerInventory = NULL;
	pItemInst->m_nUpgradeLevel = pItem->m_uCurLevel;
	pItemInst->m_pOwnerInventory = pInv;

	++pInv->m_auNumWeapons[ uNdx ];

	pWeap->SetItemInst( pItemInst );
	pWeap->SetUpgradeLevel( FMATH_MIN( pItem->m_uCurLevel - 1, 0 ) );
	pWeap->EnableAutoWork( FALSE );
	pWeap->RemoveFromWorld( TRUE );
	pWeap->SetOwner( pBot );
//	pWeap->SetItemInst( pItemInst );

	// NKM - Give the upgrade, lets you go from EUK 1 to EUK 2 right away
	if( pItem->m_uCurLevel > ( pWeap->GetUpgradeLevel() + 1 ) ) {
		pWeap->SetUpgradeLevel( pItem->m_uCurLevel - 1 );
	}

	// Special case for Glitch
	if( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		CBotGlitch *pGlitch = (CBotGlitch *) pBot;

		uWeapNdx = pGlitch->m_WeaponInv[ uNdx ].m_nWeaponInvCount;
		pGlitch->m_WeaponInv[ uNdx ].m_apWeapon[uWeapNdx] = pWeap;

		++pGlitch->m_WeaponInv[ uNdx ].m_nWeaponInvCount;

		// Make sure we pass damage to arm
		if( uNdx == 0 ) {
			pGlitch->SetupWeaponDamageToArm( pWeap );
		}
	}

	// Switch to the new weapon we just got
	if( MultiplayerMgr.IsSinglePlayer() ) {
		BOOL bSwitch = TRUE;

		// If we just got a secondary, switch to it only if we don't have another secondary selected
		// or if we are at the empty hand.
		if( uNdx == INV_INDEX_SECONDARY && pInv->m_aoWeapons[ uNdx ][ pInv->m_auCurWeapon[ uNdx ] ].m_pWeapon ) {
			bSwitch = FALSE;
		}

		if( bSwitch ) {
			pInv->SetCurWeapon( uNdx, uWeapNdx, FALSE, TRUE );
		}
	}
}

// Gives ammo to the player or if the player doesn't have the EUK of the item picked up
// upgrades the EUK.
BOOL CCollectable::_GiveWeaponAmmoToPlayer( CBot *pBot, CItemInst *pItemInst, CInventory *pInv, cchar *pszWeaponName, s32 nAmmoCount ) {
	FASSERT( pBot );
	FASSERT( pItemInst );
	FASSERT( pInv );

	if( pBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) {
		CBotGlitch *pGlitch = (CBotGlitch *) pBot;
		CWeapon *pWeap = NULL;
		CItem *pItem = CItemRepository::RetrieveEntry( pszWeaponName, NULL );
		u32 uNdx;

		// Primary or secondary?
		uNdx = INV_INDEX_PRIMARY;

		// NKM - Two handed weapons are primary weapons
		if( pItem->IsSecondaryOnly() ) {
			uNdx = INV_INDEX_SECONDARY;
		}

		// Find the weapon that is already created
		for( u32 i = 0; i < pGlitch->m_WeaponInv[ uNdx ].m_nWeaponInvCount; ++i ) {
			if( pGlitch->m_WeaponInv[ uNdx ].m_apWeapon[ i ] == NULL ) {
				continue;
			}

			if( pGlitch->m_WeaponInv[ uNdx ].m_apWeapon[ i ]->GetItemInst() == pItemInst ) {
				// Found our weapon
				pWeap = pGlitch->m_WeaponInv[ uNdx ].m_apWeapon[ i ];
				break;
			}
		}

		if( pWeap == NULL ) {
			DEVPRINTF( "CCollectable::_GiveWeaponAmmoToPlayer(): Trying to give a bad weapon name.\n" );
			return FALSE;
		}

		BOOL bRetVal = FALSE;

		// !!Nate - EUK Upgrade. Is this correct?
		if( pItem->m_uCurLevel > ( pWeap->GetUpgradeLevel() + 1 ) ) {
			pWeap->SetUpgradeLevel( pItem->m_uCurLevel - 1 );

			// Switch to the new weapon EUK
			if (MultiplayerMgr.IsSinglePlayer()) {
				pInv->SetCurWeapon( uNdx, pWeap->GetItemInst(), FALSE, TRUE );
			}

			bRetVal = TRUE;
		} 

		// If a primary, add ammo to the reserve
		// If a secondary, add ammo to the clip
		if( uNdx == INV_INDEX_PRIMARY ) {
			// Don't pick it up if we don't have any room to put ammo
			if( !pWeap->IsReserveFull() ) {
				pWeap->AddToReserve( nAmmoCount );
				bRetVal = TRUE;
			}			
		} else {
			// Don't pick it up if we don't have any room to put ammo
			if( !pWeap->IsClipFull() ) {
				pWeap->AddToClip( nAmmoCount );
				bRetVal = TRUE;
			}
		}

		if( !bRetVal ) {
			return FALSE;
		}

		pItemInst->NotifyAmmoMightHaveChanged( pWeap->GetClipAmmo(), pWeap->GetReserveAmmo() );
	}

	return TRUE;
}

BOOL CCollectable::_PlaceIntoWorld( CCollectableType *pType, 
									const CFMtx43A *pMtx, 
									const CFVec3A *pVelWS/* = NULL*/, 
									f32 fScale/* = 1.0f*/, 
									f32 fSpawnTime/* = 0.0f*/, 
									s32 nAmmoCount/* = -1*/, 
									CEntity *pSpawnIgnoreEntity/* = NULL*/, s32 nRecipientPlayer/* = -1*/ ) {
	if( pType == NULL ) {
		return FALSE;
	}

	CCollectable *pCollectable = GetCollectable();

	if( pCollectable == NULL ) {
		return FALSE;
	}

	pCollectable->m_pCollectableType = pType;
#if FANG_WINGC
	pCollectable->m_nRecipientPlayer = nRecipientPlayer;
#endif

	if( pCollectable->m_pCollectableType == NULL ) {
		DEVPRINTF( "CCollectable::_PlaceIntoWorld(): Invalid collectable type.\n" );
		goto _ExitWithError;
	}

	// If we don't have a mesh for the collectable type, something has gone wrong and we didn't get told to load this mesh
	if( pCollectable->m_pCollectableType->m_pMeshPool == NULL ) {
		DEVPRINTF( "CCollectable::_PlaceIntoWorld(): Mesh '%s' was not loaded.\n", pCollectable->m_pCollectableType->m_pszMeshName );
		goto _ExitWithError;
	}

	if( pCollectable->_Setup( pMtx, fScale, fSpawnTime, nAmmoCount ) ) {
		//pCollectable->m_nCurAmmoCount = nAmmoCount;
		pCollectable->EnableAutoWork( TRUE );
		pCollectable->AddToWorld();
		pCollectable->Relocate_RotXlatFromUnitMtx_WS( pMtx );
		pCollectable->m_eState = STATE_BOUNCE;
		//pCollectable->m_fSpawnTime = fSpawnTime;
		pCollectable->m_pSpawnIgnoreEntity = pSpawnIgnoreEntity;

		if( pVelWS ) {
			pCollectable->m_VelocityWS = *pVelWS;
		}
	} else {
		ReturnCollectable( pCollectable );
	}

	return TRUE;

_ExitWithError:
	ReturnCollectable( pCollectable );

	return FALSE;
}

BOOL CCollectable::TypeToHUDString( wchar *pDest, u32 uDestWcharCount, CCollectableType *pCollectType, BOOL bAddConcatText/*=FALSE*/, BOOL bSkipLevelText/*=FALSE*/ ) {

	FASSERT( pDest );
	FASSERT( pCollectType );
	FASSERT( uDestWcharCount <= Fang_ConfigDefs.nText_MaxCharsPerPrintf ); 

	CItem *pItem = NULL;
	u32 uEUK = 0;

	if( pCollectType->m_uItemRepositoryIndex != ITEM_UNKNOWN ) {
		pItem = CItemRepository::RetrieveEntry( pCollectType->m_uItemRepositoryIndex );
		uEUK = pItem->m_uCurLevel; // 0,1 for level 1, 2 for level 2, and 3 for level 3
	}

	cwchar *wpszLevel = NULL;

	switch( uEUK ) {
		case 3:
			wpszLevel = Game_apwszPhrases[ GAMEPHRASE_LEVEL_3 ];
		break;
		case 2:
			wpszLevel = Game_apwszPhrases[ GAMEPHRASE_LEVEL_2 ];
		break;
		case 1:
		case 0:
		default:
			wpszLevel = Game_apwszPhrases[ GAMEPHRASE_LEVEL_1 ];
		break;
	}

	switch( pCollectType->m_eType ) {
		case COLLECTABLE_WEAPON_MORTAR_L1:
		case COLLECTABLE_WEAPON_FLAMER_L1:
		case COLLECTABLE_WEAPON_CORING_CHARGE:
		case COLLECTABLE_WEAPON_EMP:
		case COLLECTABLE_WEAPON_MAGMA_BOMB:
		case COLLECTABLE_WEAPON_CLEANER:
		case COLLECTABLE_WEAPON_WRENCH:
		case COLLECTABLE_WEAPON_RECRUITER:
		case COLLECTABLE_WASHER:
		case COLLECTABLE_CHIP:
		case COLLECTABLE_DET_PACK:
		case COLLECTABLE_SECRET_CHIP:
			FASSERT( pItem ); // all defaults should be in the item repository
			_snwprintf( pDest, uDestWcharCount, L"%ls%ls", pItem->m_pwszDisplayName, ( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
		break;
		case COLLECTABLE_ARM_SERVO_L1:
			_snwprintf( pDest, uDestWcharCount, L"%ls %ls%ls", 
				Game_apwszPhrases[ GAMEPHRASE_LEVEL_1 ], Game_apwszPhrases[ GAMEPHRASE_ARM_SERVO ], 
				( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
		break;
		case COLLECTABLE_ARM_SERVO_L2:
			_snwprintf( pDest, uDestWcharCount, L"%ls %ls%ls", 
				Game_apwszPhrases[ GAMEPHRASE_LEVEL_2 ], Game_apwszPhrases[ GAMEPHRASE_ARM_SERVO], 
				( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
		break;
		case COLLECTABLE_ARM_SERVO_L3:
			_snwprintf( pDest, uDestWcharCount, L"%ls %ls%ls", 
				Game_apwszPhrases[ GAMEPHRASE_LEVEL_3 ], Game_apwszPhrases[ GAMEPHRASE_ARM_SERVO ],
				( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
		break;
		case COLLECTABLE_BATTERY:
			_snwprintf( pDest, uDestWcharCount, L"%ls%ls", Game_apwszPhrases[ GAMEPHRASE_BATTERY ], 
				( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
			
		case COLLECTABLE_PUP_ENERGY:
			_snwprintf( pDest, uDestWcharCount, L"%ls",Game_apwszPhrases[ GAMEPHRASE_ENERGY]);
			return TRUE;
		break;

		case COLLECTABLE_MEGA_HEALTH:
			_snwprintf( pDest, uDestWcharCount, L"%ls",Game_apwszPhrases[ GAMEPHRASE_MEGAENERGY]);
			return TRUE;
		break;

		case COLLECTABLE_PUP_HIJUMP:
		case COLLECTABLE_PUP_SPEED:
		case COLLECTABLE_PUP_ARMOR:
		case COLLECTABLE_PUP_WEAPON:
			return FALSE;
		break;
		default:
			FASSERT( pItem ); // all defaults should be in the item repository
			_snwprintf( pDest, uDestWcharCount, L"%ls%ls%ls%ls", 
				( bSkipLevelText ? L"" : wpszLevel ), ( bSkipLevelText ? L"" : L" " ), pItem->m_pwszDisplayName, ( bAddConcatText ? L" %ls ": L"" ) );
			return TRUE;
		break;
	}
}

void CCollectable::_PickupSpecialItem( CCollectableType *pCollectType ) {
	FASSERT( m_pCollectBot );
	FASSERT( pCollectType );

	wchar pwszName[ NUM_WCHARS_TAKE_OFF_THE_STACK ];

	// We got something new, show the HUD message if needed
	BOOL bIsOK = TypeToHUDString( pwszName, NUM_WCHARS_TAKE_OFF_THE_STACK, pCollectType );

	// Didn't have a mapping for this collectable, don't need to do anything
	if( !bIsOK) {
		return;
	}

	CHud2::GetHudForPlayer( m_pCollectBot->m_nPossessionPlayerIndex )->TextDisplay_Start( _HUD_MESSAGE_TIME, NULL, FALSE, TRUE,
		Game_apwszPhrases[ GAMEPHRASE_YOU_GOT_A ], pwszName );

	CFVec3A Pos;
	CFVec3A Up = CFVec3A::m_UnitAxisY;

	Pos.Zero();
	Up.Mul( m_pCollectBot->m_fCollCylinderHeight_WS * 0.75f );
	Pos = m_pCollectBot->m_XlatStickUnitVecXZ_WS;
	Pos.Mul( 1.5f );
	Pos.Add( Up );
	Pos.Add( m_pCollectBot->MtxToWorld()->m_vPos );

	fparticle_SpawnEmitter( m_hSpecialPickup, Pos.v3 );

	CFSoundGroup::PlaySound( m_pSpecialGroup, TRUE );
}

void CCollectable::_ForceSpecialMeshesToNull( CCollectableType *pCollectType ) {
	CCollectable *pCollectable = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );

	pCollectType->m_bSpecialEUK = FALSE;

	while( pCollectable ) {
		if( pCollectable->m_pCollectableType == pCollectType ) {
			pCollectable->_ReturnWorldMesh();
		}

		pCollectable = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCollectable );
	}
}

BOOL CCollectable::_IsWeapon( CCollectableType *pType ) {
	FASSERT( pType );
	return IsWeapon( pType->m_eType );
}

// Assumes that _IsWeapon() has returned TRUE
// This will return TRUE if we are a primary weapon or a scope.  Scope case is needed since it is the
// only secondary weapon that has multiple EUK levels.
BOOL CCollectable::IsPrimaryWeaponType( CCollectableType *pType ) {
	FASSERT( pType );
	FASSERT( _IsWeapon( pType ) );

	switch( pType->m_eType ) {
		case COLLECTABLE_WEAPON_CORING_CHARGE:
		case COLLECTABLE_WEAPON_EMP:
		case COLLECTABLE_WEAPON_MAGMA_BOMB:
		case COLLECTABLE_WEAPON_CLEANER:
		case COLLECTABLE_WEAPON_WRENCH:
		case COLLECTABLE_WEAPON_RECRUITER:
//		case COLLECTABLE_WEAPON_SCOPE_L1:
//		case COLLECTABLE_WEAPON_SCOPE_L2:
			return FALSE;
		break;
	}

	return TRUE;
}

void CCollectable::FixupInventoryWeapons( CInventory *pInv ) {
	FASSERT( pInv );

	for( u32 uWeaponCnt = 0; uWeaponCnt < pInv->m_uNumPickupWeapons; ++uWeaponCnt ) {
		CWeapon *pWeap = pInv->m_PickupWeaponInfo[ uWeaponCnt ].pWeapon;
		BOOL bLook = TRUE;

		for( s8 j = 0; j < pInv->m_auNumWeapons[ INV_INDEX_PRIMARY ]; ++j ) {
			if( pInv->m_aoWeapons[ INV_INDEX_PRIMARY ][ j ].m_pWeapon == pWeap ) {
				pInv->m_PickupWeaponInfo[ uWeaponCnt ].bUsed = TRUE;
				
				bLook = FALSE;
				break;
			}
		}

		if( bLook ) {
			for( s8 j = 0; j < pInv->m_auNumWeapons[ INV_INDEX_SECONDARY ]; ++j ) {
				if( pInv->m_aoWeapons[ INV_INDEX_SECONDARY ][ j ].m_pWeapon == pWeap ) {
					pInv->m_PickupWeaponInfo[ uWeaponCnt ].bUsed = TRUE;
					
					bLook = FALSE;
					break;
				}
			}
		}

		// If we got here, the weapon isn't used at all by the player
		if( bLook ) {
			pInv->m_PickupWeaponInfo[ uWeaponCnt ].bUsed = FALSE;
			pWeap->SetItemInst( NULL );
		}
	}
}

void CCollectable::SetUsedInScript( BOOL bScript ) {
	m_bUsedInScript = bScript;
}

// Returns EUK level for current collectable: 0, 1 or 2 ( Note the 0 based index )
// Assumes that we are a PRIMARY WEAPON or a scope.
u8 CCollectable::_GetEUKForWeapon( CollectableType_e eType ) {
	CollectableType_e eBase;

	switch( eType ) {
		case COLLECTABLE_WEAPON_LASER_L1:
		case COLLECTABLE_WEAPON_LASER_L2:
		case COLLECTABLE_WEAPON_LASER_L3:
			eBase = COLLECTABLE_WEAPON_LASER_L1;
		break;
		case COLLECTABLE_WEAPON_RIPPER_L1:
		case COLLECTABLE_WEAPON_RIPPER_L2:
		case COLLECTABLE_WEAPON_RIPPER_L3:
			eBase = COLLECTABLE_WEAPON_RIPPER_L1;
		break;
		case COLLECTABLE_WEAPON_RIVET_GUN_L1:
		case COLLECTABLE_WEAPON_RIVET_GUN_L2:
		case COLLECTABLE_WEAPON_RIVET_GUN_L3:
			eBase = COLLECTABLE_WEAPON_RIVET_GUN_L1;
		break;
		case COLLECTABLE_WEAPON_ROCKET_L1:
		case COLLECTABLE_WEAPON_ROCKET_L2:
		case COLLECTABLE_WEAPON_ROCKET_L3:
			eBase = COLLECTABLE_WEAPON_ROCKET_L1;
		break;
		case COLLECTABLE_WEAPON_BLASTER_L1:
		case COLLECTABLE_WEAPON_BLASTER_L2:
		case COLLECTABLE_WEAPON_BLASTER_L3:
			eBase = COLLECTABLE_WEAPON_BLASTER_L1;
		break;
		case COLLECTABLE_WEAPON_TETHER_L1:
		case COLLECTABLE_WEAPON_TETHER_L2:
		case COLLECTABLE_WEAPON_TETHER_L3:
			eBase = COLLECTABLE_WEAPON_TETHER_L1;
		break;
		case COLLECTABLE_WEAPON_SPEW_L1:
		case COLLECTABLE_WEAPON_SPEW_L2:
		case COLLECTABLE_WEAPON_SPEW_L3:
			eBase = COLLECTABLE_WEAPON_SPEW_L1;
		break;
		case COLLECTABLE_WEAPON_SCOPE_L1:
		case COLLECTABLE_WEAPON_SCOPE_L2:
			eBase = COLLECTABLE_WEAPON_SCOPE_L1;
		break;
		case COLLECTABLE_WEAPON_MORTAR_L1:
		case COLLECTABLE_WEAPON_FLAMER_L1:
			return 0;
		break;
		default:
			FASSERT_NOW; // Called with a non-primary weapon type
		break;
	}

	return ( eType - eBase );
}

u8 CCollectable::_GetEUKForWeapon( CCollectableType *pType ) {
	FASSERT( pType );
	FASSERT( _IsWeapon( pType ) );

	return _GetEUKForWeapon( pType->m_eType );
}

void CCollectable::_ForceNotifyHigherEUK( CCollectableType *pType ) {
	FASSERT( pType );

	switch( pType->m_eType ) {
		case COLLECTABLE_WEAPON_SPEW_L1:
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_SPEW_L2 );
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_SPEW_L3 );
		break;
		case COLLECTABLE_WEAPON_RIVET_GUN_L1:
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_RIVET_GUN_L2 );
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_RIVET_GUN_L3 );
		break;
		case COLLECTABLE_WEAPON_ROCKET_L1:
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_ROCKET_L2 );
			NotifyCollectableUsedInWorld( COLLECTABLE_WEAPON_ROCKET_L3 );
		break;
		case COLLECTABLE_WEAPON_FLAMER_L1:
			// Nothing else
		break;
		default:
			DEVPRINTF( "CCollectable::_ForceNotifyHigherEUK(): Trying to force a higher EUK of a non-L1 EUK!\n" );
			FASSERT_NOW;
		break;
	}
}

// Given the passed collectable type, we will swap all higher up EUKs to use their special EUK
// mesh if they have one.
void CCollectable::_EUKMeshSwap( CCollectableType *pType, u32 uEUKCurrent ) {
	FASSERT( pType );

	CollectableType_e eType1 = COLLECTABLE_UNKNOWN;
	CollectableType_e eType2 = COLLECTABLE_UNKNOWN;
	CollectableType_e eType3 = COLLECTABLE_UNKNOWN;

	// Not a weapon, no mesh swap needed
	if( !_IsWeapon( pType ) || !IsPrimaryWeaponType( pType ) ) {
		return;
	}

	switch( pType->m_eType ) {
		case COLLECTABLE_WEAPON_LASER_L1:
		case COLLECTABLE_WEAPON_LASER_L2:
		case COLLECTABLE_WEAPON_LASER_L3:
			eType1 = COLLECTABLE_WEAPON_LASER_L1;
			eType2 = COLLECTABLE_WEAPON_LASER_L2;
			eType3 = COLLECTABLE_WEAPON_LASER_L3;
		break;
		case COLLECTABLE_WEAPON_RIPPER_L1:
		case COLLECTABLE_WEAPON_RIPPER_L2:
		case COLLECTABLE_WEAPON_RIPPER_L3:
			eType1 = COLLECTABLE_WEAPON_RIPPER_L1;
			eType2 = COLLECTABLE_WEAPON_RIPPER_L2;
			eType3 = COLLECTABLE_WEAPON_RIPPER_L3;
		break;
		case COLLECTABLE_WEAPON_RIVET_GUN_L1:
		case COLLECTABLE_WEAPON_RIVET_GUN_L2:
		case COLLECTABLE_WEAPON_RIVET_GUN_L3:
			eType1 = COLLECTABLE_WEAPON_RIVET_GUN_L1;
			eType2 = COLLECTABLE_WEAPON_RIVET_GUN_L2;
			eType3 = COLLECTABLE_WEAPON_RIVET_GUN_L3;
		break;
		case COLLECTABLE_WEAPON_ROCKET_L1:
		case COLLECTABLE_WEAPON_ROCKET_L2:
		case COLLECTABLE_WEAPON_ROCKET_L3:
			eType1 = COLLECTABLE_WEAPON_ROCKET_L1;
			eType2 = COLLECTABLE_WEAPON_ROCKET_L2;
			eType3 = COLLECTABLE_WEAPON_ROCKET_L3;
		break;
		case COLLECTABLE_WEAPON_BLASTER_L1:
		case COLLECTABLE_WEAPON_BLASTER_L2:
		case COLLECTABLE_WEAPON_BLASTER_L3:
			eType1 = COLLECTABLE_WEAPON_BLASTER_L1;
			eType2 = COLLECTABLE_WEAPON_BLASTER_L2;
			eType3 = COLLECTABLE_WEAPON_BLASTER_L3;
		break;
		case COLLECTABLE_WEAPON_TETHER_L1:
		case COLLECTABLE_WEAPON_TETHER_L2:
		case COLLECTABLE_WEAPON_TETHER_L3:
			eType1 = COLLECTABLE_WEAPON_TETHER_L1;
			eType2 = COLLECTABLE_WEAPON_TETHER_L2;
			eType3 = COLLECTABLE_WEAPON_TETHER_L3;
		break;
		case COLLECTABLE_WEAPON_SPEW_L1:
		case COLLECTABLE_WEAPON_SPEW_L2:
		case COLLECTABLE_WEAPON_SPEW_L3:
			eType1 = COLLECTABLE_WEAPON_SPEW_L1;
			eType2 = COLLECTABLE_WEAPON_SPEW_L2;
			eType3 = COLLECTABLE_WEAPON_SPEW_L3;
		break;
		case COLLECTABLE_WEAPON_SCOPE_L1:
		case COLLECTABLE_WEAPON_SCOPE_L2:
			eType1 = COLLECTABLE_WEAPON_SCOPE_L1;
			eType2 = COLLECTABLE_WEAPON_SCOPE_L2;
		break;
		case COLLECTABLE_WEAPON_MORTAR_L1:
			eType1 = COLLECTABLE_WEAPON_MORTAR_L1;
		break;
		case COLLECTABLE_WEAPON_FLAMER_L1:
			eType1 = COLLECTABLE_WEAPON_FLAMER_L1;
		break;
		default:
			return;
		break;
	}

	CCollectableType *pChangeType;

	if( eType1 > pType->m_eType ) {
		pChangeType = GetCollectableTypeObjectFromType( eType1 );
		FASSERT( pChangeType );
		
		if( pChangeType->m_pEUKMeshPool && ( _GetEUKForWeapon( eType1 ) > uEUKCurrent ) ) {
			pChangeType->m_bSpecialEUK = TRUE;
		}
	} else {
		eType1 = COLLECTABLE_UNKNOWN;
	}
	
	if( eType2 > pType->m_eType ) {
		pChangeType = GetCollectableTypeObjectFromType( eType2 );
		FASSERT( pChangeType );
		
		if( pChangeType->m_pEUKMeshPool && ( _GetEUKForWeapon( eType2 ) > uEUKCurrent ) ) {
			pChangeType->m_bSpecialEUK = TRUE;
		}
	} else {
		eType2 = COLLECTABLE_UNKNOWN;
	}
	
	if( eType3 > pType->m_eType ) {
		pChangeType = GetCollectableTypeObjectFromType( eType3 );
		FASSERT( pChangeType );
		
		if( pChangeType->m_pEUKMeshPool && ( _GetEUKForWeapon( eType3 ) > uEUKCurrent ) ) {
			pChangeType->m_bSpecialEUK = TRUE;
		}
	} else {
		eType3 = COLLECTABLE_UNKNOWN;
	}

	CCollectable *pCurr = (CCollectable *) flinklist_GetHead( &m_CollectablePoolAlive );

	while( pCurr ) {
		if( pCurr->m_pCollectableType->m_eType == eType1 ||
			pCurr->m_pCollectableType->m_eType == eType2 ||
			pCurr->m_pCollectableType->m_eType == eType3 ) {
				pCurr->_ReturnWorldMesh();
		}

		pCurr = (CCollectable *) flinklist_GetNext( &m_CollectablePoolAlive, pCurr );
	}
}

void CCollectable::_ClearDataMembers( void ) {
#if FANG_WINGC
	m_nCoopCollectedMask = 0;
	m_nRecipientPlayer = -1;
#endif
	m_pCollectableType = NULL;
	m_pWorldMesh = NULL;

	m_LastPos.Zero();
	m_VelocityWS.Zero();
	m_PushPoint.Zero();

	m_eState = STATE_PLACED;
	m_fScale = 1.0f;
	m_pVolume = NULL;
//	m_bInActiveList = FALSE;  // NKM Don't do this here, it is set in GetCollectable()

	m_uRotOffset = 0;
	m_uBobOffset = 0;
	m_hParticleEmitter = FPARTICLE_INVALID_HANDLE;
	
	m_fSpawnTime = 0.0f;
	m_nCurAmmoCount = -1;

	m_pSpawnIgnoreEntity = NULL;
}

BOOL CCollectable::_Setup( const CFMtx43A *pMtx, f32 fScale, f32 fSpawnTime/* = 0.0f*/, s32 nAmmoCount/* = -1*/ ) {
	FASSERT( pMtx );

	m_VelocityWS.Zero();
	m_LastPos = pMtx->m_vPos;
	m_fScale = fScale;
	m_nCurAmmoCount = -1;

	m_fSpawnTime = fSpawnTime;
	m_nCurAmmoCount = nAmmoCount;

	fworld_GetVolumeContainingPoint( &pMtx->m_vPos, &m_pVolume );

	if( m_pCollectableType ) {
		if( IsWeapon( m_pCollectableType->m_eType ) ) {
			switch( m_pCollectableType->m_eType ) {
				// list weapons above who shouldn't have their scale forced up
				case COLLECTABLE_WEAPON_CORING_CHARGE:
					m_fScale = 3.0f;
				break;
				default:
					m_fScale = 2.0f;
				break;
			}

			if( MultiplayerMgr.IsSinglePlayer() ) {
				// If we have a primary weapon and its type is set to have a special EUK, we need
				// to check and make sure we draw the right mesh.  
				//
				// If the player doesn't have the type, but we have the special flag set, we want
				// to unset the flag so that the player sees the weapon mesh in the world and not
				// the EUK mesh.
				if( IsPrimaryWeaponType( m_pCollectableType ) ) {
					if( m_pCollectableType->m_bSpecialEUK ) {
						if( Player_aPlayer[ 0 ].m_pEntityOrig && ( Player_aPlayer[ 0 ].m_pEntityOrig->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
							CBot *pBot = (CBot *) Player_aPlayer[ 0 ].m_pEntityOrig;

							if( pBot->m_pInventory ) {
								CItemInst *pItemInst = pBot->m_pInventory->IsWeaponInInventory( m_pCollectableType->m_pszName );
								BOOL bFlip = FALSE;

								// Do they have it?
								if( !pItemInst ) {
									bFlip = TRUE;
								} else {
									// They have the weapon, let's make sure that lower EUKs show up as weapons
									if( ( pItemInst->m_nUpgradeLevel != 0 ) && ( ( pItemInst->m_nUpgradeLevel - 1 ) >= _GetEUKForWeapon( m_pCollectableType ) ) ) {
										bFlip = TRUE;
									}
								}

								if( bFlip ) {
									if( m_pWorldMesh ) {
										_ReturnWorldMesh();
									}

									m_pCollectableType->m_bSpecialEUK = FALSE;
								}
							}
						}
					}
				}
			}
		}
	}

	m_uRotOffset = (u8) fmath_RandomRange( 0, 255 );
	m_uBobOffset = (u8) fmath_RandomRange( 0, 255 );

	return TRUE;
}

void CCollectable::_ReturnWorldMesh( void ) {
	if( !m_pCollectableType ) {
		return;
	}

	#if FANG_WINGC
	if( m_pWorldMesh ) m_pWorldMesh->SetDrawFilter( NULL, NULL );
	#endif
	m_pCollectableType->ReturnWorldMesh( m_pWorldMesh );
	m_pWorldMesh = NULL;
}

void CCollectable::_HandleCollision( void ) {
	m_LastPos = m_MtxToWorld.m_vPos;

	// NKM - Don't test for world mesh here, we still need to move, just not draw!
	if( m_eState == STATE_FLOAT_UP || m_eState == STATE_STOPPED/* || m_pWorldMesh == NULL*/ ) {
		return;
	}
 
	m_VelocityWS.y += ( _COLLECTABLE_GRAVITY * FLoop_fPreviousLoopSecs );

	CFVec3A MoveFrame;
	CFVec3A NewPos;

	MoveFrame.Mul( m_VelocityWS, FLoop_fPreviousLoopSecs );
	NewPos.Add( m_LastPos, MoveFrame );

	m_CollInfo.pTag = NULL;

#if FANG_WINGC
	// Each moving pickup owns a new collision query. Without this reset, upward
	// loot repeatedly appends its spawning bot's meshes to the shared scratch
	// list, eventually overflowing it (especially for multi-mesh Titans).
	FWorld_nTrackerSkipListCount = 0;
#endif

	if( m_pSpawnIgnoreEntity ) {
		if( m_VelocityWS.y >= 0.0f ) {
			m_pSpawnIgnoreEntity->AppendTrackerSkipList();
		} else {
			m_pSpawnIgnoreEntity = NULL;
		}
	}

	m_CollInfo.ProjSphere.Init( &m_LastPos, &NewPos, _MESH_COL_SPHERE_SIZE );

	fcoll_Clear();
	fworld_CollideWithWorldTris( &m_CollInfo );
	fworld_CollideWithTrackers( &CollProjSphereInfo, m_pWorldMesh );

	if( FColl_nImpactCount > 0 ) {
		FCollImpact_t *pImpact;
		CFVec3A PushVec;
		CFVec3A PrevToCurr;

		PrevToCurr.Sub( NewPos, m_LastPos );
		if( PrevToCurr.MagSq() > 0.001f ) {
			PrevToCurr.Unitize();
		} else {
			PrevToCurr.Zero();
		}

		fcoll_Sort( TRUE );

		pImpact = FColl_apSortedImpactBuf[0];

		PushVec.Set( PrevToCurr );
		PushVec.Mul( pImpact->fImpactDistInfo - 0.09f );
			
		m_MtxToWorld.m_vPos = m_LastPos;
		m_MtxToWorld.m_vPos.Add( PushVec );

		_CollisionResponse( pImpact );
	} else {
		m_MtxToWorld.m_vPos = NewPos;
	}
}

void CCollectable::_CollisionResponse( FCollImpact_t *pNearImpact ) {
	FASSERT( pNearImpact );

	// If we were placed, move up the instant we hit the ground
	if( m_eState == STATE_PLACED ) {
		m_eState = STATE_FLOAT_UP;
		m_VelocityWS.Zero();
		m_PushPoint = pNearImpact->ImpactPoint;
	}

	CFVec3A UnitFaceNormal, AbsNormal;
	CFVec3A VelSurface;
	CFVec3A VelN, VelT; // velocity normal and tangent to plane
	CFVec3A FloorDampening;

	FloorDampening.Set( 0.85f, 0.65f, 0.85f );

	UnitFaceNormal.Set( pNearImpact->UnitFaceNormal );

	AbsNormal.x = FMATH_FABS( UnitFaceNormal.x );
	AbsNormal.y = FMATH_FABS( UnitFaceNormal.y );
	AbsNormal.z = FMATH_FABS( UnitFaceNormal.z );
	
	// Reflect the velocity about the plane's normal
	VelN = UnitFaceNormal;
	VelN.Mul( UnitFaceNormal.Dot( m_VelocityWS ) );
	VelT.Sub( m_VelocityWS, VelN );
	VelSurface.Sub( VelT, VelN );

	// Ground surface
	if( AbsNormal.y >= FMATH_MAX( AbsNormal.x, AbsNormal.z ) && UnitFaceNormal.y > 0.0f ) {
		CFVec3A SubVec;

		SubVec = VelSurface;
		SubVec.Mul( FloorDampening );
		VelSurface.Sub( SubVec );

		FMATH_CLAMP( VelSurface.y, -20.0f, 20.0f );
	} else {
		// Wall surface
		CFVec3A SubVec;
		CFVec3A VelNorm;

		SubVec = VelSurface;
		SubVec.Mul( FloorDampening );
		VelSurface.Sub( SubVec );

		FMATH_CLAMP( VelSurface.y, -20.0f, 20.0f );
	}

	// Don't go too fast
	if( VelSurface.MagSq() > _MAX_SPEED_SQ ) {
		VelSurface.Unitize();
		VelSurface.Mul( _MAX_SPEED );
	}

	m_VelocityWS = VelSurface;

	f32 fMagSq = m_VelocityWS.MagSq();

	if( fMagSq <= 30.0f ) {
		// Only stop completely on a ground surface that points up
		if( AbsNormal.y > FMATH_MAX( AbsNormal.x, AbsNormal.z ) && UnitFaceNormal.y > 0.0f ) {
			f32 fDistanceToPlane = UnitFaceNormal.Dot( m_MtxToWorld.m_vPos ) - UnitFaceNormal.Dot( pNearImpact->ImpactPoint );
			f32 fRadiusScale;
			f32 fAddOn;

			fRadiusScale = _MESH_COL_SPHERE_SIZE;
			fRadiusScale *= 0.85f;

			if( fDistanceToPlane > fRadiusScale ) {
				fAddOn = -( fDistanceToPlane - fRadiusScale );
			} else {
				if( fDistanceToPlane < 0.0f ) {
					fAddOn = -fDistanceToPlane + fRadiusScale;
				} else {
					fAddOn = fRadiusScale;
				}
			}

			// We stopped
			CFVec3A MoveDir = UnitFaceNormal;
			
			MoveDir.Mul( fAddOn );
			m_MtxToWorld.m_vPos.Add( MoveDir );

			m_eState = STATE_FLOAT_UP;
			m_VelocityWS.Zero();
			
			m_PushPoint = pNearImpact->ImpactPoint;
			m_pSpawnIgnoreEntity = NULL;
		} else {
			// If we didn't hit a surface we can stop on and our velocity is low, 
			// boost off of surface so we don't stick to it
			CFVec3A VelAdd;

			VelAdd = UnitFaceNormal;
			VelAdd.Mul( fmath_Sqrt( fMagSq ) );

			m_VelocityWS.Add( VelAdd );
		}
	}
}

BOOL CCollectable::_IsPickupAllowed( void ) {
	BOOL bCanPickup = FALSE;

	if( ( m_pCollectBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) ) {
		bCanPickup = TRUE;

	} else if ( ( m_pCollectBot->TypeBits() & ENTITY_BIT_BOTCHEMBOT ) ) {
		switch( m_pCollectableType->m_eType ) {
			case COLLECTABLE_PUP_ENERGY:
			case COLLECTABLE_MEGA_HEALTH:
			case COLLECTABLE_WEAPON_MAGMA_BOMB:
				bCanPickup = TRUE;
			break;
			default:
				bCanPickup = FALSE;
			break;
		}

	} else if ( ( m_pCollectBot->TypeBits() & ENTITY_BIT_BOTMOZER ) ) {
		switch( m_pCollectableType->m_eType ) {
				case COLLECTABLE_PUP_ENERGY:
				case COLLECTABLE_MEGA_HEALTH:
					bCanPickup = TRUE;
					break;
				default:
					bCanPickup = FALSE;
					break;
		}
	
	} else if ( ( m_pCollectBot->TypeBits() & ENTITY_BIT_KRUNK ) ) {
		switch( m_pCollectableType->m_eType ) {
			case COLLECTABLE_PUP_ENERGY:
			case COLLECTABLE_MEGA_HEALTH:
			case COLLECTABLE_WEAPON_EMP:
				bCanPickup = TRUE;
				break;
			default:
				bCanPickup = FALSE;
				break;
		}
	}

	return bCanPickup;
}

BOOL CCollectable::_PlayerPickupCollectable( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud );
	FASSERT( m_pCollectableType );
	FASSERT( m_pCollectBot->m_pInventory );

	BOOL bClassified = TRUE;
	BOOL bNoSound = FALSE;

	if( !_IsPickupAllowed() ) {
		return FALSE;
	}

	switch( m_pCollectableType->m_eType ) {
		case COLLECTABLE_WASHER:
#if FANG_WINGC
		case COLLECTABLE_MEGA_WASHER:
#endif
			_PickupWasher();
		break;
		case COLLECTABLE_PUP_ENERGY:
			if( !_PickupEnergy() ) {
				return FALSE;
			}
		break;
		case COLLECTABLE_PUP_HIJUMP:
			_PickupHighJump();
		break;
		case COLLECTABLE_PUP_SPEED:
			_PickupSpeed();
		break;
		case COLLECTABLE_PUP_ARMOR:
			_PickupArmor();
		break;
		case COLLECTABLE_PUP_WEAPON:
			_PickupWeaponPower();
		break;
		case COLLECTABLE_BATTERY:
			if( !_PickupBattery() ) {
				return FALSE;
			}

			bNoSound = TRUE;
		break;
		case COLLECTABLE_MEGA_HEALTH:
			if( !_PickupMegaHealth() ) {
				return FALSE;
			}
		break;
		case COLLECTABLE_ARM_SERVO_L1:
		case COLLECTABLE_ARM_SERVO_L2:
		case COLLECTABLE_ARM_SERVO_L3:
			if( !_PickupArmServo( m_pCollectableType->m_eType ) ) {
				return FALSE;
			}

			bClassified = FALSE;
			bNoSound = TRUE;
		break;
		case COLLECTABLE_SECRET_CHIP:
			if( MultiplayerMgr.IsSinglePlayer() ) {
				m_pPlayer->CreditSecretChip();
			}
			bClassified = FALSE;
		break;
		default:
			bClassified = FALSE;
		break;
	}

	// Special case for weapons, they will play their own sounds based on what happens
	if( IsWeapon( m_pCollectableType->m_eType ) ) {
		if( !GiveWeaponToPlayer( m_pCollectBot, m_pCollectableType->m_pszName, m_nCurAmmoCount ) ) {
			return FALSE;
		}

		bClassified = TRUE;
	} else {
		if( !bNoSound ) {
			// Play a tone for pickup
			CFSoundGroup::PlaySound( m_pCollectableType->m_pPickupSound, TRUE );
		}
	}

	// If we got here and bClassified == FALSE, we have a user defined collectable type
	// Stick it into the bot's inventory
	if( !bClassified ) {
		_AddToInventory();
	}

	// If we've got a collection callback, call it
	if( m_pCollectionCallback ) {
		m_pCollectionCallback( m_pCollectableType->m_eType, m_pCollectBot );
	}

#if FANG_WINGC
	BOOL bCollectedByAll = TRUE;
	if( _IsPersonalWeapon() ) {
		const s32 nPlayer = (s32)(m_pPlayer - Player_aPlayer);
		m_nCoopCollectedMask |= 1u << nPlayer;
		bCollectedByAll = m_nCoopCollectedMask == ((1u << CPlayer::m_nPlayerCount) - 1u);
		DEVPRINTF( "Co-op pickup: '%s' player %d, claims 0x%x.\n", m_pCollectableType->m_pszName, nPlayer + 1, m_nCoopCollectedMask );
	}
	// A respawning pickup starts its next cycle only once the team has collected this one.
	if( bCollectedByAll && m_fSpawnTime != 0.0f ) {
#else
	// Spawn one if needed
	if( m_fSpawnTime != 0.0f ) {
#endif
		_AddSpawnPoint();
	}

	// Give back the mesh early on so we are sure to see it in the HUD
	_ReturnWorldMesh();

	if( ( m_pCollectableType->m_eType != COLLECTABLE_WASHER ) &&
#if FANG_WINGC
		( m_pCollectableType->m_eType != COLLECTABLE_MEGA_WASHER ) &&
#endif
		( m_pCollectableType->m_eType != COLLECTABLE_PUP_WEAPON ) ) {
		m_pCollectHud->PickupItemGeneric( m_pCollectableType, ITEMTYPE_POWERUP,
			m_pCollectableType->m_bSpecialEUK ? m_pCollectableType->m_fHUDScaleSpecialEUK : m_pCollectableType->m_fHUDScale );
	}

	// If we pickup something with a special EUK mesh, we need to swap the meshes since at 
	// this point it is no longer considered special, we have the item.  Also, the swap is 
	// done here to ensure that the correct mesh, the EUK mesh, is show on the HUD
	if( m_pCollectableType->m_bSpecialEUK ) {
		_ForceSpecialMeshesToNull( m_pCollectableType );
	}

	// Return the collectable to the pool here after we are done with it
	#if FANG_WINGC
	if( bCollectedByAll )
	#endif
		ReturnCollectable( this );

	return TRUE;
}

void CCollectable::_PickupWasher( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	const s32 nCount =
#if FANG_WINGC
		m_pCollectableType->m_nAmmoCount > 0 ? m_pCollectableType->m_nAmmoCount : 1;
#else
		1;
#endif
	m_pCollectBot->m_pInventory->m_aoItems[INVPOS_WASHER].m_nClipAmmo += nCount;
	for( s32 i = 0; i < nCount; ++i ) {
		m_pPlayer->CreditWasher();
	}

	m_pCollectHud->SetWasherTimed( 2.0f );
}

#define _DEFAULT_ENERGY_HEALTH		( 1.0f )

BOOL CCollectable::_PickupEnergy( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	f32 fPlayerHealth = m_pCollectBot->ComputeUnitHealth();

	// Don't pickup health if we are at 1.0f already.
	if( fPlayerHealth == 1.0f ) {
		return FALSE;
	}

	fPlayerHealth = m_pCollectBot->NormHealth();
    fPlayerHealth += _DEFAULT_ENERGY_HEALTH;
	m_pCollectBot->SetNormHealth( fPlayerHealth );

	return TRUE;
}

#define _DEFAULT_DURATION			( 25.0f )

void CCollectable::_PickupHighJump( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	if( m_pCollectBot->GetPowerupFx() ) {
		m_pCollectBot->GetPowerupFx()->PickupPowerup(
			CBotPowerupFx::BOT_FX_TYPES_HIJUMP, 
			FSNDFX_INVALID_FX_HANDLE/*m_pCollectableType->m_hPickupMusic*/, 
			_DEFAULT_DURATION, 
			0.0f, 
			2.0f, // !!NATE CSV_GoodieProps.fJumpVelMult, 
			1.5f ); // !!NATE CSF_GoodieProps.fGravityMult );
	}
}

void CCollectable::_PickupSpeed( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	if( m_pCollectBot->GetPowerupFx()) {
		m_pCollectBot->GetPowerupFx()->PickupPowerup( 
			CBotPowerupFx::BOT_FX_TYPES_SPEEDUP, 
			FSNDFX_INVALID_FX_HANDLE/*m_pCollectableType->m_hPickupMusic*/,
			_DEFAULT_DURATION, 
			2.0f,//_GoodieProps.fSpeedMult, 
			0.0f, 
			0.0f );
	}
}

void CCollectable::_PickupArmor( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	if( m_pCollectBot->GetPowerupFx() ) {
		m_pCollectBot->GetPowerupFx()->PickupPowerup(
			CBotPowerupFx::BOT_FX_TYPES_ARMOR, 
			FSNDFX_INVALID_FX_HANDLE/*m_pCollectableType->m_hPickupMusic*/,
			_DEFAULT_DURATION, 
			0.0f, 
			0.0f, 
			0.0f );
	}
}

void CCollectable::_PickupWeaponPower( void ) {
	FASSERT( m_pCollectBot && m_pCollectHud && m_pPlayer );

	if( m_pCollectBot->GetPowerupFx() ) {
		m_pCollectBot->GetPowerupFx()->PickupPowerup(
			CBotPowerupFx::BOT_FX_TYPES_WEAPON, 
			FSNDFX_INVALID_FX_HANDLE/*m_pCollectableType->m_hPickupMusic*/,
			_DEFAULT_DURATION,
			0.0f,
			0.0f,
			0.0f );
	}

	_SpawnGlass();
}

BOOL CCollectable::_PickupBattery( void ) {
	CInventory *pInv = m_pCollectBot->m_pInventory;

	if( !pInv ) {
		return FALSE;
	}

	if( pInv->m_uNumBatteries < _MAX_BATTERIES ) {
		++pInv->m_uNumBatteries;
		m_pCollectBot->SetHealthContainerCount( (f32) pInv->m_uNumBatteries );
	}

	if( m_pCollectBot->ComputeUnitHealth() == 1.0f ) {
		return FALSE;
	}

	m_pCollectBot->SetUnitHealth( 1.0f );

// !!Nate - doesn't work
//	CHud2::GetHudForPlayer( m_pCollectBot->m_nPossessionPlayerIndex )->PickupItemBattery();
	_PickupSpecialItem( m_pCollectableType );

	return TRUE;
}

BOOL CCollectable::_PickupMegaHealth( void ) {
	CInventory *pInv = m_pCollectBot->m_pInventory;

	if( !pInv ) {
		return FALSE;
	}

	if( m_pCollectBot->ComputeUnitHealth() == 1.0f ) {
		return FALSE;
	}

	m_pCollectBot->SetUnitHealth( 1.0f );

	return TRUE;
}

BOOL CCollectable::_PickupArmServo( CollectableType_e eType ) {
	FASSERT( eType >= COLLECTABLE_ARM_SERVO_L1 && eType <= COLLECTABLE_ARM_SERVO_L3 );
	FASSERT( ( m_pCollectBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) );
	
	CBotGlitch *pGlitch = (CBotGlitch *) m_pCollectBot;
	u32 uLevel = ( eType - COLLECTABLE_ARM_SERVO_L1 );

	if( uLevel <= pGlitch->GetServoLevel( CBotGlitch::SERVO_TYPE_ARMS ) ) {
		return FALSE;
	}

	pGlitch->SetServoLevel( CBotGlitch::SERVO_TYPE_ARMS, uLevel );

	_PickupSpecialItem( m_pCollectableType );

	return TRUE;
}

void CCollectable::_AddToInventory( void ) {
	CItem *pItem = NULL;
	BOOL bIsArmServo = FALSE;

	switch( m_pCollectableType->m_eType ) {
		case COLLECTABLE_ARM_SERVO_L1:
		case COLLECTABLE_ARM_SERVO_L2:
		case COLLECTABLE_ARM_SERVO_L3:
			// This is done since we have 3 EUKs for arm servos, but only a single arm servo shows up in the item list.
			// There is no need for more than one in the item list.
			pItem = CItemRepository::RetrieveEntry( _COLLECTABLE_ARM_SERVO, NULL );
			bIsArmServo = TRUE;
		break;
		default:
			pItem = CItemRepository::RetrieveEntry( m_pCollectableType->m_uItemRepositoryIndex );
		break;
	}

	// We didn't find it in our inventory csv
	if( pItem == NULL ) {
		DEVPRINTF( "CCollectable::_AddToInventory(): Collectable '%s' was not found in the item repository.\n", m_pCollectableType->m_pszName );
		return;
	}

	if( m_pCollectBot->m_pInventory == NULL ) {
		return;
	}

	CInventory *pInv = m_pCollectBot->m_pInventory;
	CItemInst *pItemInst = NULL;
	
	if( !bIsArmServo ) {
		pItemInst = pInv->IsItemInInventory( m_pCollectableType->m_pszName );
	} else {
		pItemInst = pInv->IsItemInInventory( _COLLECTABLE_ARM_SERVO );
	}

	if( pItemInst == NULL ) {
		if( pInv->m_uNumItems >= ItemInst_uMaxInventoryItems ) {
			DEVPRINTF( "CCollectable::_AddToInventory(): Inventory is full!" );
			return;
		}

		// We don't have it
		pItemInst = &pInv->m_aoItems[ pInv->m_uNumItems ];
		++pInv->m_uNumItems;

		pItemInst->m_pItemData = pItem;
		pItemInst->m_nClipAmmo = 1;
		pItemInst->m_nReserveAmmo = CItemInst_nNoMaxAmmo;
		pItemInst->m_nUpgradeLevel = pItemInst->m_pItemData->m_uCurLevel;
		pItemInst->m_pOwnerInventory = pInv;
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	} else {
		// We have it already
		u16 uAmmo = 0;

		if( bIsArmServo ) {
			uAmmo = ( m_pCollectableType->m_eType - COLLECTABLE_ARM_SERVO_L1 ) + 1;
		} else {
			uAmmo = pItemInst->m_nClipAmmo + 1;
		}

		pItemInst->NotifyAmmoMightHaveChanged( uAmmo, pItemInst->m_nReserveAmmo );
	}
}

// Can only be called during a ClassHierarchyBuild()
// Returns TRUE if we remapped the item to something that we don't want in the world
BOOL CCollectable::_CheckRemapItem( CCollectableBuilder *pBuilder ) {
	FASSERT( pBuilder );

	// DFS -- Don't know why this is happening, but it is...
	// NKM - We get here because we create a pool of entities without collectable types
	if( m_pCollectableType == NULL ) {
		return TRUE;
	}

	// If we don't have a weapon, just leave it
	if( !_IsWeapon( m_pCollectableType ) ) {
		return FALSE;
	}

	BOOL bLimitedCollectableType = (( pBuilder->m_uGameFlags & CCollectableBuilder::GAMETYPE_FLAG_LIMITED_WEAPONS ) != 0);

	// Check primary weapon remapping
	if( IsPrimaryWeaponType( m_pCollectableType ) ) {
		BOOL bLimitPrimary = (MultiplayerMgr.PrimaryWeaponLimit() != GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT);
		BOOL bNoneAllowed = (MultiplayerMgr.PrimaryWeaponLimit() == GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_WEAPONS);

		// Remove this from the world if our restriction modes don't match
		if ( bNoneAllowed || (bLimitPrimary != bLimitedCollectableType) ) {
			m_pCollectableType = NULL;
			return TRUE;
		}
		else if ( bLimitPrimary ) {
			m_pCollectableType = _RemapItem();
		}
	}
	else {
		// Do the same for the secondary weapon
		BOOL bLimitSecondary = (MultiplayerMgr.SecondaryWeaponLimit() != GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT);
		BOOL bNoneAllowed = (MultiplayerMgr.SecondaryWeaponLimit() == GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_WEAPONS);

		// Remove this from the world if our restriction modes don't match
		if ( bNoneAllowed || (bLimitSecondary != bLimitedCollectableType) ) {
			m_pCollectableType = NULL;
			return TRUE;
		}
		else if ( bLimitSecondary ) {
			m_pCollectableType = _RemapItem();
		}
	}
	return FALSE;
}

CCollectableType *CCollectable::_RemapItem( void ) {
	CCollectableType *pRetType = NULL;

	if( IsPrimaryWeaponType( m_pCollectableType ) ) {
		GameSave_PrimaryWeaponLimit_e eLimit = MultiplayerMgr.PrimaryWeaponLimit();
		u8 uEUK = _GetEUKForWeapon( m_pCollectableType );
		CollectableType_e eType;

		FASSERT( uEUK >= 0 && uEUK <= 2 );

		switch( eLimit ) {
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_NO_LIMIT:
				eType = m_pCollectableType->m_eType;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_ROCKETS_ONLY:
				eType = COLLECTABLE_WEAPON_ROCKET_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_LASER_ONLY:
				eType = COLLECTABLE_WEAPON_BLASTER_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_RIVET_ONLY:
				eType = COLLECTABLE_WEAPON_RIVET_GUN_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_TOASTER_ONLY:
				// Only have one flamer EUK
				if( uEUK > 0 ) {
					uEUK = 0;
				}

				eType = COLLECTABLE_WEAPON_FLAMER_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_RIPPER_ONLY:
				eType = COLLECTABLE_WEAPON_RIPPER_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_SPEW_ONLY:
				eType = COLLECTABLE_WEAPON_SPEW_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_SCATTER_BLASTER_ONLY:
				eType = COLLECTABLE_WEAPON_BLASTER_L1;
			break;
			case GAMESAVE_PRIMARY_WEAPON_LIMIT_SLINGSHOT_ONLY:
				// Only have one mortar EUK
				if( uEUK > 0 ) {
					uEUK = 0;
				}
				eType = COLLECTABLE_WEAPON_MORTAR_L1;
			break;
			default:
				DEVPRINTF( "CCollectable::_RemapItem(): Unsupported weapon limit!\n" );
				FASSERT( 0 );
			break;
		}

		pRetType = _FindCollectableType( (CollectableType_e) ( eType + uEUK ) );
	} else {
		GameSave_SecondaryWeaponLimit_e eLimit = MultiplayerMgr.SecondaryWeaponLimit();
		CollectableType_e eType;

		switch( eLimit ) {
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_NO_LIMIT:
				eType = m_pCollectableType->m_eType;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_CORING_CHARGE_ONLY:
				eType = COLLECTABLE_WEAPON_CORING_CHARGE;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_MAGMA_BOMB_ONLY:
				eType = COLLECTABLE_WEAPON_MAGMA_BOMB;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_EMP_ONLY:
				eType = COLLECTABLE_WEAPON_EMP;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_SCOPE_ONLY:
				eType = COLLECTABLE_WEAPON_SCOPE_L1;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_CLEANER_ONLY:
				eType = COLLECTABLE_WEAPON_CLEANER;
			break;
			case GAMESAVE_SECONDARY_WEAPON_LIMIT_RECRUITER_ONLY:
				eType = COLLECTABLE_WEAPON_RECRUITER;
			break;
		}

		pRetType = _FindCollectableType( eType );
	}

	// Override this since we want to use what was defined in the CSV
	m_nCurAmmoCount = -1;

	// If we got here, we MUST have a valid type.  If not, something has gone terribly wrong
	FASSERT( pRetType ); 

	return pRetType;
}

BOOL CCollectable::_AddSpawnPoint( void ) {
	FASSERT( m_pCollectableType );

	if( m_pCollectableType->m_eType == COLLECTABLE_UNKNOWN ) {
		return FALSE;
	}

	CCollectableSpawnPoint *pPoint;

	pPoint = (CCollectableSpawnPoint *) flinklist_RemoveHead( &m_CollectableSpawnPoolDead );

	if( pPoint == NULL ) {
		DEVPRINTF( "CCollectable::_AddSpawnPoint(): Spawn pool is full.\n" );
		return FALSE;
	}

	pPoint->m_fScale = m_fScale;
	pPoint->m_fSpawnTime = m_fSpawnTime;
	pPoint->m_fSpawnTimeTotal = m_fSpawnTime;
	pPoint->m_SpawnPoint = MtxToWorld()->m_vPos;
	pPoint->m_eSpawnType = m_pCollectableType->m_eType;
	pPoint->m_nCurAmmoCount = m_nCurAmmoCount;

	flinklist_AddTail( &m_CollectableSpawnPoolAlive, pPoint );

	return TRUE;
}

void CCollectable::_ActiveListGetResources( void ) {
	if( !m_pWorldMesh ) {
		// Grab a mesh
		m_pWorldMesh = m_pCollectableType->GetWorldMesh();
#if FANG_WINGC
		if( m_pWorldMesh ) m_pWorldMesh->SetDrawFilter( _CoopDrawFilter, this );
#endif

		if( m_pWorldMesh ) {
			m_pWorldMesh->m_Xfm.BuildFromMtx( CFMtx43A::m_IdentityMtx, 1.0f );
			m_pWorldMesh->AddToWorld();
			m_pWorldMesh->UpdateTracker();
		}
	}

	// Get particles if we need them
	if( m_hParticleEmitter == FPARTICLE_INVALID_HANDLE ) {
		if( m_pWorldMesh && m_pCollectableType->m_hParticleDef != FPARTICLE_INVALID_HANDLE ) {
			m_hParticleEmitter = fparticle_SpawnEmitter( m_pCollectableType->m_hParticleDef, &m_pWorldMesh->GetBoundingSphere().m_Pos, &CFVec3A::m_UnitAxisY.v3, NULL, 1.0f );
		}
	}
}

void CCollectable::_NonActiveListReleaseResources( void ) {
	// Give the mesh back
	if( m_pWorldMesh ) {
		_ReturnWorldMesh();
	}

	// Give the particles back
	if( m_hParticleEmitter != FPARTICLE_INVALID_HANDLE ) {
		fparticle_StopEmitter( m_hParticleEmitter );
		m_hParticleEmitter = FPARTICLE_INVALID_HANDLE;
	}
}

void CCollectable::_SpawnGlass( void ) {
	if( !m_pGlassGroup ) {
		return;
	}

	CFDebrisSpawner DebrisSpawner;
	DebrisSpawner.InitToDefaults();
	CFVec3A Forward = m_pCollectBot->MtxToWorld()->m_vFront;

	Forward.Mul( 3.0f );

	DebrisSpawner.m_Mtx.m_vPos = m_MtxToWorld.m_vPos;
	DebrisSpawner.m_Mtx.m_vPos.Add( Forward );
	DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;
	DebrisSpawner.m_nEmitterType = CFDebrisSpawner::EMITTER_TYPE_POINT;
	DebrisSpawner.m_pDebrisGroup = m_pGlassGroup;
	DebrisSpawner.m_fSpawnerAliveSecs = 0.0f;
	DebrisSpawner.m_fMinSpeed = 20.0f;
	DebrisSpawner.m_fMaxSpeed = 30.0f;
	DebrisSpawner.m_fUnitDirSpread = 1.0f;
	DebrisSpawner.m_fScaleMul = 1.0f;
	DebrisSpawner.m_fGravityMul = 1.0f;
	DebrisSpawner.m_fRotSpeedMul = 1.5f;
	DebrisSpawner.m_nMinDebrisCount = 20;
	DebrisSpawner.m_nMaxDebrisCount = 30;
	DebrisSpawner.m_pFcnCallback = NULL;

	CGColl::SpawnDebris( &DebrisSpawner );
}

void CCollectable::_RespawnItem( CCollectableSpawnPoint *pPoint ) {
	CFMtx43A Mtx;

	Mtx.Identity();
	Mtx.m_vPos = pPoint->m_SpawnPoint;

	// Using _FindCollectableType() here is okay since we ensured that we don't have a unknown type at creation
	CCollectableType *pType = _FindCollectableType( pPoint->m_eSpawnType );

	_PlaceIntoWorld( pType, &Mtx, NULL, pPoint->m_fScale, pPoint->m_fSpawnTimeTotal, pPoint->m_nCurAmmoCount );

	CFSoundGroup::PlaySound( pType->m_pRespawnSound, FALSE, &Mtx.m_vPos );

	// !!Nate - right position?
	if( pType->m_hParticleRespawnDef != FPARTICLE_INVALID_HANDLE ) {
		fparticle_SpawnEmitter( pType->m_hParticleRespawnDef, pPoint->m_SpawnPoint.v3 );
	}
}

#if FANG_WINGC
BOOL CCollectable::_IsPersonalWeapon() const {
	return MultiplayerMgr.IsSinglePlayer() && CPlayer::m_nPlayerCount > 1 &&
		m_nRecipientPlayer < 0 && m_pCollectableType && IsWeapon( m_pCollectableType->m_eType );
}
BOOL CCollectable::_CoopDrawFilter( CFMeshInst *pMesh, void *pData ) {
	CCollectable *pPickup = (CCollectable *)pData;
	if( pPickup->m_nRecipientPlayer >= 0 && pPickup->m_nRecipientPlayer != CPlayer::m_nCurrent ) return FALSE;
	return !pPickup->_IsPersonalWeapon() || !(pPickup->m_nCoopCollectedMask & (1u << CPlayer::m_nCurrent));
}
#endif

s32 CCollectable::_CanPickup( void ) {
	if( m_pWorldMesh == NULL ) {
		return -1;
	}

	// NKM  - Added the m_fScale
	f32 fCollRadius = m_pWorldMesh->m_BoundSphere_MS.m_fRadius * _PICKUP_RADIUS_MULTIPLY * m_fScale;
	f32 fBestDistance = FMATH_MAX_FLOAT;
	s32 uBest = -1;

	for( s32 i = 0; i < CPlayer::m_nPlayerCount; ++i ) {
#if FANG_WINGC
		if( m_nRecipientPlayer >= 0 && m_nRecipientPlayer != i ) continue;
		if( _IsPersonalWeapon() && (m_nCoopCollectedMask & (1u << i)) ) continue;
#endif
		CEntity *pPlayer = Player_aPlayer[i].m_pEntityCurrent;
		CBot *pPlayerBot = (CBot *)(pPlayer);

		// DFS -- The player bot must be Glitch right now, because
		// other code casts the CBot to a blink bot. Later this might 
		// need to be fixed so different sorts of bots can be used in multiplayer
		//
		// NKM - Slosh can pickup some items.
		if ( !( pPlayerBot->TypeBits() & ENTITY_BIT_BOTGLITCH ) && 
			 !( pPlayerBot->TypeBits() & ENTITY_BIT_BOTCHEMBOT ) &&
			 !( pPlayerBot->TypeBits() & ENTITY_BIT_BOTMOZER ) &&
			 !( pPlayerBot->TypeBits() & ENTITY_BIT_KRUNK ) ) {
			continue;
		}

#if FANG_WINGC
		if( _IsPersonalWeapon() && (pPlayerBot->TypeBits() & ENTITY_BIT_BOTGLITCH) &&
			!PlayerNeeds( (CBotGlitch *)pPlayerBot, m_pCollectableType->m_eType, FALSE ) &&
			!PlayerNeeds( (CBotGlitch *)pPlayerBot, m_pCollectableType->m_eType, TRUE ) ) continue;
#endif
		// NKM - Added "don't pick up" flag
		if( !pPlayerBot->CanPickupItems() ) {
			continue;
		}

		// DFS - Dying bots shouldn't pick up their own dropped goodies
		if ( pPlayerBot->IsDeadOrDying() ) {
			continue;
		}

		// Don't pick up things when you are in a mech
		if( pPlayerBot->GetCurMech() ) {
			continue;
		}

		f32 fPlayerRadius = pPlayerBot->m_fCollCylinderRadius_WS * 1.5f;
		CFVec3A PosUp = CFVec3A::m_UnitAxisY;

		PosUp.Mul( pPlayerBot->m_fCollCylinderHeight_WS * 0.5f );
		PosUp.Add( pPlayerBot->MtxToWorld()->m_vPos );

		CFVec3A vecPlayerMinusMe = PosUp;
		vecPlayerMinusMe.Sub(m_MtxToWorld.m_vPos);
		f32 fDistToPlayerSq = vecPlayerMinusMe.MagSq();
		f32 fHitRadius = fPlayerRadius + fCollRadius;

		if ((fDistToPlayerSq < (fHitRadius*fHitRadius)) && (fDistToPlayerSq < fBestDistance)) {
			fBestDistance = fDistToPlayerSq;
			uBest = i;
		}	
	}

	if( uBest != -1 ) {
		CPlayer *pPlayer = &Player_aPlayer[uBest];
		
		m_pPlayer = pPlayer;
		m_pCollectBot = (CBot *) pPlayer->m_pEntityCurrent;
		m_pCollectHud = CHud2::GetHudForPlayer( uBest );
	}

	return uBest;
}

BOOL CCollectable::_TrackersCallbackProjSphere( CFWorldTracker *pTracker, FVisVolume_t *pWorldLeafNode ) {
	u32 i;
	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;
	
	if( !pWorldMesh->IsCollisionFlagSet() ) {
		return TRUE;
	}

	for( i = 0; i < FWorld_nTrackerSkipListCount; ++i ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	if( pTracker->m_nUser == MESHTYPES_ENTITY ) {
		if( !( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_SITEWEAPON ) ) {
            if( ((CEntity *)pTracker->m_pUser)->TypeBits() & ENTITY_BIT_BOT ) {
				return TRUE;
			}
		}
	}

	m_CollInfo.pTag = pTracker;
	((CFWorldMesh *)pTracker)->CollideWithMeshTris( &m_CollInfo );

	return TRUE;
}

#if 0
#define _NUM_SPLINE_POINTS ( 16 )

BOOL CCollectable::_RaceToTheRocketSetup( void ) {
	if( MultiplayerMgr.IsMultiplayer() ) {
		return TRUE;
	}

	static const char *pszSplineName = "gimmedemgoods";
	CEntity *pEnt = CEntity::FindInWorld( pszSplineName );
	CESpline *pSpline = (CESpline *) pEnt;

	if( !pSpline || !( pSpline->TypeBits() & ENTITY_BIT_SPLINE ) ) {
		DEVPRINTF( "CCollectable::_RaceToTheRocketSetup(): Unable to find spline with name of '%s'.\n", pszSplineName );
		return FALSE;
	}

	if( pSpline->PointCount() != _NUM_SPLINE_POINTS ) {
		DEVPRINTF( "CCollectable::_RaceToTheRocketSetup(): Spline does not have %d points.  It has %d.\n", _NUM_SPLINE_POINTS, pSpline->PointCount() );
		return FALSE;
	}

	CInventory *pInv = CPlayer::GetInventory( 0 );

	FASSERT( pInv );

	s32 i, nAmmo;
	u32 uRandomNdx;
	u32 uNumPlaced = 0;
	CCollectableType *pType;
	BOOL bUsed[ _NUM_SPLINE_POINTS ];
	CFVec3A Pos;

	fang_MemZero( bUsed, sizeof( bUsed ) );

	for( u32 uInvIndex = 0; uInvIndex < INV_INDEX_COUNT; ++uInvIndex ) {
		for( i = 0; i < pInv->m_auNumWeapons[ uInvIndex ]; ++i ) {
			cchar *pszCodeName = pInv->m_aoWeapons[ uInvIndex ][ i ].m_pItemData->m_pszCodeName;

			if( !fclib_stricmp( "Empty Primary", pszCodeName ) || !fclib_stricmp( "Empty Secondary", pszCodeName ) ) {
				continue;
			}

			uRandomNdx = fmath_RandomRange( 0, _NUM_SPLINE_POINTS - 1 );

			if( bUsed[ uRandomNdx ] ) {
				while( bUsed[ uRandomNdx ] ) {
					uRandomNdx = ( uRandomNdx + 1 ) % _NUM_SPLINE_POINTS;
				}
			}

			nAmmo = pInv->m_aoWeapons[ uInvIndex ][ i ].m_nClipAmmo + pInv->m_aoWeapons[ uInvIndex ][ i ].m_nReserveAmmo;

			pType = CCollectable::GetCollectableTypeObjectFromString( pInv->m_aoWeapons[ uInvIndex ][ i ].m_pItemData->m_pszTag );
			if( !pType ) {
				continue;
			}

			bUsed[ uRandomNdx ] = TRUE;
			Pos = pSpline->PointArray()[ uRandomNdx ];
			CFMtx43A::m_Temp.Identity();
			CFMtx43A::m_Temp.m_vPos = Pos;
			CFMtx43A::m_Temp.m_vPos.Add( CFVec3A::m_UnitAxisY );

			CCollectable::NotifyCollectableUsedInWorld( pType->m_eType );

			if( !pType->Setup() ) {
				return FALSE;
			}

			if( !CCollectable::PlaceIntoWorld( pType->m_eType, &CFMtx43A::m_Temp, NULL, 1.0f, nAmmo ) ) {
				FASSERT_NOW;
			}

			++uNumPlaced;

			FASSERT( uNumPlaced <= _NUM_SPLINE_POINTS );
		}
	}

	u32 uBats = pInv->m_uNumBatteries;

	pInv->SetToUnarmed();
	pInv->m_uNumBatteries = uBats;

	return TRUE;
}
#endif

// Definition of 'need'
// Yes for health, if the player has less than full health
//  No for health, if the player is maxed
// Yes for weapon ammo, if the player has base instance of weapon, and room for the ammo
//  No for weapon ammo, if the player has maximum ammo already, or doesn't own the weapon
// Yes for weapons the player doesn't have
//  No for weapons the player does have
// Yes for others until such time determination is made
BOOL CCollectable::PlayerNeeds( CBotGlitch* pPlayer, CollectableType_e eType, BOOL bRepresentAmmo ) {
	FASSERT( pPlayer );
	FASSERT( pPlayer->m_pInventory );
	FASSERT( pPlayer->TypeBits() & ENTITY_BIT_BOTGLITCH );
	FASSERT( eType < COLLECTABLE_COUNT);

	if( bRepresentAmmo ) { // if it's ammo
		if( IsWeapon( eType ) == FALSE ) { // better be a weapon type, else what the hell?
			DEVPRINTF( "CCollectable::PlayerNeeds() requested ammo for non-weapon type. Bug?\n" );
			return FALSE;
		}

		CItemInst *pItemInst = pPlayer->m_pInventory->IsWeaponInInventory( GetCollectableTypeObjectFromType( eType )->m_pszName );

		// See if we have this weapon
		if( pItemInst ) {
			// If we have it, check for reserve ammo room 
			if( pItemInst->m_pWeapon->IsReserveFull() ) {
				return FALSE;
			}

			return TRUE;
		} else { // failure to have the weapon means player does not need its ammo
			return FALSE;
		}
	}

	FASSERT( !bRepresentAmmo );

	if( eType == COLLECTABLE_PUP_ENERGY ) {
		if( pPlayer->ComputeUnitHealth() < 1.0f ) { // not fully charged
			return TRUE;
		}
		return FALSE;
	} else if( eType == COLLECTABLE_MEGA_HEALTH) {
		if( pPlayer->ComputeUnitHealth() < 1.0f ) { // not fully charged
			return TRUE;
		}
		return FALSE;
	} else if( eType == COLLECTABLE_BATTERY ) {
		if( pPlayer->m_pInventory->m_uNumBatteries < _MAX_BATTERIES ) {
			return TRUE;
		}
		return FALSE;
	} else if (	(eType==COLLECTABLE_ARM_SERVO_L1) ||
				(eType==COLLECTABLE_ARM_SERVO_L2) ||
				(eType==COLLECTABLE_ARM_SERVO_L3)) {
		u32 uLevel = ( eType - COLLECTABLE_ARM_SERVO_L1 );
		FASSERT( (uLevel == 0) || (uLevel == 1) || (uLevel == 2) );
        if( uLevel <= pPlayer->GetServoLevel( CBotGlitch::SERVO_TYPE_ARMS ) )
			return FALSE;
		return TRUE;
	} else if( IsWeapon( eType ) ) {
		CCollectableType *pTypeObject = GetCollectableTypeObjectFromType( eType );
		CItemInst *pItemInst = pPlayer->m_pInventory->IsWeaponInInventory( pTypeObject->m_pszName );

		// See if we have this weapon
		if( pItemInst == NULL) { // don't have it? Sure need it then!
			return TRUE;
		}

		// throwable, check for ammo room,
		if( pItemInst->m_pWeapon->m_pInfo->nInfoFlags & CWeapon::INFOFLAG_THROWABLE ) {
			// If we have it, check for clip ammo room (throwables have no reserve)
			if( pItemInst->m_pWeapon->IsClipFull() ) {
				return FALSE;
			}

			return TRUE;
		}

		// else just check if you've got it or not....
		// If we have it, but we don't have the higher EUK, we therefore need it:
		if( ( pItemInst->m_nUpgradeLevel != 0 ) && ( ( pItemInst->m_nUpgradeLevel - 1 ) < _GetEUKForWeapon( pTypeObject ) ) ) {
			return TRUE;
		} else  {// we've got it already
			return FALSE;
		}
	}

	// if you fall through the cases above, you're an unspecified type, and we always need those
	return TRUE;
}

void CCollectableBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS(CEntityBuilder, ENTITY_BIT_GOODIE, pszEntityType);

	m_pCollectableType = NULL;
	m_fScale = 1.0f;
	m_fSpawnTime = 0.0f;
	m_nAmmo = -1;

	m_uGameFlags = 0;
	m_uBatteryNumber = -1;
}

BOOL CCollectableBuilder::InterpretTable( void ) {
	FGameDataTableHandle_t hCurTable = CEntityParser::m_hTable;

	if( !fclib_stricmp(CEntityParser::m_pszTableName, "goodietype") ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			cchar *pszName = NULL;

			CEntityParser::Interpret_String( &pszName );
			m_pCollectableType = CCollectable::_FindCollectableType( pszName );

			if( m_pCollectableType == NULL ) {
				DEVPRINTF( "CCollectableBuilder::InterpretTable(): Invalid 'goodietype' '%s'.\n", pszName );
				return FALSE;
			} 
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "goodieduration" ) ) {
		// Do nothing. 
		// We need to handle this since it is a leftover from the old system.
		//CEntityParser::Interpret_F32( &m_fDuration );
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "mesh" ) ) {
		// Do nothing. 
		// We need to handle this since it is a leftover from the old system.
		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "scale" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fScale, FMATH_POS_EPSILON, 1000.0f, TRUE );
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "spawntime" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_F32( &m_fSpawnTime, FMATH_POS_EPSILON, 1000.0f, TRUE );
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "ammo" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_S32( &m_nAmmo, -1, 0x7fffffff, TRUE );
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "gametype" ) ) {
		if( CEntityParser::m_nFieldCount < 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			cchar *pszString;

			for( u32 i = 0; i < CEntityParser::m_nFieldCount; ++i ) {
				if( CEntityParser::Interpret_String( &pszString, FALSE, i ) ) {
					if( !fclib_stricmp( pszString, "limited-weapons" ) ) {
						m_uGameFlags |= GAMETYPE_FLAG_LIMITED_WEAPONS;
					} else {
						CEntityParser::Error_InvalidParameterValue();
					}
				}
			}
		}

		return TRUE;
	} else if( !fclib_stricmp( CEntityParser::m_pszTableName, "batterynum" ) ) {
		if( CEntityParser::m_nFieldCount != 1 ) {
			CEntityParser::Error_InvalidParameterCount();
		} else {
			CEntityParser::Interpret_U32( &m_uBatteryNumber, 2, 6, TRUE );
		}

		return TRUE;
	}

	return CEntityBuilder::InterpretTable();
}

