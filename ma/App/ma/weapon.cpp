//////////////////////////////////////////////////////////////////////////////////////
// weapon.cpp - Main weapon module.
//
// Author: Steve Ranck     
//////////////////////////////////////////////////////////////////////////////////////
// THIS CODE IS PROPRIETARY PROPERTY OF SWINGIN' APE STUDIOS, INC.
// Copyright (c) 2002
//
// The contents of this file may not be disclosed to third
// parties, copied or duplicated in any form, in whole or in part,
// without the prior written permission of Swingin' Ape Studios, Inc.
//////////////////////////////////////////////////////////////////////////////////////
// Modification History:
//
// Date     Who         Description
// -------- ----------  --------------------------------------------------------------
// 03/28/02 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon.h"
#if FANG_WINGC
#include "weapon_blaster.h"
#endif
#include "reticle.h"
#include "player.h"
#include "fxfm.h"
#include "fviewport.h"
#include "fworld.h"
#include "fmath.h"
#include "fcoll.h"
#include "fcamera.h"
#include "fworld_coll.h"
#include "FCheckPoint.h"
#include "bot.h"
#include "meshtypes.h"
#include "entity.h"
#include "iteminst.h"
#include "protrack.h"
#include "fsound.h"
#if defined(MA_PC_INPUT)
#include "pc_cheats.h"
#endif


#define _USER_PROP_FILENAME		"weapons.csv"

#define _ENABLE_SHARED_WORLDMESHES	1



//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponBuilder _WeaponBuilder;


void CWeaponBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CEntityBuilder, ENTITY_BIT_WEAPON, pszEntityType );

	// All weapons detonate projectiles by default...
	m_nProjectileReaction = CEntity::PROJECTILE_REACTION_HURT_ME;

	m_nWeaponType = CWeapon::WEAPON_TYPE_COUNT;
	m_pOwnerBot = NULL;
	m_nUpgradeLevel = 0;
	m_nClipAmmo = 1;
	m_nReserveAmmo = 1;
}


BOOL CWeaponBuilder::PostInterpretFixup( void ) {
	return CEntityBuilder::PostInterpretFixup();
}


BOOL CWeaponBuilder::InterpretTable( void ) {
	return CEntityBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeapon
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

CWeapon::_UserProps_t CWeapon::m_UserProps;
CEntity *CWeapon::m_pCallback_ClosestEntity;			// The entity that's closest to our ray start point (NULL=none)
const FViewport_t *CWeapon::m_pCallback_Viewport;		// Viewport
CFSphere CWeapon::m_Callback_TrackerSphere;				// The tracker's sphere
CFVec3A *CWeapon::m_pCallback_RayStartPoint;			// Our ray start point
CFVec3A *CWeapon::m_pCallback_RayEndPoint;				// Our ray end point
f32 CWeapon::m_fCallback_ShortestDistance2;				// Shortest distance so far
CWeapon::Info_t CWeapon::m_aaInfo[WEAPON_TYPE_COUNT][EUK_COUNT_MAX];
CWeapon *CWeapon::m_pCallback_Weapon;
CReticle *CWeapon::m_pCallback_Reticle;
MuzzleFlash_GroupHandle_t CWeapon::m_ahMuzzleFlashGroup[MUZZLEFLASH_TYPE_COUNT];
CEProjPool::PoolHandle_t CWeapon::m_ahProjPool[PROJ_POOL_TYPE_COUNT];


const CWeapon::MuzzleFlashDef_t CWeapon::m_aMuzzleFlashDef[MUZZLEFLASH_TYPE_COUNT] = {
	// MUZZLEFLASH_TYPE_BLASTER:
	MUZZLEFLASH_TYPE_CARD_3D,					// nType
	"TFDMScatr02",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_LASER_YELLOW:
	MUZZLEFLASH_TYPE_CARD_3D,					// nType
	"TFDMLaser04",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_LASER_RED:
	MUZZLEFLASH_TYPE_CARD_3D,					// nType
	"TFDMLaser02",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_FLAME_POSTER_Z_1:
	MUZZLEFLASH_TYPE_CARD_POSTER_Z,				// nType
	"TF_1Muzzle1",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_FLAME_POSTER_Z_2:
	MUZZLEFLASH_TYPE_CARD_POSTER_Z,				// nType
	"TF_1Muzzle2",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_BALL_POSTER_XY_1:
	MUZZLEFLASH_TYPE_CARD_POSTER_XY,			// nType
	"TF_1Muzzle3",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_3D_STAR_1:
	MUZZLEFLASH_TYPE_MESH_3D,					// nType
	"GF_1Muzzle4",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_3D_STAR_2:
	MUZZLEFLASH_TYPE_MESH_3D,					// nType
	"GF_1Muzzle5",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	// MUZZLEFLASH_TYPE_3D_ENERGY_WHITE
	MUZZLEFLASH_TYPE_MESH_3D,					// nType
	"GFMPmuzzle1",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

	//MUZZLEFLASH_TYPE_3D_ENERGY_ORANGE
	MUZZLEFLASH_TYPE_MESH_3D,					// nType
	"GFMPmuzzle2",								// pszTexName
	15,											// nMaxCount
	FDRAW_BLENDOP_ALPHA_TIMES_SRC_PLUS_DST,		// nBlendOp

};


const FGameData_TableEntry_t CWeapon::m_aUserPropVocab[] = {
	// pszRocketMeshName:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// nRocketPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_10000,

	FGAMEDATA_VOCAB_DEBRIS_GROUP,	// pDebrisGroupClipEject


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeapon::m_aUserPropMapTable[] = {
	"Gen",
	m_aUserPropVocab,
	sizeof(m_UserProps),
	(void *)&m_UserProps,

	NULL
};


const u32 CWeapon::m_anMaxUpgradeCount[WEAPON_TYPE_COUNT] = {
	EUK_COUNT_BLINK_RIGHT_HAND,	// WEAPON_TYPE_BLINK_RIGHT_HAND
	EUK_COUNT_LASER,			// WEAPON_TYPE_LASER
	EUK_COUNT_RIVET,			// WEAPON_TYPE_RIVET_GUN
	EUK_COUNT_GRENADE,			// WEAPON_TYPE_GRENADE
	EUK_COUNT_BLASTER,			// WEAPON_TYPE_BLASTER
	EUK_COUNT_ROCKET_LAUNCHER,	// WEAPON_TYPE_ROCKET_LAUNCHER
	EUK_COUNT_TETHER,			// WEAPON_TYPE_TETHER
	EUK_COUNT_SPEW,				// WEAPON_TYPE_SPEW
	EUK_COUNT_MORTAR,			// WEAPON_TYPE_MORTAR
	EUK_COUNT_FLAMER,			// WEAPON_TYPE_FLAMER
	EUK_COUNT_RIPPER,			// WEAPON_TYPE_RIPPER
	EUK_COUNT_CLEANER,			// WEAPON_TYPE_CLEANER
	EUK_COUNT_SCOPE,			// WEAPON_TYPE_SCOPE
	EUK_COUNT_CHAINGUN,			// WEAPON_TYPE_CHAINGUN
	EUK_COUNT_EMP,				// WEAPON_TYPE_EMP
	EUK_COUNT_QUADLASER,		// WEAPON_TYPE_QUADLASER
	EUK_COUNT_MAGMABOMB,		// WEAPON_TYPE_MAGMABOMB
	EUK_COUNT_STAFF,			// WEAPON_TYPE_STAFF
	EUK_COUNT_WRENCH,			// WEAPON_TYPE_WRENCH
	EUK_COUNT_RECRUITER,		// WEAPON_TYPE_RECRUITER
};




BOOL CWeapon::InitSystem( void ) {
	u32 i, j;

	FResFrame_t ResFrame = fres_GetFrame();

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponRocket::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		return FALSE;
	}

	for( j=0; j<EUK_COUNT_MAX; j++ ) {
		for( i=0; i<WEAPON_TYPE_COUNT; i++ ) {
			m_aaInfo[i][j].nGrip = GRIP_COUNT;
			m_aaInfo[i][j].nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
			m_aaInfo[i][j].nStanceType = STANCE_TYPE_STANDARD;
			m_aaInfo[i][j].fMinTargetAssistDist = 0.0f;
			m_aaInfo[i][j].fMaxTargetAssistDist = 0.0f;
			m_aaInfo[i][j].fMaxLiveRange = 0.0f;
			m_aaInfo[i][j].nClipAmmoMax = INFINITE_AMMO;
			m_aaInfo[i][j].nReserveAmmoMax = INFINITE_AMMO;
			m_aaInfo[i][j].nInfoFlags = INFOFLAG_NONE;
			m_aaInfo[i][j].nReticleType = CReticle::TYPE_NONE;
		}
	}

	// Init muzzle flash groups...
	for( i=0; i<MUZZLEFLASH_TYPE_COUNT; ++i ) {
		m_ahMuzzleFlashGroup[i] = CMuzzleFlash::AllocateNewGroup( m_aMuzzleFlashDef[i].nType, m_aMuzzleFlashDef[i].nMaxCount, m_aMuzzleFlashDef[i].pszResName, m_aMuzzleFlashDef[i].nBlendOp );
	}

	// Init projectile pools...
	m_ahProjPool[PROJ_POOL_TYPE_SWARMER_ROCKET] = CEProjPool::Create( CEProj::PROJTYPE_SWARMER, m_UserProps.pszRocketMeshName, m_UserProps.nRocketPoolCount );
	if( m_ahProjPool[PROJ_POOL_TYPE_SWARMER_ROCKET] == EPROJPOOL_NULL_HANDLE ) {
		DEVPRINTF( "CWeapon::InitSystem(): Could not create projectile pool.\n" );
		goto _ExitWithError;
	}

	return TRUE;

_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeapon::UninitSystem( void ) {
}


CWeapon::CWeapon() {
	m_nSingleMeshForEUKs = -1;
}


CWeapon::~CWeapon() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeapon::Create( WeaponType_e nWeaponType, cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponBuilder *pBuilder = (CWeaponBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_WEAPON );
	}

	// Set our builder parameters...
	pBuilder->m_nWeaponType = nWeaponType;
	m_pOwnerBot = NULL;
	m_nUpgradeLevel = 0;
	m_nClipAmmo = 0;
	m_nReserveAmmo = 0;

	// Create an entity...
	if( !CEntity::Create( pszEntityName, pMtx ) ) {
		// Entity could not be created...
		goto _ExitWithError;
	}

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeapon::ClassHierarchyBuild( void ) {
	CWeaponBuilder *pBuilder;

	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Build parent class...
	if( !CEntity::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Get pointer to the leaf class's builder object...
	pBuilder = (CWeaponBuilder *)GetLeafClassBuilder();

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...
	m_nWeaponType = pBuilder->m_nWeaponType;
	m_pOwnerBot = pBuilder->m_pOwnerBot;
	m_nUpgradeLevel = pBuilder->m_nUpgradeLevel;
	if( (u32)m_nWeaponType >= WEAPON_TYPE_COUNT || m_nUpgradeLevel >= GetUpgradeLevelCount() ) {
		DEVPRINTF( "CWeapon::ClassHierarchyBuild(): Unavailable weapon variant %u for type %u.\n", m_nUpgradeLevel, m_nWeaponType );
		goto _ExitWithError;
	}
	m_nClipAmmo = pBuilder->m_nClipAmmo;
	m_nReserveAmmo = pBuilder->m_nReserveAmmo;

	if( m_nReserveAmmo!=INFINITE_AMMO && m_nClipAmmo!=INFINITE_AMMO ) {
		if( ((u32)m_nReserveAmmo + (u32)m_nClipAmmo) >= INFINITE_AMMO ) {
			DEVPRINTF( "CWeapon::ClassHierarchyBuild(): Total ammo must not exceed 65534 rounds.\n" );
			goto _ExitWithError;
		}
	}

	m_pInfo = &m_aaInfo[m_nWeaponType][m_nUpgradeLevel];

	Reticle_EnableDraw( TRUE );

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


CEntityBuilder *CWeapon::GetLeafClassBuilder( void ) {
	return &_WeaponBuilder;
}


void CWeapon::_ClearDataMembers( void ) {
	m_nWeaponType = WEAPON_TYPE_BLINK_RIGHT_HAND;
	m_nWeaponFlags = WEAPONFLAG_NONE;
	m_nBotDamageBoneIndex = -1;
	m_pOwnerBot = NULL;
	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	m_pAttachedScope = NULL;
	m_nUpgradeLevel = 0;
	m_nClipAmmo = 0;
	m_nReserveAmmo = 0;
	m_pItemInst = NULL;

	m_pInfo = &m_aaInfo[m_nWeaponType][m_nUpgradeLevel];

	m_nCurrentState = STATE_PRESENT;
	m_nDesiredState = STATE_PRESENT;

	m_fSecsUntilNextSound = 0.0f;
	m_uAISoundHandle = 0;

	for( u32 i=0; i<EUK_COUNT_MAX; i++ ) {
		m_aSharedResourceData[i].pWorldMesh		= NULL;
		m_aSharedResourceData[i].pAnimCombiner	= NULL;
		m_aSharedResourceData[i].pAnimMeshRest	= NULL;
	}
}


void CWeapon::ClassHierarchyAddToWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( !IsInWorld() );

	CEntity::ClassHierarchyAddToWorld();

	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	ClearPreventReloadFlag();
	ClearPreventSwitchFlag();
	if (IsOwnedByPlayer()){
		_UpdateReticle(GetOwner()->m_nPossessionPlayerIndex);
		// Reticle_EnableDraw( TRUE );
	}
	m_uAISoundHandle = 0;
}


void CWeapon::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CEntity::ClassHierarchyRemoveFromWorld();

	DetachFromParent();

	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	ClearPreventReloadFlag();
	ClearPreventSwitchFlag();
}


// Protected function for enabling/disabling drawing of reticle.
// Note that public function CBot::EnableReticle(FALSE) will disable
// reticle drawing regardless of setting of this function.
void CWeapon::Reticle_EnableDraw( BOOL bEnable ) {
	s32 nPlayerIndex;

	FASSERT( IsCreated() );

	// Set new protected enable state...
	if( bEnable ) {
		// Set protected enable state...
		FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_DISABLE_RETICLE );
	} else {
		// Clear protected enable state...
		FMATH_SETBITMASK( m_nWeaponFlags, WEAPONFLAG_DISABLE_RETICLE );
	}

	if( IsOwnedByPlayer() && (m_nCurrentState==STATE_DEPLOYED || m_nCurrentState==STATE_RELOAD_OPENED) ) {
		nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;
		// This weapon is carried by a player, and weapon
		// is in state that allows reticle drawing...

		if( Player_aPlayer[ nPlayerIndex ].m_Reticle.IsCreated() ) {
			if( Player_aPlayer[ nPlayerIndex ].m_Reticle.IsReticleOverriden() == FALSE ) {
				// Public master draw in weapon object is enabled...

				if( Reticle_IsDrawEnabled() ) {
					// Enable drawing in the reticle object...
					Player_aPlayer[ nPlayerIndex ].m_Reticle.EnableDraw( TRUE );
					ReticleTypeSet();
				} else {
					// Disable drawing in the reticle object...
					Player_aPlayer[ nPlayerIndex ].m_Reticle.EnableDraw( FALSE );
				}
			} else {
				// Public master draw in weapon object is disabled...

				// Disable drawing in the reticle object...
				Player_aPlayer[ nPlayerIndex ].m_Reticle.EnableDraw( FALSE );
			}
		}
	}
}


void CWeapon::SetDesiredState( State_e nDesiredState ) {
	FASSERT( IsCreated() );
	FASSERT( !IsTransitionState( nDesiredState ) );

	if( nDesiredState == m_nCurrentState ) {
		FASSERT( m_nDesiredState == nDesiredState );
		return;
	}

	m_nDesiredState = nDesiredState;
	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	ClearPreventReloadFlag();
	ClearPreventSwitchFlag();

	if( IsTransitionState( m_nCurrentState ) ) {
		// We're currently in a transition between states...
		return;
	}

	// We're currently not in a transition state...

	switch( m_nCurrentState ) {
	case STATE_PRESENT:
		ResetToState( STATE_STOWED );

		if( nDesiredState != m_nCurrentState ) {
			// We have not reached our desired state yet...
			SetDesiredState( nDesiredState );
		}

		return;

	case STATE_STOWED:
		if( m_nDesiredState == STATE_PRESENT ) {
			ResetToState( STATE_PRESENT );

			if( nDesiredState != m_nCurrentState ) {
				// We have not reached our desired state yet...
				SetDesiredState( nDesiredState );
			}

			return;
		} else {
			m_nCurrentState = STATE_DEPLOYING;
		}

		break;

	case STATE_DEPLOYED:
		if( m_nDesiredState <= STATE_STOWED ) {
			m_nCurrentState = STATE_STOWING;

			if( IsOwnedByPlayer() ) {
				if( m_pInfo->nReticleType != CReticle::TYPE_NONE ) {
					Player_aPlayer[ GetOwner()->m_nPossessionPlayerIndex ].m_Reticle.EnableDraw( FALSE );
					ReticleTypeSet();
				}
			}
		} else {
			m_nCurrentState = STATE_OPENING_RELOAD;
		}

		break;

	case STATE_RELOAD_OPENED:
		m_nCurrentState = STATE_CLOSING_RELOAD;
		break;

	}

	BeginStateChange();
}


// Returns the final state based on the given state.
// For example, if STATE_DEPLOYING is provided as the
// parameter then STATE_DEPLOYED will be returned.
//
// If the provided state is a "final" state, then it
// is simply returned.
CWeapon::State_e CWeapon::ComputeDestinationState( State_e nState ) {
	FASSERT( IsValidState(nState) );

	if( !IsTransitionState(nState) ) {
		// Not a transition state...
		return nState;
	} else {
		// Transition state...

		switch( nState ) {
		case STATE_STOWING:
			return STATE_STOWED;

		case STATE_DEPLOYING:
			return STATE_DEPLOYED;

		case STATE_OPENING_RELOAD:
			return STATE_RELOAD_OPENED;

		case STATE_CLOSING_RELOAD:
			return STATE_DEPLOYED;

		case STATE_RELOADING:
			return STATE_RELOAD_OPENED;

		default:
			FASSERT_NOW;
			return STATE_DEPLOYED;
		}
	}
}


void CWeapon::ResetToState( State_e nState, BOOL bForce ) {
	FASSERT( IsCreated() );
	FASSERT( !IsTransitionState(nState) );

	m_nDesiredState = nState;

	if( !bForce && (m_nCurrentState == nState) ) {
		return;
	}

	m_nCurrentState = nState;
	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	ClearPreventReloadFlag();
	ClearPreventSwitchFlag();

	FMATH_CLEARBITMASK( m_nWeaponFlags, WEAPONFLAG_MADE_EMPTY_CLICK_SOUND | WEAPONFLAG_WAIT_FOR_TRIGGER_UP );

	ClassHierarchyResetToState();

	if( IsOwnedByPlayer() )
	{
		if ( (m_nCurrentState == STATE_DEPLOYED) || (m_nCurrentState == STATE_RELOAD_OPENED) ) {
			_UpdateReticle( GetOwner()->m_nPossessionPlayerIndex);
		}

		if ( nState == STATE_STOWED ) {
			Player_aPlayer[ GetOwner()->m_nPossessionPlayerIndex ].ResetTargetAssistance();
		}
	}
}


void CWeapon::StateChangeComplete( void ) {
	BOOL bUpdateReticle = FALSE;

	FASSERT( IsCreated() );
	FASSERT( IsTransitionState( m_nCurrentState ) || m_nCurrentState==STATE_PRESENT );

	switch( m_nCurrentState ) {
	case STATE_STOWING:
		m_nCurrentState = STATE_STOWED;
		break;

	case STATE_DEPLOYING:
		m_nCurrentState = STATE_DEPLOYED;
		bUpdateReticle = TRUE;
		break;

	case STATE_OPENING_RELOAD:
		m_nCurrentState = STATE_RELOAD_OPENED;
		bUpdateReticle = TRUE;
		break;

	case STATE_CLOSING_RELOAD:
		m_nCurrentState = STATE_DEPLOYED;
		break;

	case STATE_RELOADING:
		m_nCurrentState = STATE_RELOAD_OPENED;

		break;

	}

	if( bUpdateReticle && IsOwnedByPlayer()) {
		_UpdateReticle( GetOwner()->m_nPossessionPlayerIndex );
	}

	if( m_nDesiredState != m_nCurrentState ) {
		// Another desired state is waiting for us...
		SetDesiredState( m_nDesiredState );
	}
}


void CWeapon::UpdateReticle( void ) {
	FASSERT( IsCreated() );

	if( IsOwnedByPlayer() ) {
		_UpdateReticle( GetOwner()->m_nPossessionPlayerIndex );
	}
}


void CWeapon::_UpdateReticle( s32 nPlayerIndex ) {
	if( nPlayerIndex >= 0 ) {
		if( m_pAttachedScope == NULL ) {
			if( m_pInfo->nReticleType != CReticle::TYPE_NONE ) {
				CReticle *pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;

				pReticle->SetType( m_pInfo->nReticleType );
				pReticle->EnableDraw( Reticle_IsDrawEnabled() );
				pReticle->ColorScheme_SelectInactive();

				ReticleTypeSet();
			}
		}
	}
}


// Performs one reload. Must be called only when the current state is STATE_RELOAD_OPENED.
// The reload is complete when the current state returns to STATE_RELOAD_OPENED.
void CWeapon::Reload( void ) {
	FASSERT( IsCreated() );
	FASSERT( m_nCurrentState == STATE_RELOAD_OPENED );

	if( !CanClipBeReloaded() ) {
		// Either clip is already full, or there is no
		// more reserve ammo. Just exit.
		return;
	}

	m_nCurrentState = STATE_RELOADING;

	BeginReload();
}


// Called by the class hierarchy to indicate when the reload operation has completed.
void CWeapon::ReloadComplete( void ) {
	FASSERT( IsCreated() );
	FASSERT( m_nCurrentState == STATE_RELOADING );

	m_nCurrentState = STATE_RELOAD_OPENED;

	if( m_nDesiredState != m_nCurrentState ) {
		// Another desired state is waiting for us...
		SetDesiredState( m_nDesiredState );
	}
}


void CWeapon::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	FASSERT( IsCreated() );

	if( m_pItemInst ) {
		// Added this check so that we don't trampel on other weapons using this item inst
		if( m_pItemInst->m_pWeapon == this ) {
			m_pItemInst->m_pWeapon = NULL;
		}
		m_pItemInst = NULL;
	}
	
	m_pItemInst = pItemInst;

	if( m_pItemInst ) {
		m_pItemInst->m_pWeapon = this;
	}

	if( bUpdateItemInstAmmoFromWeaponAmmo ) {
		if( m_pItemInst ) {
			m_pItemInst->NotifyAmmoMightHaveChanged( GetClipAmmo(), GetReserveAmmo() );
		}
	}
}


u32 CWeapon::GetUpgradeLevelCount( void ) const {
#if FANG_WINGC
	if( m_nWeaponType == WEAPON_TYPE_BLASTER ) return CWeaponBlaster::GetAvailableLevelCount();
#endif
	return (u32)m_nWeaponType < WEAPON_TYPE_COUNT ? m_anMaxUpgradeCount[m_nWeaponType] : 0;
}

void CWeapon::SetUpgradeLevel( u32 nUpgradeLevel ) {
	FASSERT( IsCreated() );
	FASSERT( nUpgradeLevel < GetUpgradeLevelCount() );
	if( nUpgradeLevel >= GetUpgradeLevelCount() ) return;

	if( nUpgradeLevel == GetUpgradeLevel() ) {
		return;
	}

	u32 nPreviousUpgradeLevel = GetUpgradeLevel();
	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;
	ClearPreventReloadFlag();
	ClearPreventSwitchFlag();

	// Snap out of any transition state...
	ResetToState( ComputeDestinationState( CurrentState() ), FALSE );

	// Set new upgrade level parameters...
	m_nUpgradeLevel = nUpgradeLevel;
	m_pInfo = &m_aaInfo[m_nWeaponType][nUpgradeLevel];
	AdjustAmmoToFitMax( FALSE );

	// Let the class hierarchy do its work...
	ClassHierarchySetUpgradeLevel( nPreviousUpgradeLevel );

	// Tell new EUK level weapon of the new state...
	ResetToState( ComputeDestinationState( CurrentState() ), TRUE );

	// Tell item inst of weapon upgrade...
	if( m_pItemInst ) {
		m_pItemInst->NotifyWeaponUpgraded( GetUpgradeLevel() );
	}

	// Tell owner bot of weapon upgrade...
	if( m_pOwnerBot ) {
		m_pOwnerBot->NotifyWeaponUpgraded( this );
	}
}


// Returns FALSE if the weapon was already at its maximum upgrade level.
BOOL CWeapon::UpgradeWeapon( void ) {
	FASSERT( IsCreated() );

	u32 nNewLevel = GetUpgradeLevel() + 1;

	if( nNewLevel >= GetUpgradeLevelCount() ) {
		// Already at maximum upgrade level...
		return FALSE;
	}

	// Call the class hierarchy to set the level...
	SetUpgradeLevel( nNewLevel );

	// Level upgrade successful...

	return TRUE;
}


void CWeapon::_AmmoMayHaveChanged( BOOL bNotify ) {
	if( bNotify ) {
		NotifyAmmoMightHaveChanged();
	}

	// Tell item inst of ammo change...
	if( m_pItemInst ) {
		m_pItemInst->NotifyAmmoMightHaveChanged( GetClipAmmo(), GetReserveAmmo() );
	}

	// Tell owner bot of ammo change...
	if( m_pOwnerBot ) {
		m_pOwnerBot->NotifyAmmoMightHaveChanged( this );
	}
}


void CWeapon::AdjustAmmoToFitMax( BOOL bNotify ) {
	// Adjust reserve ammo to fit its max value...
	FMATH_CLAMPMAX( m_nReserveAmmo, GetMaxReserveAmmo() );

	// Adjust clip ammo to fit its max value...
	if( GetMaxClipAmmo() != INFINITE_AMMO ) {
		// Finite clip ammo ceiling...

		if( m_nClipAmmo != INFINITE_AMMO ) {
			// Finite clip ammo...

			if( m_nClipAmmo > GetMaxClipAmmo() ) {
				// Current clip ammo can't fit into clip...

				TransferFromClipToReserve( m_nClipAmmo - GetMaxClipAmmo(), FALSE );

				if( m_nClipAmmo > GetMaxClipAmmo() ) {
					SetClipAmmo( GetMaxClipAmmo(), FALSE );
				}
			}
		} else {
			// Infinite clip ammo...
			m_nClipAmmo = GetMaxClipAmmo();
		}
	}

	FASSERT( m_nClipAmmo <= GetMaxClipAmmo() );
	FASSERT( m_nReserveAmmo <= GetMaxReserveAmmo() );

	_AmmoMayHaveChanged( bNotify );
}


u16 CWeapon::AddToClipThenReserve( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	if( nRounds == INFINITE_AMMO ) {
		AddToClip( INFINITE_AMMO, bNotify );

		if( !(m_pInfo->nInfoFlags & INFOFLAG_THROWABLE) ) {
			AddToReserve( INFINITE_AMMO, bNotify );
		}

		return INFINITE_AMMO;
	}

	u16 nAddedToClipCount, nAddedToReserveCount;

	nAddedToReserveCount = 0;
	nAddedToClipCount = AddToClip( nRounds, bNotify );

	FASSERT( nAddedToClipCount <= nRounds );
	nRounds -= nAddedToClipCount;

	if( nRounds && !(m_pInfo->nInfoFlags & INFOFLAG_THROWABLE) ) {
		nAddedToReserveCount = AddToReserve( nRounds, bNotify );
		FASSERT( nAddedToReserveCount <= nRounds );
	}

	return nAddedToClipCount + nAddedToReserveCount;
}



u16 CWeapon::ComputeTotalAmmo( void ) const {
	FASSERT( IsCreated() );

	if( m_nClipAmmo!=INFINITE_AMMO && m_nReserveAmmo!=INFINITE_AMMO ) {
		FASSERT( ((u32)m_nClipAmmo + (u32)m_nReserveAmmo) < INFINITE_AMMO );
		return m_nClipAmmo + m_nReserveAmmo;
	}

	return INFINITE_AMMO;
}


// Set nRounds to INFINITE_AMMO to indicate infinite clip ammo.
// Returns the new clip ammo count.
u16 CWeapon::SetClipAmmo( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	m_nClipAmmo = nRounds;
	FMATH_CLAMPMAX( m_nClipAmmo, GetMaxClipAmmo() );

	_AmmoMayHaveChanged( bNotify );

	return m_nClipAmmo;
}


// Set nRounds to INFINITE_AMMO to indicate infinite reserve ammo.
// Returns the new reserve ammo count.
u16 CWeapon::SetReserveAmmo( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	m_nReserveAmmo = nRounds;
	FMATH_CLAMPMAX( m_nReserveAmmo, GetMaxReserveAmmo() );

	_AmmoMayHaveChanged( bNotify );

	return m_nReserveAmmo;
}


// Set nRounds to INFINITE_AMMO to indicate infinite max clip ammo.
void CWeapon::OverrideMaxClipAmmo( u16 nMaxRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	((Info_t *)m_pInfo)->nClipAmmoMax = nMaxRounds;
	AdjustAmmoToFitMax( bNotify );
}


// Set nRounds to INFINITE_AMMO to indicate infinite max reserve ammo.
void CWeapon::OverrideMaxReserveAmmo( u16 nMaxRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	((Info_t *)m_pInfo)->nReserveAmmoMax = nMaxRounds;
	AdjustAmmoToFitMax( bNotify );
}


// Set nRounds to INFINITE_AMMO to add as much ammo as possible.
// Returns the number of rounds actually added.
u16 CWeapon::AddToClip( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	if( m_nClipAmmo == INFINITE_AMMO ) {
		// Clip has infinite ammo...

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	// Clip has finite ammo...

	u16 nPrevClipAmmo = m_nClipAmmo;

	if( nRounds == INFINITE_AMMO ) {
		// Trying to add infinite ammo...

		m_nClipAmmo = GetMaxClipAmmo();

		if( GetMaxClipAmmo() == INFINITE_AMMO ) {
			// Clip can hold infinite ammo...

			_AmmoMayHaveChanged( bNotify );

			return nRounds;
		} else {
			// Clip holds finite ammo...

			_AmmoMayHaveChanged( bNotify );

			return m_nClipAmmo - nPrevClipAmmo;
		}
	}

	// Adding finite ammo...

	FASSERT( ((u32)m_nClipAmmo + (u32)nRounds) < INFINITE_AMMO );
	m_nClipAmmo += nRounds;
	FMATH_CLAMPMAX( m_nClipAmmo, GetMaxClipAmmo() );

	_AmmoMayHaveChanged( bNotify );

	return m_nClipAmmo - nPrevClipAmmo;
}


// Set nRounds to INFINITE_AMMO to remove as much ammo as possible.
// Returns the number of rounds actually removed.
u16 CWeapon::RemoveFromClip( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

#if defined(MA_PC_INPUT)
	if( nRounds != INFINITE_AMMO && pccheats_InfiniteAmmo( this ) ) {
		_AmmoMayHaveChanged( bNotify );
		return nRounds;
	}
#endif

	if( m_nClipAmmo == INFINITE_AMMO ) {
		// Clip has infinite ammo...

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	// Clip has finite ammo...

	if( nRounds <= m_nClipAmmo ) {
		m_nClipAmmo -= nRounds;

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	u16 nPrevAmmo = m_nClipAmmo;
	m_nClipAmmo = 0;

	_AmmoMayHaveChanged( bNotify );

	return nPrevAmmo;
}


// Set nRounds to INFINITE_AMMO to add as much ammo as possible.
// Returns the number of rounds actually added.
u16 CWeapon::AddToReserve( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	if( m_nReserveAmmo == INFINITE_AMMO ) {
		// Reserve has infinite ammo...

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	// Reserve has finite ammo...

	u16 nPrevReserveAmmo = m_nReserveAmmo;

	if( nRounds == INFINITE_AMMO ) {
		// Trying to add infinite ammo...

		m_nReserveAmmo = GetMaxReserveAmmo();

		if( GetMaxReserveAmmo() == INFINITE_AMMO ) {
			// Reserve can hold infinite ammo...

			_AmmoMayHaveChanged( bNotify );

			return nRounds;
		} else {
			// Reserve holds finite ammo...

			_AmmoMayHaveChanged( bNotify );

			return m_nReserveAmmo - nPrevReserveAmmo;
		}
	}

	// Adding finite ammo...

	FASSERT( ((u32)m_nReserveAmmo + (u32)nRounds) < INFINITE_AMMO );
	m_nReserveAmmo += nRounds;
	FMATH_CLAMPMAX( m_nReserveAmmo, GetMaxReserveAmmo() );

	_AmmoMayHaveChanged( bNotify );

	return m_nReserveAmmo - nPrevReserveAmmo;
}


// Set nRounds to INFINITE_AMMO to remove as much ammo as possible.
// Returns the number of rounds actually removed.
u16 CWeapon::RemoveFromReserve( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

#if defined(MA_PC_INPUT)
	if( nRounds != INFINITE_AMMO && pccheats_InfiniteAmmo( this ) ) {
		_AmmoMayHaveChanged( bNotify );
		return nRounds;
	}
#endif

	if( m_nReserveAmmo == INFINITE_AMMO ) {
		// Reserve has infinite ammo...

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	// Reserve has finite ammo...

	if( nRounds <= m_nReserveAmmo ) {
		m_nReserveAmmo -= nRounds;

		_AmmoMayHaveChanged( bNotify );

		return nRounds;
	}

	u16 nPrevAmmo = m_nReserveAmmo;
	m_nReserveAmmo = 0;

	_AmmoMayHaveChanged( bNotify );

	return nPrevAmmo;
}


// Set nRounds to INFINITE_AMMO to transfer as much ammo as possible.
u16 CWeapon::TransferFromReserveToClip( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	u16 nAmmoCount = FMATH_MIN( nRounds, m_nReserveAmmo );
	u16 nReturnValue = RemoveFromReserve( AddToClip( nAmmoCount, FALSE ), FALSE );

	_AmmoMayHaveChanged( bNotify );

	return nReturnValue;
}


// Set nRounds to INFINITE_AMMO to transfer as much ammo as possible.
u16 CWeapon::TransferFromClipToReserve( u16 nRounds, BOOL bNotify ) {
	FASSERT( IsCreated() );

	u16 nAmmoCount = FMATH_MIN( nRounds, m_nClipAmmo );
	u16 nReturnValue = RemoveFromClip( AddToReserve( nAmmoCount, FALSE ), FALSE );

	_AmmoMayHaveChanged( bNotify );

	return nReturnValue;
}


void CWeapon::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CEntity::ClassHierarchyWork();

#if 0
	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}
#endif

	// Timer to prevent too many weapon sounds from being kicked off...
	if( m_fSecsUntilNextSound > 0.0f ) {
		m_fSecsUntilNextSound -= FLoop_fPreviousLoopSecs;
	} else {
		m_fSecsUntilNextSound = 0.0f;
	}

	if( m_pInfo->nInfoFlags & INFOFLAG_DISPLAY_AMMO_TEXT ) {
		if( IsOwnedByPlayer() ) {
			if( m_pInfo->nReticleType != CReticle::TYPE_NONE ) {
				CReticle *pReticle = &Player_aPlayer[ GetOwner()->m_nPossessionPlayerIndex ].m_Reticle;

				if( HasAmmo() ) {
					// We have at least some ammo...

					if( m_pInfo->nClipAmmoMax != INFINITE_AMMO ) {
						u16 nLowAmmo = (GetMaxClipAmmo() + 3) >> 2;

						if( ComputeTotalAmmo() <= nLowAmmo ) {
							pReticle->SetAmmoText( CReticle::AMMO_TEXT_LOW );
						} else if( GetClipAmmo() <= nLowAmmo ) {
							pReticle->SetAmmoText( CReticle::AMMO_TEXT_RELOAD );
						}
					}
				} else {
					// We have no ammo...

					pReticle->SetAmmoText( CReticle::AMMO_TEXT_NO );
				}
			}
		}
	}
}


CWeapon::AmmoAlert_e CWeapon::ComputeAmmoAlert( void ) const {
	if( m_pInfo->nInfoFlags & INFOFLAG_THROWABLE ) {
		if( (GetClipAmmo() > 1) || (GetMaxClipAmmo() <= 1) ) {
			return AMMO_ALERT_OK;
		} else if( GetClipAmmo() == 1 ) {
			return AMMO_ALERT_LOW_AMMO;
		} else {
			return AMMO_ALERT_NO_AMMO;
		}
	}

	if( m_pInfo->nInfoFlags & INFOFLAG_NO_AMMO ) {
		return AMMO_ALERT_OK;
	}

	if( (m_pInfo->nClipAmmoMax == INFINITE_AMMO) || (m_pInfo->nReserveAmmoMax == INFINITE_AMMO) ) {
		return AMMO_ALERT_OK;
	}

	if( HasAmmo() ) {
		// We have at least some ammo...

		u16 nLowAmmo = (GetMaxClipAmmo() + 3) >> 2;

		if( ComputeTotalAmmo() <= nLowAmmo ) {
			return AMMO_ALERT_LOW_AMMO;
		}
	} else {
		// We have no ammo...

		return AMMO_ALERT_NO_AMMO;
	}

	return AMMO_ALERT_OK;
}


void CWeapon::HumanTargeting_FindDesiredTargetPoint_WS( CFVec3A *pTargetPoint_WS ) {
#if 1
	FASSERT( IsCreated() );
	FASSERT( IsOwnedByPlayer() );
	FASSERT( m_pOwnerBot );

	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f; 

	// For standard weapons, use the owner bot's human targeting routines
	m_pOwnerBot->ComputeHumanTargetPoint_WS( pTargetPoint_WS, this );
#else
	CFCamera *pCamera;
	CReticle *pReticle;
	const CFXfm *pCamXfm;
	const FViewport_t *pViewport;
	f32 fReticleX, fReticleY, fMaxLiveRange, fEndPointReticleAdjust;
	CFVec3A RayStartPoint, RayEndPoint, ReticleAdjustX, ReticleAdjustY;
	FCollImpact_t CollImpact;
	CFTrackerCollideRayInfo CollRayInfo;
	CFWorldTracker *pHitTracker;
	CEntity *pHitEntity;
	BOOL bRayHitSomething;

	FASSERT( IsCreated() );

	FASSERT( IsOwnedByPlayer() );

	s32 nPlayerIndex = GetOwner()->m_nPossessionPlayerIndex;

	m_pTargetedEntity = NULL;
	m_fTargetedEntityHoldTimer = 0.0f;

	// Get reticle info...
	pReticle = &Player_aPlayer[ nPlayerIndex ].m_Reticle;
	fReticleX = pReticle->GetNormOriginX();
	fReticleY = pReticle->GetNormOriginY();

	pReticle->ColorScheme_SelectInactive();

	// Get camera and viewport info...
	pCamera = fcamera_GetCameraByIndex( nPlayerIndex );
	pCamXfm = pCamera->GetFinalXfm();
	pViewport = pCamera->GetViewport();

	// Ray start point is the camera position...
	RayStartPoint.Set( pCamXfm->m_MtxR.m_vPos );

	// Compute the ray end point which lies in the center of the reticle...
	fMaxLiveRange = m_pInfo->fMaxLiveRange;

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleX) * pViewport->fTanHalfFOVX;
	ReticleAdjustX.Mul( pCamXfm->m_MtxR.m_vRight, fEndPointReticleAdjust );

	fEndPointReticleAdjust = (fMaxLiveRange * fReticleY) * pViewport->fTanHalfFOVY;
	ReticleAdjustY.Mul( pCamXfm->m_MtxR.m_vUp, fEndPointReticleAdjust );

	RayEndPoint.Mul( pCamXfm->m_MtxR.m_vFront, fMaxLiveRange ).Add( RayStartPoint ).Add( ReticleAdjustX ).Add( ReticleAdjustY );

	if( m_pInfo->nInfoFlags & INFOFLAG_NO_AUTO_TARGETING ) {
		*pTargetPoint_WS = RayEndPoint;
		return;
	}

	// Build tracker skip list...
	FWorld_nTrackerSkipListCount = 0;
	if( m_pOwnerBot ) {
		m_pOwnerBot->AppendTrackerSkipList();
	}

	u16 nCollMask;

	if( !(m_pInfo->nInfoFlags & INFOFLAG_THICK_TARGETING) ) {
		nCollMask = FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES;
	} else {
		nCollMask = FCOLL_MASK_COLLIDE_WITH_THICK_PROJECTILES;
	}

	PROTRACK_BEGINBLOCK("FindTargetPtColl");
	// Find the closest impact to the camera...
	if( !fworld_FindClosestImpactPointToRayStart( &CollImpact, &RayStartPoint, &RayEndPoint, FWorld_nTrackerSkipListCount, (const CFWorldTracker **)FWorld_apTrackerSkipList, TRUE, NULL, -1, nCollMask ) ) {
		// Our ray didn't hit anything...
	PROTRACK_ENDBLOCK();
		bRayHitSomething = FALSE;
	} else {
		// Our ray hit something...
  	PROTRACK_ENDBLOCK();

		bRayHitSomething = TRUE;

		pHitTracker = (CFWorldTracker *)CollImpact.pTag;

		if( pHitTracker ) {
			// We hit a tracker...

			if( pHitTracker->m_nUser == MESHTYPES_ENTITY ) {
				// We hit an entity...

				pHitEntity = (CEntity *)pHitTracker->m_pUser;

				if( pHitEntity->IsTargetable() ) {
					// It's a targetable entity...

					m_pTargetedEntity = pHitEntity;

					pTargetPoint_WS->Set( CollImpact.ImpactPoint );
					pReticle->ColorScheme_SelectActive();
					return;
				}
			}
		}
	}

	// We did not hit a targetable entity...

	CollRayInfo.pCallback = _FindTrackersIntersectingRayCallback;
	CollRayInfo.nTrackerTypeBits = FWORLD_TRACKERTYPEBIT_MESH;
	CollRayInfo.bIgnoreCollisionFlag = FALSE;
	CollRayInfo.bComputeIntersection = FALSE;
	CollRayInfo.bSort = FALSE;
	CollRayInfo.StartPoint_WS = RayStartPoint;
	CollRayInfo.EndPoint_WS = RayEndPoint;
	CollRayInfo.nTrackerUserTypeBitsMask = 0xffffffffffffffff;
	CollRayInfo.nTrackerSkipCount = FWorld_nTrackerSkipListCount;
	CollRayInfo.ppTrackerSkipList = FWorld_apTrackerSkipList;

	m_pCallback_ClosestEntity = NULL;
	m_pCallback_RayStartPoint = &RayStartPoint;
	m_pCallback_RayEndPoint = &RayEndPoint;

	fworld_CollideWithTrackers( &CollRayInfo );

	if( m_pCallback_ClosestEntity == NULL ) {
		// Our ray didn't intersect any targetable entities...

		if( bRayHitSomething ) {
			// We hit something before, so just use that point...
			pTargetPoint_WS->Set( CollImpact.ImpactPoint );
			return;
		} else {
			// Our ray hit nothing at all. Just target the ray's end point...
			*pTargetPoint_WS = RayEndPoint;
			return;
		}
	}

	// exclude vehicles from target assistance
	FASSERT( m_pCallback_ClosestEntity );
	if( m_pCallback_ClosestEntity->TypeBits() & ENTITY_BIT_BOT ) {
		CBot *pBot = (CBot*) m_pCallback_ClosestEntity;
		if( pBot->TypeBits() & ENTITY_BIT_VEHICLE ||
			(pBot->TypeBits() & ENTITY_BIT_SITEWEAPON && pBot->m_pBotDef->m_nSubClass == BOTSUBCLASS_SITEWEAPON_RATGUN) ) {
			if( bRayHitSomething ) {
				// We hit something before, so just use that point...
				pTargetPoint_WS->Set( CollImpact.ImpactPoint );
				return;
			} else {
				// Our ray hit nothing at all. Just target the ray's end point...
				*pTargetPoint_WS = RayEndPoint;
				return;
			}
		}
	}

	// Introduce targeting assistance...

	CFVec3A RaySphereIntersectionPoint_WS, SphereCenter_WS;

	SphereCenter_WS.Set( m_Callback_TrackerSphere.m_Pos );

	if( m_Callback_TrackerSphere.IsIntersecting( RayStartPoint.v3, RayEndPoint.v3, &RaySphereIntersectionPoint_WS.v3 ) ) {
		f32 fDistFromCamToRayIntersect = RaySphereIntersectionPoint_WS.Dist( RayStartPoint );

		if( fDistFromCamToRayIntersect < m_pInfo->fMaxTargetAssistDist ) {
			// Intersect point is within targeting assistance range...

			f32 fUnitAssist = 1.0f - fmath_Div( fDistFromCamToRayIntersect - m_pInfo->fMinTargetAssistDist, m_pInfo->fMaxTargetAssistDist - m_pInfo->fMinTargetAssistDist );
			if( fUnitAssist > 0.0f ) {
				FMATH_CLAMPMAX( fUnitAssist, 1.0f );

				CFVec3A UnitVecFromRayIntersectPointToSphereCenter_WS;
				UnitVecFromRayIntersectPointToSphereCenter_WS.Sub( SphereCenter_WS, RaySphereIntersectionPoint_WS );

				f32 fMag2 = UnitVecFromRayIntersectPointToSphereCenter_WS.MagSq();
				if( fMag2 > 0.01f ) {
//					f32 fOOMag = fmath_InvSqrt( fMag2 );
//					UnitVecFromRayIntersectPointToSphereCenter_WS.Mul( fOOMag );

					m_pTargetedEntity = m_pCallback_ClosestEntity;

					pTargetPoint_WS->Mul( UnitVecFromRayIntersectPointToSphereCenter_WS, 0.5f * fUnitAssist ).Add( RaySphereIntersectionPoint_WS );
					pReticle->ColorScheme_SelectActive();
					return;
				}
			}
		}
	}

	if( bRayHitSomething ) {
		// We hit something before, so just use that point...
		pTargetPoint_WS->Set( CollImpact.ImpactPoint );
	} else {
		// Our ray hit nothing at all. Just target the ray's end point...
		*pTargetPoint_WS = RayEndPoint;
	}
#endif
}


BOOL CWeapon::_FindTrackersIntersectingRayCallback( CFWorldTracker *pTracker, FVisVolume_t *pVolume, const CFVec3 *pIntersectionPoint_WS, f32 fUnitDistToIntersection ) {
	u32 i;

	if( !pTracker->IsCollisionFlagSet() ) {
		return TRUE;
	}

	if( pTracker->m_nUser != MESHTYPES_ENTITY ) {
		// Not an entity...
		return TRUE;
	}

	CFWorldMesh *pWorldMesh = (CFWorldMesh *)pTracker;

	if( pWorldMesh->m_nFlags & FMESHINST_FLAG_NOCOLLIDE ) {
		return TRUE;
	}

	CEntity *pEntity = (CEntity *)pWorldMesh->m_pUser;

	if( !pEntity->IsTargetable() ) {
		// Not targetable...
		return TRUE;
	}

	for( i=0; i<FWorld_nTrackerSkipListCount; ++i ) {
		if( pTracker == FWorld_apTrackerSkipList[i] ) {
			return TRUE;
		}
	}

	// This is a targetable entity...

	// Let's use just a fraction of the tracker's bounding sphere for targeting...
	CFSphere Sphere;
	Sphere = pTracker->GetBoundingSphere();
	Sphere.m_fRadius *= 0.75f;
	if( !Sphere.IsIntersecting( m_pCallback_RayStartPoint->v3, m_pCallback_RayEndPoint->v3 ) ) {
		// Our ray doesn't intersect the smaller sphere...
		return TRUE;
	}

	if( Sphere.IsIntersecting( m_pCallback_RayStartPoint->v3 ) ) {
		// Our camera is inside this sphere...
		return TRUE;
	}

	// Compute distance squared from camera to center of tracker bounding sphere...
	CFVec3A TrackerBoundPoint;
	TrackerBoundPoint.Set( pTracker->GetBoundingSphere().m_Pos );
	f32 fDist2 = m_pCallback_RayStartPoint->DistSq( TrackerBoundPoint );

	if( (m_pCallback_ClosestEntity == NULL) || (fDist2 < m_fCallback_ShortestDistance2) ) {
		// Found a closer target...

		m_pCallback_ClosestEntity = pEntity;
		m_Callback_TrackerSphere = Sphere;
		m_fCallback_ShortestDistance2 = fDist2;
	}

	return TRUE;
}


void CWeapon::CheckpointSaveSelect( s32 nCheckpoint ) {
	// Save self...
	CheckpointSaveList_AddTailAndMark( nCheckpoint );
}


// Saves state for checkpoint.
BOOL CWeapon::CheckpointSave( void ) {
	// save base class data
	CEntity::CheckpointSave();

	// save weapon class data.
	// order must match load order below
	CFCheckPoint::SaveData( m_nUpgradeLevel );
	CFCheckPoint::SaveData( m_nClipAmmo );
	CFCheckPoint::SaveData( m_nReserveAmmo );
	CFCheckPoint::SaveData( m_nBotDamageBoneIndex );

	return TRUE;
}


// Loads state for checkpoint.
void CWeapon::CheckpointRestore( void ) {
	u32 nRestoredUpgradeLevel;
	u16 nClipAmmo;

	// load base class data
	CEntity::CheckpointRestore();

	RemoveFromWorld(); 

	// load weapon class data.
	// order must match save order above
	CFCheckPoint::LoadData( nRestoredUpgradeLevel );
	CFCheckPoint::LoadData( m_nClipAmmo );
	CFCheckPoint::LoadData( m_nReserveAmmo );
	CFCheckPoint::LoadData( m_nBotDamageBoneIndex );

	if( IsInWorld() ) {
		AddToWorld();
	}

	nClipAmmo = m_nClipAmmo;
	SetUpgradeLevel( nRestoredUpgradeLevel );

	// SetUpgrageLevel tries to refill clip to max,
	// here we return it to restored value
	SetClipAmmo( nClipAmmo );

	_AmmoMayHaveChanged( TRUE );
}


void CWeapon::SetBotDamageBoneIndex( s32 nBotDamageBoneIndex ) {
	FASSERT( IsCreated() );
	
	m_nBotDamageBoneIndex = nBotDamageBoneIndex;

	if( m_nBotDamageBoneIndex > -1 ) {
		SetInvincible( FALSE );
		SetTargetable( TRUE );
		SetArmorProfile( &CDamage::m_ArmorProfileNone );
	} else {
		SetInvincible( TRUE );
		SetTargetable( FALSE );
		SetArmorProfile( &CDamage::m_ArmorProfileInfinite );
	}
}


void CWeapon::InflictDamage( CDamageData *pDamageData ) {
	FASSERT( IsCreated() );

	if( m_pOwnerBot==NULL || m_nBotDamageBoneIndex==-1 ) {
		// Don't pass on damage to bot...

		return;
	}

	if( m_pOwnerBot->IsInvincible() ) {
		// Owner bot is invincible...

		return;
	}

	if( m_pOwnerBot->IsInvincible() ) {
		// Owner bot is invincible...

		return;
	}

	// Pass on damage to bot...

	if( pDamageData->m_nDamageLocale != CDamageForm::DAMAGE_LOCALE_IMPACT ) {
		// Don't pass blast damage to the bot since the bot will
		// most likely receive the same blast damage infliction...

		return;
	}

	// Impact damage. Let's replace the bone index with the bot's bone index...

	CDamageForm::TriData_t NewTriData;
	const CDamageForm::TriData_t *pPrevTriData;

	fang_MemCopy( &NewTriData, pDamageData->m_pTriData, sizeof(CDamageForm::TriData_t) );

	NewTriData.nDamageeBoneIndex = m_nBotDamageBoneIndex;

	pPrevTriData = pDamageData->m_pTriData;
	pDamageData->m_pTriData = &NewTriData;

	m_pOwnerBot->InflictDamage( pDamageData );

	pDamageData->m_pTriData = pPrevTriData;
}


BOOL CWeapon::IsOwnedByPlayer( void ) const {
	return m_pOwnerBot && m_pOwnerBot->IsPlayerBot();
}


s32 CWeapon::GetSafeOwnerPlayerIndex( void ) const {
	return m_pOwnerBot ? m_pOwnerBot->m_nPossessionPlayerIndex: -1;
}


CReticle *CWeapon::GetSafeReticle( void ) const {
	s32 nPlayerIndex = GetSafeOwnerPlayerIndex();

	if( nPlayerIndex >= 0 ) {
		return &Player_aPlayer[ nPlayerIndex ].m_Reticle;
	}

	return NULL;
}


void CWeapon::PlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride, const CFVec3A *pSoundPos_WS, CFAudioEmitter **ppUserAudioEmitter ) const {
	if( pSoundPos_WS == NULL ) {
		pSoundPos_WS = &m_MtxToWorld.m_vPos;
	}

	if( IsOwnedByPlayer() ) {
		// Weapon is owned by a player. Play a 2D sound and inform AI about it...
		CFSoundGroup::PlaySound( pSoundGroup, TRUE, pSoundPos_WS, m_pOwnerBot->m_nPossessionPlayerIndex, TRUE, fVolMult, fPitchMult, fRadiusOverride, ppUserAudioEmitter );
	} else {
		// Weapon is not owned by a player. Play a 3D sound and don't tell AI about it...
		CFSoundGroup::PlaySound( pSoundGroup, FALSE, pSoundPos_WS, -1, FALSE, fVolMult, fPitchMult, fRadiusOverride, ppUserAudioEmitter );
	}
}


CFAudioEmitter *CWeapon::AllocAndPlaySound( CFSoundGroup *pSoundGroup, f32 fVolMult, f32 fPitchMult, f32 fRadiusOverride, const CFVec3A *pSoundPos_WS ) const {
	if( pSoundPos_WS == NULL ) {
		pSoundPos_WS = &m_MtxToWorld.m_vPos;
	}

	if( IsOwnedByPlayer() ) {
		// Bot is being controlled by a player. Play a 2D sound and inform AI about it...
		return CFSoundGroup::AllocAndPlaySound( pSoundGroup, TRUE, pSoundPos_WS, fVolMult, fPitchMult, fRadiusOverride );
	} else {
		// Bot is not being controlled by a player. Play a 3D sound and don't tell AI about it...
		return CFSoundGroup::AllocAndPlaySound( pSoundGroup, FALSE, pSoundPos_WS, fVolMult, fPitchMult, fRadiusOverride );
	}
}


void CWeapon::EjectClip( const CFVec3A *pPos_WS, FMesh_t *pEjectMesh, f32 fDebrisMeshScale ) {
	if( pEjectMesh ) {
		if( m_UserProps.pDebrisGroupClipEject ) {
			CFDebrisSpawner DebrisSpawner;
			CFDebrisMesh DebrisMesh;
			CFVec3A ScaledY_WS;
			f32 fMag2, fInvMag, fSpeed;

			DebrisSpawner.InitToDefaults();

			DebrisSpawner.m_Mtx.m_vPos = *pPos_WS;

			ScaledY_WS.Mul( CFVec3A::m_UnitAxisY, fmath_RandomFloatRange( 8.0f, 10.0f ) );

			DebrisSpawner.m_Mtx.m_vZ.Mul( MtxToWorld()->m_vRight, fmath_RandomFloatRange( 3.0f, 5.0f ) );
			DebrisSpawner.m_Mtx.m_vZ.Add( ScaledY_WS );

			if( m_pOwnerBot ) {
				DebrisSpawner.m_Mtx.m_vZ.Add( m_pOwnerBot->m_Velocity_WS );
			}

			fMag2 = DebrisSpawner.m_Mtx.m_vZ.MagSq();
			if( fMag2 > 0.00001f ) {
				fInvMag = fmath_InvSqrt( fMag2 );
				DebrisSpawner.m_Mtx.m_vZ.Mul( fInvMag );

				fSpeed = fmath_Inv( fInvMag );
			} else {
				DebrisSpawner.m_Mtx.m_vZ = CFVec3A::m_UnitAxisY;

				fSpeed = fmath_RandomFloatRange( 8.0f, 10.0f );
			}

			DebrisMesh.m_nFlags = CFDebrisMesh::FLAG_CAN_KICKUP_DUST | CFDebrisMesh::FLAG_FLAT_OBJECT;
			DebrisMesh.m_fScale = fDebrisMeshScale;
			DebrisMesh.m_nTintRed = 255;
			DebrisMesh.m_nTintGreen = 255;
			DebrisMesh.m_nTintBlue = 255;
			DebrisMesh.m_pMesh = pEjectMesh;

			DebrisSpawner.m_pDebrisMesh = &DebrisMesh;
			DebrisSpawner.m_pDebrisGroup = m_UserProps.pDebrisGroupClipEject;
			DebrisSpawner.m_fMinSpeed = fSpeed * fDebrisMeshScale;
			DebrisSpawner.m_fMaxSpeed = fSpeed * fDebrisMeshScale;

			DebrisSpawner.m_pUser = this;
			DebrisSpawner.m_pFcnCallback = _EjectClipDebrisCallback;

			FMATH_SETBITMASK( DebrisSpawner.m_nFlags, CFDebrisSpawner::FLAG_PLAY_IMPACT_SOUND );

			DebrisSpawner.Spawn();
		}
	}
}


void CWeapon::_EjectClipDebrisCallback( CFDebris *pDebris, CFDebrisDef::CallbackReason_e nReason, const FCollImpact_t *pCollImpact ) {
	if( nReason != CFDebrisDef::CALLBACK_REASON_BUILD_TRACKER_SKIP_LIST ) {
		return;
	}

	CWeapon *pWeapon = (CWeapon *)pDebris->GetDebrisDef()->m_pUser;

	if( pWeapon->GetOwner() ) {
		pWeapon->GetOwner()->AppendTrackerSkipList();
	} else {
		pWeapon->AppendTrackerSkipList();
	}
}


#define _AIMCLAMP_OUTSIDE_ANGLE_COS		( 0.76604444311897803520239265055542f	)	// cos(40)
#define _AIMCLAMP_INSIDE_ANGLE_COS		( 0.86602540378443864676372317075294f )		// cos(30)
#define _AIMCLAMP_OO_RANGE				( 1.0f / (_AIMCLAMP_INSIDE_ANGLE_COS - _AIMCLAMP_OUTSIDE_ANGLE_COS) )


void CWeapon::ComputeFireUnitDir( CFVec3A *pUnitFireDir_WS, const CFVec3A *pMuzzlePos_WS, const CFVec3A *pMuzzleDir_WS, const CFVec3A *pTargetPos_WS, BOOL bNegateMuzzleDir ) {
	f32 fMag2;
	CFVec3A MuzzleUnitDir_WS;

	// Compute muzzle unit direction...
	fMag2 = pMuzzleDir_WS->MagSq();
	if( fMag2 < 0.0001f ) {
		// Muzzle direction is too close to the NULL vector. Just fire along the X axis...

		*pUnitFireDir_WS = CFVec3A::m_UnitAxisX;

		return;
	}

	if( !bNegateMuzzleDir ) {
		MuzzleUnitDir_WS.Mul( *pMuzzleDir_WS, fmath_InvSqrt( fMag2 ) );
	} else {
		MuzzleUnitDir_WS.Mul( *pMuzzleDir_WS, -fmath_InvSqrt( fMag2 ) );
	}

	// Compute desired fire unit direction...
	pUnitFireDir_WS->Sub( *pTargetPos_WS, *pMuzzlePos_WS );
	fMag2 = pUnitFireDir_WS->MagSq();
	if( fMag2 < 0.0001f ) {
		// Muzzle and target positions are very close. Fire along muzzle direction...

		*pUnitFireDir_WS = MuzzleUnitDir_WS;

		return;
	}

	pUnitFireDir_WS->Mul( fmath_InvSqrt( fMag2 ) );

#if 1
	if( GetOwner() && (GetOwner()->TypeBits() & ENTITY_BIT_BOTGLITCH) ) {
		return;
	}
#endif

	// Make sure the angle between the muzzle direction and our fire direction isn't too great...

	fMag2 = pUnitFireDir_WS->Dot( MuzzleUnitDir_WS );
	if( fMag2 >= _AIMCLAMP_INSIDE_ANGLE_COS ) {
		// Angle is within tolerance of the muzzle direction. We're good to go
		return;

	} else if( fMag2 <= _AIMCLAMP_OUTSIDE_ANGLE_COS ) {
		// angle is too far out, just fire down the muzzle
		*pUnitFireDir_WS = MuzzleUnitDir_WS;
	}

	// angle is within the range we want to interpolate
	pUnitFireDir_WS->Lerp( (_AIMCLAMP_INSIDE_ANGLE_COS - fMag2) * _AIMCLAMP_OO_RANGE, MuzzleUnitDir_WS );
	pUnitFireDir_WS->Unitize();
}


// Constructs the weapon's damager data and stores it in pDestDamager.
// If pDestDamager is NULL, the data is stored in CDamageForm::m_TempDamager.
void CWeapon::ConstructDamagerData( CDamageForm::Damager_t *pDestDamager ) {
	if( pDestDamager == NULL ) {
		pDestDamager = &CDamageForm::m_TempDamager;
	}

	pDestDamager->nDamagerPlayerIndex = GetSafeOwnerPlayerIndex();
	pDestDamager->pBot = GetOwner();
	pDestDamager->pEntity = this;
	pDestDamager->pWeapon = this;
}


//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	static u32 _uNoWeaponMeshesLoaded = 0;
//#endif

BOOL CWeapon::CreateSharedEUKData( FMeshInit_t *pMeshInit, u32 *puEUK, CFWorldMesh **ppWorldMesh, CFAnimCombiner **ppAnimCombiner/*=NULL*/, CFAnimMeshRest **ppAnimMeshRest/*=NULL*/ ) {
	FASSERT( pMeshInit && ppWorldMesh );
	u32 uNextID = 0;

#if _ENABLE_SHARED_WORLDMESHES
	while( m_aSharedResourceData[uNextID].pWorldMesh ) {
		if( !fclib_strcmp( m_aSharedResourceData[uNextID].pWorldMesh->m_pMesh->szName, pMeshInit->pMesh->szName ) ) {
			// this mesh already loaded, return the data...
			*puEUK = m_aSharedResourceData[uNextID].uEUK;
			*ppWorldMesh = m_aSharedResourceData[uNextID].pWorldMesh;
			
			if( ppAnimCombiner ) {
				*ppAnimCombiner = m_aSharedResourceData[uNextID].pAnimCombiner;
			}

			if( ppAnimMeshRest ) {
				*ppAnimMeshRest = m_aSharedResourceData[uNextID].pAnimMeshRest;
			}

			return TRUE;
		}

		uNextID++;
		FASSERT( uNextID < EUK_COUNT_MAX );
	}

#else
	while( m_aSharedResourceData[uNextID].pWorldMesh ) {
		uNextID++;
		FASSERT( uNextID < EUK_COUNT_MAX );
	}
#endif


	// didn't find it, create new data...
	FASSERT( uNextID < EUK_COUNT_MAX );
	m_aSharedResourceData[uNextID].uEUK = *puEUK;

	m_aSharedResourceData[uNextID].pWorldMesh = fnew CFWorldMesh;

//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
//	DEVPRINTF( "Loading weapon worldmesh.  %d loaded\n", ++_uNoWeaponMeshesLoaded );
//#endif

	if( !m_aSharedResourceData[uNextID].pWorldMesh ) {
		goto _ExitWithError;
	}

	m_aSharedResourceData[uNextID].pWorldMesh->Init( pMeshInit );
	*ppWorldMesh = m_aSharedResourceData[uNextID].pWorldMesh;

	// now the anim combiner
	if( ppAnimCombiner ) {
		*ppAnimCombiner = m_aSharedResourceData[uNextID].pAnimCombiner = fnew CFAnimCombiner;
		if( !m_aSharedResourceData[uNextID].pAnimCombiner ) {
			goto _ExitWithError;
		}
	}

	// now the rest anim
	if( ppAnimMeshRest ) {
		*ppAnimMeshRest = m_aSharedResourceData[uNextID].pAnimMeshRest = fnew CFAnimMeshRest;
		if( !m_aSharedResourceData[uNextID].pAnimMeshRest ) {
			goto _ExitWithError;
		}

		if( !m_aSharedResourceData[uNextID].pAnimMeshRest->Create( m_aSharedResourceData[uNextID].pWorldMesh->m_pMesh ) ) {
			goto _ExitWithError;
		}
	}

	return TRUE;

_ExitWithError:
	if( m_aSharedResourceData[uNextID].pWorldMesh ) {
		fdelete( m_aSharedResourceData[uNextID].pWorldMesh );
		m_aSharedResourceData[uNextID].pWorldMesh = NULL;
	}

	if( m_aSharedResourceData[uNextID].pAnimCombiner ) {
		fdelete( m_aSharedResourceData[uNextID].pAnimCombiner );
		m_aSharedResourceData[uNextID].pAnimCombiner = NULL;
	}

	if( m_aSharedResourceData[uNextID].pAnimMeshRest ) {
		fdelete( m_aSharedResourceData[uNextID].pAnimMeshRest );
		m_aSharedResourceData[uNextID].pAnimMeshRest = NULL;
	}

	*ppWorldMesh = NULL;
	if( ppAnimCombiner ) {
		*ppAnimCombiner = NULL;
	}
	if( ppAnimMeshRest ) {
		*ppAnimMeshRest = NULL;
	}
	return FALSE;

}


BOOL CWeapon::DestroySharedEUKData( u32 uEUK ) {
	for( u32 i=0; i<EUK_COUNT_MAX; i++ ) {
		if( uEUK == m_aSharedResourceData[i].uEUK ) {
			if( m_aSharedResourceData[i].pWorldMesh ) {
				//#if SAS_ACTIVE_USER == SAS_USER_ELLIOTT
				//	DEVPRINTF( "deleting weapon worldmesh.  %d remaining\n", --_uNoWeaponMeshesLoaded );
				//#endif
				fdelete( m_aSharedResourceData[i].pWorldMesh );
				m_aSharedResourceData[i].pWorldMesh = NULL;
			}

			if( m_aSharedResourceData[i].pAnimCombiner ) {
				fdelete( m_aSharedResourceData[i].pAnimCombiner );
				m_aSharedResourceData[i].pAnimCombiner = NULL;
			}

			if( m_aSharedResourceData[i].pAnimMeshRest ) {
				fdelete( m_aSharedResourceData[i].pAnimMeshRest );
				m_aSharedResourceData[i].pAnimMeshRest = NULL;
			}
			return TRUE;
		}
	}

	return FALSE;
}


void CWeapon::_BuildProjSkipList( CEProj *pProj ) {
	CWeapon *pWeapon = (CWeapon*)pProj->GetDamagerWeapon();
	FASSERT( pWeapon && (pWeapon->TypeBits() & ENTITY_BIT_WEAPON) );

	pWeapon->m_pOwnerBot->AppendTrackerSkipList();
}


void CWeapon::Throwable_ThrowGrenade( CEProj *pProjToThrow, const CFVec3A &vVelocity_WS, CFSoundGroup *pSoundToPlay/*=NULL*/ ) {
	CFVec3A vLaunchUnitVec;
	f32 fLaunchSpeed;

	// Linear velocity
	fLaunchSpeed = vLaunchUnitVec.SafeUnitAndMag( vVelocity_WS );
	if( fLaunchSpeed <= 0.0f ) {
		fLaunchSpeed = 0.0f;
		vLaunchUnitVec = CFVec3A::m_UnitAxisY;
	}

	pProjToThrow->SetLinSpeed( fLaunchSpeed );
	pProjToThrow->SetLinUnitDir_WS( &vLaunchUnitVec );

	// Angular velocity
	CFVec3A RotVel_WS;
	if( fmath_RandomChance( 0.5f ) ) {
		RotVel_WS.Set( -fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	} else {
		RotVel_WS.Set( fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	}
	RotVel_WS.Unitize();
	
	pProjToThrow->SetRotUnitAxis_WS( &RotVel_WS );
	pProjToThrow->SetRotSpeed( fmath_RandomFloatRange( FMATH_DEG2RAD( 180.0f ), FMATH_DEG2RAD( 360.0f ) ) );

	// Launch it
	pProjToThrow->SetSkipListCallback( _BuildProjSkipList );
	pProjToThrow->DetachFromParent();
	pProjToThrow->Launch();

	// Remove the round we launched from the clip...
	RemoveFromClip( 1 );

	// play a sound
	if( pSoundToPlay ) {
		PlaySound( pSoundToPlay );
	}
}

// These values were taken from weapon.h
u32 CWeapon::GetMaxUpgradeLevelForGlitch( WeaponType_e eType ) {
	switch( eType ) {
		case WEAPON_TYPE_EMP:
		case WEAPON_TYPE_MORTAR:
		case WEAPON_TYPE_FLAMER:
		case WEAPON_TYPE_WRENCH:
		case WEAPON_TYPE_CLEANER:
		case WEAPON_TYPE_GRENADE:
		case WEAPON_TYPE_MAGMABOMB:
		case WEAPON_TYPE_RECRUITER:
		case WEAPON_TYPE_BLINK_RIGHT_HAND:
			return 0;
		break;
		case WEAPON_TYPE_SCOPE:
			return 1;
		break;
		case WEAPON_TYPE_SPEW:
		case WEAPON_TYPE_LASER:
		case WEAPON_TYPE_TETHER:
		case WEAPON_TYPE_RIPPER:
		case WEAPON_TYPE_BLASTER:
		case WEAPON_TYPE_RIVET_GUN:
		case WEAPON_TYPE_ROCKET_LAUNCHER:
			return 2;
		break;

		default:
			return 0;
		break;
	}
}
