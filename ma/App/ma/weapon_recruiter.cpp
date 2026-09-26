//////////////////////////////////////////////////////////////////////////////////////
// weapon_recruiter.cpp - Recruiter grenade weapon.
//
// Author: Steve Ranck     
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
// 04/28/03 Ranck       Created.
//////////////////////////////////////////////////////////////////////////////////////

#include "fang.h"
#include "weapon_recruiter.h"
#include "weapon.h"
#include "eproj.h"
#include "iteminst.h"
#include "fresload.h"
#include "fsndfx.h"
#include "bot.h"
#include "fmeshpool.h"
#include "frenderer.h"
#include "fdraw.h"
#include "fvtxpool.h"
#include "muzzleflash.h"
#include "MultiplayerMgr.h"
#include "ai\aiapi.h"
#include "ai\aibrain.h"
#include "ai\aimover.h"



#define _USER_PROP_FILENAME		"w_recruiter.csv"

#define _RECRUITED_BOT_SHOCK_SECS			0.75f
#define _ICON_INTRODUCTION_ANIM_SECS		1.0f
#define _ICON_INTRODUCTION_ANIM_AMP_Y		2.0f
#define _ICON_INTRODUCTION_ANIM_AMP_SCALE	2.0f
#define _ICON_SPIN_REVS_PER_SEC				0.5f



FCLASS_NOALIGN_PREFIX class CRecruitData {
public:

	enum State_e {
		STATE_UNRECRUITED,			// Unused slot
		STATE_SHOCKING,				// Bot is being shocked
		STATE_INTRODUCING_ICON,		// Playing icon introduction animation
		STATE_SPINNING_ICON,		// Spinning the icon

		STATE_COUNT
	};


	CBot *m_pBot;					// The recruited bot
	CBot *m_pRecruiterBot;			// The bot that recruited him
	u8 m_nNewRace;					// The new race this bot will become
	u8 m_nNewTeam;					// The new team this bot will be on
	u8 m_nOrigTeam;					// The original team for this bot
	BOOL8 m_bEpicenterProvided;		// TRUE if m_Epicenter_WS was provided

	State_e m_nState;				// Recruitment state
	f32 m_fTimer;					// Used to time animations

	f32 m_fIconUnitOffsetY;			// Unit Y offset to displace icon above bot's head
	f32 m_fIconUnitScale;			// Unit scale of icon

	CFVec3 m_Epicenter_WS;			// NULL=none

	u8 m_nOrigRace;					// AI: Original race
	u8 m_nOrigAgression;			// AI: Original aggression level (0-100)
	u8 m_nOrigIntelligence;			// AI: Original intelligence level (0-100)
	u8 m_nOrigCourage;				// AI: Original courage level (0-100)
	u32 m_nOrigBuddyBrainFlags;		// AI: Original buddy brain flags
	f32 m_fOrigWeaponUnitAccuracy;	// AI: Original weapon unit accuracy (0-1)


	FCLASS_STACKMEM_NOALIGN( CRecruitData );
} FCLASS_NOALIGN_SUFFIX;




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRecruiterBuilder
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

static CWeaponRecruiterBuilder _WeaponGrenBuilder;


void CWeaponRecruiterBuilder::SetDefaults( u64 nEntityTypeBits, u64 nEntityLeafTypeBit, cchar *pszEntityType ) {
	ENTITY_BUILDER_SET_PARENT_CLASS_DEFAULTS( CWeaponBuilder, ENTITY_BIT_UNSPECIFIED, pszEntityType );
}


BOOL CWeaponRecruiterBuilder::PostInterpretFixup( void ) {
	return CWeaponBuilder::PostInterpretFixup();
}


BOOL CWeaponRecruiterBuilder::InterpretTable( void ) {
	return CWeaponBuilder::InterpretTable();
}




//**********************************************************************************************************************************
//**********************************************************************************************************************************
//
// CWeaponRecruiter
//
//**********************************************************************************************************************************
//**********************************************************************************************************************************

u32 CWeaponRecruiter::m_nWeaponClassClientCount = 0;
CFBolt::CFBoltData CWeaponRecruiter::m_BoltData;
CWeaponRecruiter::_UserProps_t CWeaponRecruiter::m_aUserProps[EUK_COUNT_RECRUITER];
CEProjPool::PoolHandle_t CWeaponRecruiter::m_ahProjPool[EUK_COUNT_RECRUITER];
SmokeTrailAttrib_t CWeaponRecruiter::m_SmokeTrailAttrib;
CFTexInst CWeaponRecruiter::m_StreamerTexInst;
CFMeshPool *CWeaponRecruiter::m_pIconMeshPool;

static CRecruitData *_pRecruitDataArray;


// This table describes to fgamedata how our user property table is to be interpreted:
const FGameData_TableEntry_t CWeaponRecruiter::m_aUserPropVocab[] = {
	// apszMeshName[_MESH_GRENADE]:
	FGAMEDATA_VAR_TYPE_STRING|
	FGAMEDATA_FLAGS_STRING_PTR_TO_MAIN_STR_TBL,
	sizeof( char * ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fClipAmmoMax:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Neg1,
	F32_DATATABLE_65534,

	// fRoundsPerSec:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_1000,

	// nMaxRecruites:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_CONVERT_TO_U32 | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( u32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100,

	FGAMEDATA_VOCAB_MESH,	// pMeshIcon
	FGAMEDATA_VOCAB_TEXDEF,	// pTexDefBolt

	// fGrenadeCullDist:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fGrenadesInPoolCount:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_1000,

	// fMaxLiveRange:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fGrenadeSpeed:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_1,
	F32_DATATABLE_100000,

	// fGrenadeLife:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_100000,

	// fGrenadeMeshScale:
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X | FGAMEDATA_FLAGS_FLOAT_CLAMP_AND_GO,
	sizeof( f32 ),
	F32_DATATABLE_Pt001,
	F32_DATATABLE_100000,

	// fFatRadiusDelta
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationRadius
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationLife
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	// fDetonationTime
	FGAMEDATA_VAR_TYPE_FLOAT|
	FGAMEDATA_FLAGS_FLOAT_X,
	sizeof( f32 ),
	F32_DATATABLE_0,
	F32_DATATABLE_0,

	FGAMEDATA_VOCAB_EXPLODE_GROUP,	// hExplosionGroup
	FGAMEDATA_VOCAB_PARTICLE,		// hParticleIconOff

	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupUse
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBounce
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupEffect
	FGAMEDATA_VOCAB_SOUND_GROUP,	// pSoundGroupBolt


	// End of table:
	FGAMEDATA_VAR_TYPE_COUNT| 0, 0, F32_DATATABLE_0, F32_DATATABLE_0
};


const FGameDataMap_t CWeaponRecruiter::m_aUserPropMapTable[] = {
	"Gren",
	m_aUserPropVocab,
	sizeof(m_aUserProps),
	(void *)&m_aUserProps[0],


	NULL
};



BOOL CWeaponRecruiter::InitSystem( void ) {
	Info_t *pInfo;
	u32 i;
	FTexDef_t *pTexDef;

	FResFrame_t ResFrame = fres_GetFrame();

	fang_MemZero( m_aUserProps, sizeof(m_aUserProps) );

	m_nWeaponClassClientCount = 0;

	// Read the user properties for all EUK levels of this weapon...
	if( !fgamedata_ReadFileUsingMap( m_aUserPropMapTable, _USER_PROP_FILENAME ) ) {
		DEVPRINTF( "CWeaponRecruiter::InitSystem(): Could not read user properties from file '%s'.\n", _USER_PROP_FILENAME );
		goto _ExitWithError;
	}

	// Do this for each EUK level...
	for( i=0; i<EUK_COUNT_RECRUITER; i++ ) {
		// Create a pool of projectiles...
		m_ahProjPool[i] = CEProjPool::Create( CEProj::PROJTYPE_GRENADE, m_aUserProps[i].apszMeshName[_MESH_GRENADE], (u32)m_aUserProps[i].fGrenadesInPoolCount );

		if( m_ahProjPool[i] == EPROJPOOL_NULL_HANDLE ) {
			DEVPRINTF( "CWeaponRecruiter::InitSystem(): Could not create projectile pool.\n" );
			goto _ExitWithError;
		}

		// Fill out our global info data...
		pInfo = &m_aaInfo[WEAPON_TYPE_RECRUITER][i];

		pInfo->nGrip = GRIP_LEFT_ARM;
		pInfo->nReloadType = RELOAD_TYPE_FULLYAUTOMATIC;
		pInfo->nStanceType = STANCE_TYPE_STANDARD;
		pInfo->fMinTargetAssistDist = 0.0f;
		pInfo->fMaxTargetAssistDist = 0.0f;
		pInfo->fMaxLiveRange = m_aUserProps[i].fMaxLiveRange;
		pInfo->nClipAmmoMax = (m_aUserProps[i].fClipAmmoMax) >= 0.0f ? (u16)m_aUserProps[i].fClipAmmoMax : INFINITE_AMMO;
		pInfo->nReserveAmmoMax = INFINITE_AMMO;
		pInfo->nInfoFlags = INFOFLAG_THROWABLE;
		pInfo->nReticleType = CReticle::TYPE_NONE;
	}

	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "tf_plasma01" );
	if( pTexDef == NULL ) {
		DEVPRINTF( "CWeaponRecruiter::InitSystem(): Could not load smoke trail texture.\n" );
	}

	m_SmokeTrailAttrib.nFlags = SMOKETRAIL_FLAG_ADDITIVE_BLEND_MODE;//SMOKETRAIL_FLAG_NONE;
	m_SmokeTrailAttrib.pTexDef = pTexDef;

	m_SmokeTrailAttrib.fScaleMin_WS = 0.5f;
	m_SmokeTrailAttrib.fScaleMax_WS = 1.0f;
	m_SmokeTrailAttrib.fScaleSpeedMin_WS = -0.5f;//0.5f;
	m_SmokeTrailAttrib.fScaleSpeedMax_WS = -0.8f;//0.8f;
	m_SmokeTrailAttrib.fScaleAccelMin_WS = -1.0f;
	m_SmokeTrailAttrib.fScaleAccelMax_WS = -1.5f;

	m_SmokeTrailAttrib.fXRandSpread_WS = 0.25f;//0.05f;
	m_SmokeTrailAttrib.fYRandSpread_WS = 0.25f;//0.05f;
	m_SmokeTrailAttrib.fDistBetweenPuffs_WS = 0.2f;//0.1f;
	m_SmokeTrailAttrib.fDistBetweenPuffsRandSpread_WS = 0.025f;

	m_SmokeTrailAttrib.fYSpeedMin_WS = 0.25f;
	m_SmokeTrailAttrib.fYSpeedMax_WS = 0.5f;
	m_SmokeTrailAttrib.fYAccelMin_WS = -0.2f;
	m_SmokeTrailAttrib.fYAccelMax_WS = -0.5f;

	m_SmokeTrailAttrib.fUnitOpaqueMin_WS = 0.2f;
	m_SmokeTrailAttrib.fUnitOpaqueMax_WS = 0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMin_WS = -0.5f;
	m_SmokeTrailAttrib.fUnitOpaqueSpeedMax_WS = -2.5f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMin_WS = 0.0f;
	m_SmokeTrailAttrib.fUnitOpaqueAccelMax_WS = 0.0f;

	m_SmokeTrailAttrib.StartColorRGB.Set( 1.0f, 1.0f, 0.2f );
	m_SmokeTrailAttrib.EndColorRGB.Set( 1.0f, 1.0f, 1.0f );
	m_SmokeTrailAttrib.fStartColorUnitIntensityMin = 0.5f;
	m_SmokeTrailAttrib.fStartColorUnitIntensityMax = 0.7f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMin = 0.0f;
	m_SmokeTrailAttrib.fEndColorUnitIntensityMax = 0.1f;

	m_SmokeTrailAttrib.fColorUnitSliderSpeedMin = 3.5f;
	m_SmokeTrailAttrib.fColorUnitSliderSpeedMax = 4.5f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMin = 0.0f;
	m_SmokeTrailAttrib.fColorUnitSliderAccelMax = 0.0f;

	// Load streamer texture...
	pTexDef = (FTexDef_t *)fresload_Load( FTEX_RESNAME, "trdmstrmry1" );
	m_StreamerTexInst.SetTexDef( pTexDef );
	m_StreamerTexInst.SetFlags( CFTexInst::FLAG_WRAP_S | CFTexInst::FLAG_WRAP_T );

	m_pIconMeshPool = NULL;
	_pRecruitDataArray = NULL;

	m_BoltData.m_TexInst.SetTexDef( m_aUserProps[0].pTexDefBolt );
	m_BoltData.m_Color.Set( 0.4f, 0.4f, 1.0f, 1.0f );
	m_BoltData.m_nMaxJointCount = 100;
	m_BoltData.m_nBoltCount = 10;
	m_BoltData.m_fDistBetweenJoints = 1.0f;
	m_BoltData.m_fTrumpetRadius = 10.0f;
	m_BoltData.m_fMinBoltThickness = 0.8f;
	m_BoltData.m_fMaxBoltThickness = 0.8f;
	m_BoltData.m_fMinBoltDisplacement = 0.2f;
	m_BoltData.m_fMaxBoltDisplacement = 1.0f;
	m_BoltData.m_fUnitEmitterPinch = 1.0f;
	m_BoltData.m_fUnitDistPinch = 0.0f;
	m_BoltData.m_StartPos_WS.Zero();
	m_BoltData.m_EndPos_WS.Zero();

	// Success...

	return TRUE;

	// Error...
_ExitWithError:
	UninitSystem();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponRecruiter::UninitSystem( void ) {
	m_pIconMeshPool = NULL;
	_pRecruitDataArray = NULL;
}


BOOL CWeaponRecruiter::ClassHierarchyLoadSharedResources( void ) {
	FASSERT( m_nWeaponClassClientCount != 0xffffffff );

	++m_nWeaponClassClientCount;

	if( !CWeapon::ClassHierarchyLoadSharedResources() ) {
		// Bail now since parent class has already called
		// ClassHierarchyUnloadSharedResources() and decremented
		// our client counter...

		return FALSE;
	}

	if( m_nWeaponClassClientCount > 1 ) {
		// Resources already loaded...

		return TRUE;
	}

	// Resources not yet loaded...

	FResFrame_t ResFrame = fres_GetFrame();

	// Create icon mesh pool...

	m_pIconMeshPool = NULL;
	_pRecruitDataArray = NULL;

	if( m_aUserProps[0].nMaxRecruites ) {
		m_pIconMeshPool = CFMeshPool::CreatePool( m_aUserProps[0].nMaxRecruites );

		if( m_pIconMeshPool == NULL ) {
			DEVPRINTF( "CWeaponRecruiter::ClassHierarchyBuild(): Not enough memory to create m_pIconMeshPool.\n" );
			goto _ExitWithError;
		}

		_pRecruitDataArray = fnew CRecruitData[ m_aUserProps[0].nMaxRecruites ];
		if( _pRecruitDataArray == NULL ) {
			DEVPRINTF( "CWeaponRecruiter::ClassHierarchyBuild(): Not enough memory to create _pRecruitDataArray.\n" );
			goto _ExitWithError;
		}
	}

	if( m_pIconMeshPool ) {
		CFWorldMeshItem *pWorldMeshItem;
		FMeshInit_t MeshInit;
		u32 i;

		MeshInit.pMesh = m_aUserProps[0].pMeshIcon;
		MeshInit.nFlags = FMESHINST_FLAG_NOCOLLIDE;
		MeshInit.fCullDist = FMATH_MAX_FLOAT;
		MeshInit.Mtx.Identity();

		for( i=0; i<m_aUserProps[0].nMaxRecruites; ++i ) {
			pWorldMeshItem = m_pIconMeshPool->GetItemByIndex( i );

			pWorldMeshItem->Init( &MeshInit );
			pWorldMeshItem->SetCollisionFlag( FALSE );
			pWorldMeshItem->SetLineOfSightFlag( FALSE );
			pWorldMeshItem->RemoveFromWorld();

			_pRecruitDataArray[i].m_pBot = NULL;
			_pRecruitDataArray[i].m_pRecruiterBot = NULL;
			_pRecruitDataArray[i].m_nNewRace = AIRACE_AMBIENT;
			_pRecruitDataArray[i].m_nNewTeam = 0;
			_pRecruitDataArray[i].m_bEpicenterProvided = FALSE;
			_pRecruitDataArray[i].m_nState = CRecruitData::STATE_UNRECRUITED;
			_pRecruitDataArray[i].m_fTimer = 0.0f;
			_pRecruitDataArray[i].m_fIconUnitOffsetY = 0.0f;
			_pRecruitDataArray[i].m_fIconUnitScale = 0.001f;
			_pRecruitDataArray[i].m_Epicenter_WS.Zero();

			_pRecruitDataArray[i].m_nOrigTeam = 0;
			_pRecruitDataArray[i].m_nOrigRace = AIRACE_AMBIENT;
			_pRecruitDataArray[i].m_nOrigAgression = 0;
			_pRecruitDataArray[i].m_nOrigIntelligence = 0;
			_pRecruitDataArray[i].m_nOrigCourage = 0;
			_pRecruitDataArray[i].m_nOrigBuddyBrainFlags = 0;
			_pRecruitDataArray[i].m_fOrigWeaponUnitAccuracy = 0.0f;
		}
	}

	return TRUE;

	// Error:
_ExitWithError:
	ClassHierarchyUnloadSharedResources();
	fres_ReleaseFrame( ResFrame );

	return FALSE;
}


void CWeaponRecruiter::ClassHierarchyUnloadSharedResources( void ) {
	FASSERT( m_nWeaponClassClientCount > 0 );

	--m_nWeaponClassClientCount;

	if( m_nWeaponClassClientCount == 0 ) {
		if( m_pIconMeshPool ) {
			UnrecruitAllBots();

			m_pIconMeshPool->ReturnAllToFreePool();
			CFMeshPool::DestroyPool( m_pIconMeshPool );
		}

		m_pIconMeshPool = NULL;
		_pRecruitDataArray = NULL;
	}

	CWeapon::ClassHierarchyUnloadSharedResources();
}


CWeaponRecruiter::CWeaponRecruiter() {
	_ClearDataMembers();
}


CWeaponRecruiter::~CWeaponRecruiter() {
	if( IsSystemInitialized() && IsCreated() ) {
		DetachFromParent();
		DetachAllChildren();
		RemoveFromWorld( TRUE );
		ClassHierarchyDestroy();
	}
}


BOOL CWeaponRecruiter::Create( cchar *pszEntityName, const CFMtx43A *pMtx, cchar *pszAIBuilderName ) {
	if( !ClassHierarchyLoadSharedResources() ) {
		// Failure! (resources have already been cleaned up, so we can bail now)
		return FALSE;
	}

	// Get pointer to the leaf class's builder object...
	CWeaponRecruiterBuilder *pBuilder = (CWeaponRecruiterBuilder *)GetLeafClassBuilder();

	// If we're the leaf class, set the builder defaults...
	if( pBuilder == &_WeaponGrenBuilder ) {
		pBuilder->SetDefaults( 0, 0, ENTITY_TYPE_RECRUITER );
	}

	// Set our builder parameters...

	// Create an entity...
	return CWeapon::Create( WEAPON_TYPE_RECRUITER, pszEntityName, pMtx, pszAIBuilderName );
}


BOOL CWeaponRecruiter::ClassHierarchyBuild( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( !IsCreated() );
	FASSERT( FWorld_pWorld );

	// Get a frame...
	FResFrame_t ResFrame = fres_GetFrame();

	// Get pointer to the leaf class's builder object...
	CWeaponRecruiterBuilder *pBuilder = (CWeaponRecruiterBuilder *)GetLeafClassBuilder();

	// Set input parameters for CWeapon creation...

	// Build parent class...
	if( !CWeapon::ClassHierarchyBuild() ) {
		// Parent class could not be built...
		goto _ExitWithError;
	}

	// Set defaults...
	_ClearDataMembers();

	// Initialize from builder object...

	m_pResourceData = &m_aResourceData[ m_nUpgradeLevel ];

	return TRUE;

	// Error:
_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


BOOL CWeaponRecruiter::ClassHierarchyBuilt( void ) {
	FASSERT( IsSystemInitialized() );
	FASSERT( IsCreated() );

	FResFrame_t ResFrame = fres_GetFrame();

	if( !CWeapon::ClassHierarchyBuilt() ) {
		goto _ExitWithError;
	}

	EnableOurWorkBit();

	return TRUE;

_ExitWithError:
	Destroy();
	fres_ReleaseFrame( ResFrame );
	return FALSE;
}


void CWeaponRecruiter::ClassHierarchyDestroy( void ) {
	// Destroy ourselves first...
	_ClearDataMembers();

	// Now destroy our parent...
	CWeapon::ClassHierarchyDestroy();
}


CEntityBuilder *CWeaponRecruiter::GetLeafClassBuilder( void ) {
	return &_WeaponGrenBuilder;
}


void CWeaponRecruiter::_ClearDataMembers( void ) {
	m_pResourceData = NULL;
	m_fSecondsCountdownTimer = 0.0f;
	m_pProjToThrow = NULL;
}


void CWeaponRecruiter::ClassHierarchyRemoveFromWorld( void ) {
	FASSERT( IsCreated() );
	FASSERT( IsInWorld() );

	CWeapon::ClassHierarchyRemoveFromWorld();

	if( m_pProjToThrow ) {
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );
		m_pProjToThrow = NULL;
	}
}


void CWeaponRecruiter::SetItemInst( CItemInst *pItemInst, BOOL bUpdateItemInstAmmoFromWeaponAmmo ) {
	CWeapon::SetItemInst( pItemInst, bUpdateItemInstAmmoFromWeaponAmmo );

	if( pItemInst ) {
		pItemInst->SetAmmoDisplayType( CItemInst::AMMODISPLAY_NUMERIC, CItemInst::AMMODISPLAY_NONE );
	}
}


BOOL CWeaponRecruiter::Throwable_TriggerWork( f32 fUnitTriggerVal ) {
	if( fUnitTriggerVal <= 0.0f ) {
		// Trigger not down...
		return FALSE;
	}

	if( CurrentState() != STATE_DEPLOYED ) {
		// Not in a state to fire...
		return FALSE;
	}

	if( m_nClipAmmo == 0 ) {
		// No rounds in clip...
		return FALSE;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		// Can't fire another round yet...
		return FALSE;
	}

	if( !_GetProjectileFromPoolAndInit() ) {
		// No more projectiles...
		return FALSE;
	}

	m_fSecondsCountdownTimer = m_aUserProps[ m_nUpgradeLevel ].fRoundsPerSec;

	return TRUE;
}


BOOL CWeaponRecruiter::_GetProjectileFromPoolAndInit( void ) {
	if( m_pProjToThrow ) {
		// Our projectile hasn't been returned yet...
		return FALSE;
	}

	// Get a free projectile...
	m_pProjToThrow = CEProjPool::GetProjectileFromFreePool( m_ahProjPool[m_nUpgradeLevel] );
	if( m_pProjToThrow == NULL ) {
		// No more projectiles...
		return FALSE;
	}

	const _UserProps_t *pUserProps = &m_aUserProps[m_nUpgradeLevel];

	// Init projectile...
	CEProjExt::CEProj_Grenade_Params_t GrenadeParams;

	GrenadeParams.nGrenadeType = CEProjExt::CEPROJ_GRENADE_TYPE_RECRUITER;
	GrenadeParams.FloorDampening.Set( 0.85f, 0.65f, 0.85f );
	GrenadeParams.WallDampening.Set( 0.95f, 0.95f, 0.95f );
	GrenadeParams.bUseAngularWallDampening = TRUE;
	GrenadeParams.afAngularWallDampeningRange[0] = 0.2f;
	GrenadeParams.afAngularWallDampeningRange[1] = 0.4f;
	GrenadeParams.fRadiusPercentUse = 0.6f;
	GrenadeParams.fFatRadiusDelta = pUserProps->fFatRadiusDelta;
	GrenadeParams.fDetonationRadius = pUserProps->fDetonationRadius;
	GrenadeParams.fDetonationLife = pUserProps->fDetonationLife;
	GrenadeParams.fShutdownTime = pUserProps->fShutdownTime;
	GrenadeParams.pSoundGroupBounce = pUserProps->pSoundGroupBounce;
	GrenadeParams.pSoundGroupEMPEffect = pUserProps->pSoundGroupEffect;
	GrenadeParams.pSoundGroupRecruiterBolt = pUserProps->pSoundGroupBolt;

	m_pProjToThrow->Init();
	m_pProjToThrow->SetDamager( this, m_pOwnerBot );
	m_pProjToThrow->SetMaxDistCanTravel( pUserProps->fMaxLiveRange );
	m_pProjToThrow->SetExplosionGroup( pUserProps->hExplosionGroup );
	m_pProjToThrow->SetGrenadeParams( &GrenadeParams );
	m_pProjToThrow->Relocate_RotXlatFromUnitMtx_WS_NewScale_WS( &CFMtx43A::m_IdentityMtx, pUserProps->fGrenadeMeshScale, FALSE );

	return TRUE;
}


BOOL CWeaponRecruiter::Throwable_Prime( void ) {
	FASSERT( IsCreated() );

	if( !_GetProjectileFromPoolAndInit() ) {
		// No more projectiles...
		return FALSE;
	}

	return TRUE;
}


void CWeaponRecruiter::Throwable_AttachGrenadeToOwnerBotBone( cchar *pszBoneName ) {
	FASSERT( IsCreated() );

	if( m_pOwnerBot ) {
		if( m_pProjToThrow ) {
			m_pProjToThrow->Attach_UnitMtxToParent_PS( m_pOwnerBot, pszBoneName, &CFMtx43A::m_IdentityMtx, FALSE );
			m_pProjToThrow->AddToWorld();
		}
	}
}


// Throws the active grenade toward the specified point.
void CWeaponRecruiter::Throwable_ThrowGrenade_TargetPoint( const CFVec3A *pTargetPos_WS, f32 fSpeed ) {
	FASSERT( IsCreated() );
	FASSERT( m_pProjToThrow );

	FASSERT_NOW;	// TBD!!!

	m_pProjToThrow->DetachFromParent();
}


// Throws the active grenade toward the owner bot's mount aim direction.
void CWeaponRecruiter::Throwable_ThrowGrenade_MountAimDirection( f32 fSlingSpeed, BOOL bLimbDangling ) {
	f32 fUnitAimPitch, fProjSpeed;
	CFQuatA Quat;
	CFVec3A LaunchVelocity_WS, BotAddVelocity_WS, LaunchUnitVec_WS;

	FASSERT( IsCreated() );
	FASSERT( m_pOwnerBot );
	FASSERT( m_pProjToThrow );

	// Detach from any parent...
	m_pProjToThrow->DetachFromParent();

	if( fSlingSpeed < 0.0f ) {
		f32 fRandomSpeedMult = 1.0f;
		f32 fRandomAngleSum = 0.0f;

		if( bLimbDangling ) {
			fRandomSpeedMult = fmath_RandomFloatRange( 0.5f, 0.75f );
			fRandomAngleSum = fmath_RandomFloatRange( FMATH_DEG2RAD(-40.0f), FMATH_DEG2RAD(40.0f) );
		}

		// Compute speed...
		fUnitAimPitch = 1.01f - fmath_Div( m_pOwnerBot->m_fMountPitch_WS - m_pOwnerBot->m_fMountPitchMin_WS, m_pOwnerBot->m_fMountPitchMax_WS - m_pOwnerBot->m_fMountPitchMin_WS );
		fUnitAimPitch = fmath_Sin( fUnitAimPitch * FMATH_HALF_PI );
		fProjSpeed = fRandomSpeedMult * FMATH_FPOT( fUnitAimPitch, 25.0f, 60.0f );

		// Compute velocity vector...
		Quat.BuildQuat( m_pOwnerBot->MtxToWorld()->m_vRight, fRandomAngleSum + FMATH_DEG2RAD( -20.0f ) );
		Quat.MulPoint( LaunchVelocity_WS, m_pOwnerBot->MtxToWorld()->m_vFront );
		LaunchVelocity_WS.Mul( fProjSpeed );
	} else {
		fProjSpeed = fSlingSpeed;

		Quat.BuildQuat( m_pOwnerBot->MtxToWorld()->m_vRight, FMATH_DEG2RAD( -10.0f ) );
		Quat.MulPoint( LaunchVelocity_WS, m_pOwnerBot->MtxToWorld()->m_vFront );
		LaunchVelocity_WS.Mul( fProjSpeed );

		m_pProjToThrow->SetDetonateOnImpact( TRUE );
	}

	// Add in bot's initial velocity and extract unit trajectory and speed...
	BotAddVelocity_WS = m_pOwnerBot->m_Velocity_MS;
	BotAddVelocity_WS.x = 0.0f;
	m_pOwnerBot->MS2WS( BotAddVelocity_WS );

	LaunchVelocity_WS.Add( BotAddVelocity_WS );
	fProjSpeed = LaunchUnitVec_WS.SafeUnitAndMag( LaunchVelocity_WS );
	if( fProjSpeed <= 0.0f ) {
		fProjSpeed = 0.0f;
		LaunchUnitVec_WS = CFVec3A::m_UnitAxisY;
	}

	m_pProjToThrow->SetLinSpeed( fProjSpeed );
	m_pProjToThrow->SetLinUnitDir_WS( &LaunchUnitVec_WS );

	CFVec3A RotVel_WS;

	if( fmath_RandomChance( 0.5f ) ) {
		RotVel_WS.Set( -fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	} else {
		RotVel_WS.Set( fmath_RandomFloatRange( 0.1f, 1.0f), fmath_RandomBipolarUnitFloat(), fmath_RandomBipolarUnitFloat() );
	}
	RotVel_WS.Unitize();

	// Launch it!
	m_pProjToThrow->SetRotUnitAxis_WS( &RotVel_WS );
	m_pProjToThrow->SetRotSpeed( fmath_RandomFloatRange( FMATH_DEG2RAD( 180.0f ), FMATH_DEG2RAD( 360.0f ) ) );
	m_pProjToThrow->SetMaxLifeSecs( m_aUserProps[m_nUpgradeLevel].fGrenadeLife );

	if( fSlingSpeed < 0.0f ) {
		m_pProjToThrow->SmokeTrailOn( &m_SmokeTrailAttrib );
		m_pProjToThrow->SetSmokePos_MS( &CFVec3A( 0.0f, 0.5f, 0.0f ) );
	} else {
		m_pProjToThrow->StreamerOn( &m_StreamerTexInst, 0.15f, CFXStreamerEmitter::USE_RIGHT_AXIS, 1.0f, 10 );
	}

	m_pProjToThrow->SetSkipListCallback( _BuildSkipList );
	m_pProjToThrow->Launch();
	m_pProjToThrow = NULL;

	// Remove the round we launched from the clip...
	RemoveFromClip( 1 );

	PlaySound( m_aUserProps[m_nUpgradeLevel].pSoundGroupUse );
}


void CWeaponRecruiter::Throwable_ReturnGrenade( void ) {
	FASSERT( IsCreated() );

	if( m_pProjToThrow ) {
		m_pProjToThrow->DetachFromParent();
		CEProjPool::ReturnProjectileToFreePool( m_pProjToThrow );
		m_pProjToThrow = NULL;
	}
}


// Called once per frame to update animations, timers, etc.
void CWeaponRecruiter::ClassHierarchyWork( void ) {
	// Call our parent. It might need to do some work...
	CWeapon::ClassHierarchyWork();

	// Exit if we don't have any work to do...
	if( !IsOurWorkBitSet() ) {
		return;
	}

	if( m_fSecondsCountdownTimer > 0.0f ) {
		m_fSecondsCountdownTimer -= FLoop_fPreviousLoopSecs;
		FMATH_CLAMPMIN( m_fSecondsCountdownTimer, 0.0f );
	}
}


void CWeaponRecruiter::_BuildSkipList( CEProj *pProj ) {
	CWeaponRecruiter *pWeaponGren = (CWeaponRecruiter *)pProj->GetDamagerWeapon();
	pWeaponGren->m_pOwnerBot->AppendTrackerSkipList();
}


void CWeaponRecruiter::ClassHierarchyDrawEnable( BOOL bDrawingHasBeenEnabled ) {
	CWeapon::ClassHierarchyDrawEnable( bDrawingHasBeenEnabled );
	
	if( m_pProjToThrow ) {
		m_pProjToThrow->DrawEnable(bDrawingHasBeenEnabled);
	}
}


void CWeaponRecruiter::UnrecruitAllBots( BOOL bMakeEffects ) {
	if( m_pIconMeshPool == NULL ) {
		return;
	}

	u32 i;

	for( i=0; i<m_pIconMeshPool->GetMaxItemCount(); ++i ) {
		if( _pRecruitDataArray[i].m_pBot ) {
			UnrecruitBot( _pRecruitDataArray[i].m_pBot, bMakeEffects );
		}
	}
}


// Attempts to recruite the specified bot.
//
// Returns TRUE if successful.
// Returns FALSE if unsuccessful.
BOOL CWeaponRecruiter::RecruitBot( CBot *pBot, CBot *pRecruitingBot, const CFVec3A *pEpicenter_WS ) {
	if( m_pIconMeshPool == NULL ) {
		return FALSE;
	}

	FASSERT( pBot );
	FASSERT( pRecruitingBot );
	FASSERT( pBot->m_nRecruitID < 0 );

	CFVec3A BotCylinderCenter_WS;

	_ComputeBotCylinderCenter( &BotCylinderCenter_WS, pBot );

	if( pEpicenter_WS ) {
		FWorld_nTrackerSkipListCount = 0;
		pBot->AppendTrackerSkipList();

		if( fworld_IsLineOfSightObstructed( pEpicenter_WS, &BotCylinderCenter_WS, FWorld_nTrackerSkipListCount, FWorld_apTrackerSkipList, pBot->GetMesh(), -1, FCOLL_MASK_COLLIDE_WITH_THIN_PROJECTILES ) ) {
			// Path is obstructed from epicenter to bot...
			return FALSE;
		}
	}

	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetFromFreePool();

	if( pWorldMeshItem == NULL ) {
		// Cannot recruit any more bots...

		u32 i;

		for( i=0; i<m_pIconMeshPool->GetMaxItemCount(); ++i ) {
			if( _pRecruitDataArray[i].m_pBot ) {
				if( !(_pRecruitDataArray[i].m_pBot->m_pWorldMesh->GetVolumeFlags() & FVIS_VOLUME_IN_ACTIVE_LIST) ) {
					// This bot is no longer in the active list. Let's use his recruitment data slot...

					UnrecruitBot( _pRecruitDataArray[i].m_pBot, FALSE );

					break;
				}
			}
		}

		CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetFromFreePool();

		if( pWorldMeshItem == NULL ) {
			// Cannot recruit any more bots...
			return FALSE;
		}
	}

	// Set the icon color to match the recruiting bot
	const CFColorRGBA& rIconColor = Multiplayer_PlayerColor( pRecruitingBot, TRUE );
	pWorldMeshItem->SetMeshTint( rIconColor.fRed, rIconColor.fGreen, rIconColor.fBlue );

	s32 nRecruitID = pWorldMeshItem->GetPoolIndex();

	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == NULL );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;

	pRecruitData->m_pBot = pBot;
	pRecruitData->m_pRecruiterBot = pRecruitingBot;
	pRecruitData->m_nNewRace = ai_GetRace(pRecruitingBot->AIBrain());
	pRecruitData->m_nNewTeam = (u8)pRecruitingBot->GetOrigTeam();
	pRecruitData->m_bEpicenterProvided = FALSE;
	pRecruitData->m_nState = CRecruitData::STATE_SHOCKING;
	pRecruitData->m_fTimer = _RECRUITED_BOT_SHOCK_SECS;
	pRecruitData->m_fIconUnitOffsetY = 0.0f;
	pRecruitData->m_fIconUnitScale = 0.0f;

	if( pEpicenter_WS ) {
		pRecruitData->m_bEpicenterProvided = TRUE;
		pRecruitData->m_Epicenter_WS = pEpicenter_WS->v3;
	} else {
		pRecruitData->m_bEpicenterProvided = FALSE;
		pRecruitData->m_Epicenter_WS.Zero();
	}

	CAIBrain *pBrain = pBot->AIBrain();

	pRecruitData->m_nOrigTeam = pBot->GetOrigTeam();
	if (pBrain) {
		pRecruitData->m_nOrigRace = ai_GetRace( pBrain );
		pRecruitData->m_nOrigAgression = pBrain->GetAttrib( CAIBrain::ATTRIB_AGGRESSION );
		pRecruitData->m_nOrigIntelligence = pBrain->GetAttrib( CAIBrain::ATTRIB_INTELLIGENCE );
		pRecruitData->m_nOrigCourage = pBrain->GetAttrib( CAIBrain::ATTRIB_COURAGE );
		pRecruitData->m_nOrigBuddyBrainFlags = pBrain->GetFlag_Buddy_Ctrl_Auto() | pBrain->GetFlag_Buddy_Ctrl_OnAction() | pBrain->GetFlag_Buddy_Ctrl_NoWeaponStow();

		if( pBrain->GetAIMover()->GetWeaponCtrlPtr(0) ) {
			pRecruitData->m_fOrigWeaponUnitAccuracy = pBrain->GetAIMover()->GetWeaponCtrlPtr(0)->m_fAccuracy;
		} else {
			pRecruitData->m_fOrigWeaponUnitAccuracy = 0.0f;
		}
	}	else { // is SiteWeapon
		pRecruitData->m_nOrigRace = pBot->GetRace();
		pRecruitData->m_nOrigAgression = 0;
		pRecruitData->m_nOrigIntelligence = 0;
		pRecruitData->m_nOrigCourage = 0;
		pRecruitData->m_nOrigBuddyBrainFlags = 0;
	}

	pBot->m_nRecruitID = nRecruitID;
	pBot->m_nRecruitPlayerIndex = pRecruitingBot->m_nPossessionPlayerIndex;

	pBot->DataPort_Reserve( TRUE );
	pBot->DataPort_Shock( TRUE );

	if (pBrain)
	{//now that teams have been switched up, probably should do this too
		if (pBrain->GetNumFollowers())
		{
			//go through list of followers and yank any that don't match team
			pBrain->BootUnhappyFollowers();
		}
	}

	return TRUE;
}


void CWeaponRecruiter::UnrecruitBot( CBot *pBot, BOOL bMakeEffect ) {
	if( m_pIconMeshPool == NULL ) {
		return;
	}

	FASSERT( pBot );

	s32 nRecruitID = pBot->m_nRecruitID;

	FASSERT( nRecruitID >= 0 );
	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == pBot );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;
	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetItemByIndex( nRecruitID );

	if( bMakeEffect ) {
		if( pBot->m_nPossessionPlayerIndex >= 0 ) {
			if( pBot->IsDrawEnabled() ) {
				if( m_aUserProps[0].hParticleIconOff != FPARTICLE_INVALID_HANDLE ) {
					fparticle_SpawnEmitter( m_aUserProps[0].hParticleIconOff, pWorldMeshItem->m_Xfm.m_MtxF.m_vPos.v3, &CFVec3A::m_UnitAxisY.v3, 1.0f );
				}
			}
		}
	}

	pBot->DataPort_Reserve( FALSE );
	pBot->DataPort_Shock( FALSE );

	m_pIconMeshPool->ReturnToFreePool( pWorldMeshItem );

	pRecruitData->m_nState = CRecruitData::STATE_UNRECRUITED;
	pRecruitData->m_pBot = NULL;
	pRecruitData->m_pRecruiterBot = NULL;

	CAIBrain *pBrain = pBot->AIBrain();

	pBot->SetTeam( pRecruitData->m_nOrigTeam );
	if (pBrain &&
		pBot->IsInWorld())	{  //pgm added the IsInWOrld because I just don' see how this recruiter weapon can know that it is safe to unrecruit this guy. and not all these AI functions would be happy
		ai_SetRace( pBrain, pRecruitData->m_nOrigRace );
		pBrain->AttribStack_Push( CAIBrain::ATTRIB_AGGRESSION, pRecruitData->m_nOrigAgression );
		pBrain->AttribStack_Push( CAIBrain::ATTRIB_INTELLIGENCE, pRecruitData->m_nOrigIntelligence );
		pBrain->AttribStack_Push( CAIBrain::ATTRIB_COURAGE, pRecruitData->m_nOrigCourage );

		ai_ChangeBuddyCtrl( pBrain, AI_BUDDY_CTRL_NEVER );

		if( pRecruitData->m_nOrigBuddyBrainFlags & CAIBrain::BRAINFLAG_BUDDY_CTRL_ONACTION ) {
			pBrain->SetFlag_Buddy_Ctrl_OnAction();
		}

		if( pRecruitData->m_nOrigBuddyBrainFlags & CAIBrain::BRAINFLAG_BUDDY_CTRL_AUTO ) {
			pBrain->SetFlag_Buddy_Ctrl_Auto();
		}

		if( pRecruitData->m_nOrigBuddyBrainFlags & CAIBrain::BRAINFLAG_BUDDY_CTRL_NOWEAPONSTOW ) {
			pBrain->SetFlag_Buddy_Ctrl_NoWeaponStow();
		}

		if( pBrain->GetAIMover()->GetWeaponCtrlPtr(0) ) {
			pBrain->GetAIMover()->GetWeaponCtrlPtr(0)->m_fAccuracy = pRecruitData->m_fOrigWeaponUnitAccuracy;
		}

		ai_ResetToJob( pBrain );

		if (pBrain->GetNumFollowers())
		{
			//go through list of followers and yank any that don't match team
			pBrain->BootUnhappyFollowers();
		}
	}
	else {
		switch(pRecruitData->m_nOrigRace) 
		{
		case AIRACE_MIL:
			pBot->SetRace(BOTRACE_MIL);
			break;
		case AIRACE_DROID:
			pBot->SetRace(BOTRACE_DROID);
			break;
		case AIRACE_ZOMBIE:
			pBot->SetRace(BOTRACE_ZOMBIE);
			break;
		case AIRACE_AMBIENT:
			pBot->SetRace(BOTRACE_AMBIENT);
		default:
			break;
		}
	}
	
	pBot->m_nRecruitID = -1;
	pBot->m_nRecruitPlayerIndex = -1;
}

void CWeaponRecruiter::SetUserViewMode( CBot* pBot, BOOL bUserView ) {
	FASSERT( pBot );

	s32 nRecruitID = pBot->m_nRecruitID;

	FASSERT( nRecruitID >= 0 );
	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == pBot );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;
	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetItemByIndex( nRecruitID );

	if( pWorldMeshItem->IsAddedToWorld() ) {
		if( (pRecruitData->m_nState == CRecruitData::STATE_INTRODUCING_ICON) ||
			(pRecruitData->m_nState == CRecruitData::STATE_SPINNING_ICON) ) {

			CFMtx43A Mtx;
			Mtx.Identity();

			if( pRecruitData->m_nState == CRecruitData::STATE_SPINNING_ICON ) {
				Mtx.RotateY( pRecruitData->m_fTimer );
			}

			Mtx.m_vPos.Mul( CFVec3A::m_UnitAxisY, pBot->m_fCollCylinderHeight_WS + pRecruitData->m_fIconUnitOffsetY * _ICON_INTRODUCTION_ANIM_AMP_Y );
			Mtx.m_vPos.Add( pBot->MtxToWorld()->m_vPos );

			f32 fScale = pRecruitData->m_fIconUnitScale;
			if( bUserView )
				fScale *= 0.5f;

			FMATH_CLAMPMIN( fScale, 0.001f );
			pWorldMeshItem->m_Xfm.BuildFromMtx( Mtx, fScale * _ICON_INTRODUCTION_ANIM_AMP_SCALE );
			pWorldMeshItem->UpdateTracker();
		}
	}
}

void CWeaponRecruiter::UpdateRecruiterIconPos( CBot *pBot ) {
	FASSERT( pBot );

	s32 nRecruitID = pBot->m_nRecruitID;

	FASSERT( nRecruitID >= 0 );
	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == pBot );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;
	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetItemByIndex( nRecruitID );

	if( MultiplayerMgr.IsSinglePlayer() && (pBot->m_nPossessionPlayerIndex >= 0) ) {
		pWorldMeshItem->RemoveFromWorld();
		return;
	}

	switch( pRecruitData->m_nState ) {
	case CRecruitData::STATE_UNRECRUITED:
	case CRecruitData::STATE_SHOCKING:
		pWorldMeshItem->RemoveFromWorld();
		break;

	case CRecruitData::STATE_INTRODUCING_ICON:
	case CRecruitData::STATE_SPINNING_ICON:
		{
			CFMtx43A Mtx;

			Mtx.Identity();

			if( pRecruitData->m_nState == CRecruitData::STATE_SPINNING_ICON ) {
				Mtx.RotateY( pRecruitData->m_fTimer );
			}

			Mtx.m_vPos.Mul( CFVec3A::m_UnitAxisY, pBot->m_fCollCylinderHeight_WS + pRecruitData->m_fIconUnitOffsetY * _ICON_INTRODUCTION_ANIM_AMP_Y );
			Mtx.m_vPos.Add( pBot->MtxToWorld()->m_vPos );

			FMATH_CLAMPMIN( pRecruitData->m_fIconUnitScale, 0.001f );
			pWorldMeshItem->m_Xfm.BuildFromMtx( Mtx, pRecruitData->m_fIconUnitScale * _ICON_INTRODUCTION_ANIM_AMP_SCALE );
			pWorldMeshItem->UpdateTracker();
		}

		break;
	};
}


void CWeaponRecruiter::RecruitWork( CBot *pBot ) {
	s32 nRecruitID = pBot->m_nRecruitID;
	FASSERT( nRecruitID >= 0 );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;

	switch( pRecruitData->m_nState ) {
	case CRecruitData::STATE_SHOCKING:
		pRecruitData->m_fTimer -= FLoop_fPreviousLoopSecs;
		if( pRecruitData->m_fTimer <= 0.0f ) {
			// Done shocking...

			pRecruitData->m_pBot->DataPort_Reserve( FALSE );
			pRecruitData->m_pBot->DataPort_Shock( FALSE );
			pRecruitData->m_nState = CRecruitData::STATE_INTRODUCING_ICON;
			pRecruitData->m_fTimer = _ICON_INTRODUCTION_ANIM_SECS;

			pRecruitData->m_pBot->MobilizeBot();
			pRecruitData->m_pBot->WakeUp();
			pRecruitData->m_pBot->Power( TRUE );

			_SetAIToRecruitedState( pRecruitData );

			UpdateRecruiterIconPos( pBot );
		}

		break;

	case CRecruitData::STATE_INTRODUCING_ICON:
		pRecruitData->m_fTimer -= FLoop_fPreviousLoopSecs;
		if( pRecruitData->m_fTimer > 0.0f ) {
			f32 fUnitVal = (_ICON_INTRODUCTION_ANIM_SECS - pRecruitData->m_fTimer) * (1.0f / _ICON_INTRODUCTION_ANIM_SECS);

			pRecruitData->m_fIconUnitOffsetY = fmath_Sin( fUnitVal * FMATH_HALF_PI );

			pRecruitData->m_fIconUnitScale = fUnitVal + 4.84f * (1.0f - fUnitVal) * fmath_Sqrt( fUnitVal ) * fmath_PositiveSin( fUnitVal * (FMATH_PI * 2.5f) );
		} else {
			// Done playing icon introduction animation...

			pRecruitData->m_nState = CRecruitData::STATE_SPINNING_ICON;
			pRecruitData->m_fTimer = 0.0f;
			pRecruitData->m_fIconUnitOffsetY = 1.0f;
			pRecruitData->m_fIconUnitScale = 1.0f;
		}

		break;

	case CRecruitData::STATE_SPINNING_ICON:
		pRecruitData->m_fTimer += (FMATH_2PI * _ICON_SPIN_REVS_PER_SEC) * FLoop_fPreviousLoopSecs;
		while( pRecruitData->m_fTimer >= FMATH_2PI ) {
			pRecruitData->m_fTimer -= FMATH_2PI;
		}

		pRecruitData->m_fIconUnitScale = 1.0f + 0.1f * fmath_Sin( pRecruitData->m_fTimer * 3.0f );

		break;
	};
}


void CWeaponRecruiter::BotDrawEnable( CBot *pBot, BOOL bDrawingHasBeenEnabled ) {
	FASSERT( pBot );

	s32 nRecruitID = pBot->m_nRecruitID;

	FASSERT( nRecruitID >= 0 );
	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == pBot );

	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetItemByIndex( nRecruitID );

	if( bDrawingHasBeenEnabled ) {
		UpdateRecruiterIconPos( pBot );
	} else {
		pWorldMeshItem->RemoveFromWorld();
	}
}


void CWeaponRecruiter::BotCheckpointSaved( CBot *pBot ) {
	FASSERT( pBot );

	s32 nRecruitID = pBot->m_nRecruitID;

	if( nRecruitID < 0 ) {
		// Bot not recruited...

		CFCheckPoint::SaveData( (BOOL)FALSE );

		return;
	}

	// Bot is recruited...

	CFCheckPoint::SaveData( (BOOL)TRUE );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;

	CFCheckPoint::SaveData( (u32&)pRecruitData->m_pRecruiterBot );
	CFCheckPoint::SaveData( pRecruitData->m_nNewRace );
	CFCheckPoint::SaveData( pRecruitData->m_nNewTeam );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigTeam );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigRace );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigAgression );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigIntelligence );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigCourage );
	CFCheckPoint::SaveData( pRecruitData->m_nOrigBuddyBrainFlags );
	CFCheckPoint::SaveData( pRecruitData->m_fOrigWeaponUnitAccuracy );
}


void CWeaponRecruiter::BotCheckpointRestored( CBot *pBot ) {
	BOOL bRecruited = FALSE;

	pBot->m_nRecruitID = -1;

	CFCheckPoint::LoadData( bRecruited );

	if( !bRecruited ) {
		// This bot was not recruited...

		return;
	}

	// This bot was recruited...

	CFWorldMeshItem *pWorldMeshItem = m_pIconMeshPool->GetFromFreePool();

	if( pWorldMeshItem == NULL ) {
		// Cannot recruit any more bots...
		return;
	}

	s32 nRecruitID = pWorldMeshItem->GetPoolIndex();

	FASSERT( _pRecruitDataArray[nRecruitID].m_pBot == NULL );

	CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;

	CFCheckPoint::LoadData( (u32&)pRecruitData->m_pRecruiterBot );
	CFCheckPoint::LoadData( pRecruitData->m_nNewRace );
	CFCheckPoint::LoadData( pRecruitData->m_nNewTeam );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigTeam );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigRace );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigAgression );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigIntelligence );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigCourage );
	CFCheckPoint::LoadData( pRecruitData->m_nOrigBuddyBrainFlags );
	CFCheckPoint::LoadData( pRecruitData->m_fOrigWeaponUnitAccuracy );

	pRecruitData->m_pBot = pBot;

	pRecruitData->m_nState = CRecruitData::STATE_SPINNING_ICON;
	pRecruitData->m_bEpicenterProvided = FALSE;
	pRecruitData->m_fTimer = 0.0f;
	pRecruitData->m_fIconUnitOffsetY = 1.0f;
	pRecruitData->m_fIconUnitScale = 1.0f;
	pRecruitData->m_Epicenter_WS.Zero();

	pBot->DataPort_Shock( FALSE );
	pBot->MobilizeBot();

	_SetAIToRecruitedState( pRecruitData );

	pBot->m_nRecruitID = nRecruitID;
	if( pRecruitData->m_pRecruiterBot )
		pBot->m_nRecruitPlayerIndex = pRecruitData->m_pRecruiterBot->m_nPossessionPlayerIndex;

	UpdateRecruiterIconPos( pBot );
}

BOOL CWeaponRecruiter::WasRecruitedBy( CBot *pBot, CBot *pRecruitingBot ) {
	FASSERT( pBot );
	FASSERT( pRecruitingBot );

	return (pBot->Recruit_GetRecruiter() == pRecruitingBot->m_nPossessionPlayerIndex);
	//s32 nRecruitID = pBot->m_nRecruitID;
	//CRecruitData *pRecruitData = _pRecruitDataArray + nRecruitID;
	//return (pRecruitingBot == pRecruitData->m_pRecruiterBot);
}

void CWeaponRecruiter::_SetAIToRecruitedState( CRecruitData *pRecruitData ) {

	// Always set the team, brain or not
	pRecruitData->m_pBot->SetTeam( pRecruitData->m_nNewTeam );

	CAIBrain *pBrain = pRecruitData->m_pBot->AIBrain();
	if (pBrain)
	{
		ai_SetRace( pBrain, pRecruitData->m_nNewRace );

		// Yeah, you have to call this twice...
		ai_ChangeBuddyCtrl( pBrain, AI_BUDDY_CTRL_NEVER );
		ai_ChangeBuddyCtrl( pBrain, AI_BUDDY_CTRL_AUTO );

		// If this is multiplayer, we want to set the bot wandering immediately,
		// otherwise he just sort of stands there. Normally he will see the player
		// right away and buddy up with him.
		if( MultiplayerMgr.IsMultiplayer() ) {
			ai_AssignJob_Wander( pBrain, 5000.0f, FALSE, 0, 0, 0, 0, 0, NULL );
		}

		pBrain->AttribStack_Push( CAIBrain::ATTRIB_AGGRESSION, 100 );
		pBrain->AttribStack_Push( CAIBrain::ATTRIB_INTELLIGENCE, 100 );
		pBrain->AttribStack_Push( CAIBrain::ATTRIB_COURAGE, 100 );
		if( pBrain->GetAIMover()->GetWeaponCtrlPtr(0) ) {
			pBrain->GetAIMover()->GetWeaponCtrlPtr(0)->m_fAccuracy = 1.0f;
		}

		ai_ResetToJob( pBrain );
	}
	else
	{
		switch(pRecruitData->m_nNewRace) 
		{
			case AIRACE_MIL:
				pRecruitData->m_pBot->SetRace(BOTRACE_MIL);
				break;
			case AIRACE_DROID:
				pRecruitData->m_pBot->SetRace(BOTRACE_DROID);
				break;
			case AIRACE_ZOMBIE:
				pRecruitData->m_pBot->SetRace(BOTRACE_ZOMBIE);
				break;
			case AIRACE_AMBIENT:
				pRecruitData->m_pBot->SetRace(BOTRACE_AMBIENT);
			default:
				break;
		}
	}
}


void CWeaponRecruiter::WorkAll( void ) {
	if( m_pIconMeshPool == NULL ) {
		return;
	}

	u32 i;
	f32 fLightRadius;
	CFColorMotif ColorMotif;
	CRecruitData *pRecruitData;
	CFVec3A LightPos_WS, WeightedStartPos_WS, WeightedEndPos_WS;

	for( i=0; i<m_pIconMeshPool->GetMaxItemCount(); ++i ) {
		pRecruitData = _pRecruitDataArray + i;

		if( pRecruitData->m_nState == CRecruitData::STATE_SHOCKING ) {
			if( pRecruitData->m_bEpicenterProvided ) {
				m_BoltData.m_StartPos_WS.Set( pRecruitData->m_Epicenter_WS );

				_ComputeBotCylinderCenter( &m_BoltData.m_EndPos_WS, pRecruitData->m_pBot );

				fLightRadius = 1.2f * m_BoltData.m_StartPos_WS.Dist( m_BoltData.m_EndPos_WS );

				if( fLightRadius > 1.0f ) {
					ColorMotif = FColor_MotifWhite;
					ColorMotif.nMotifIndex = fcolor_GetRandomMotif( FCOLORMOTIF_ROCKET0 );

					WeightedStartPos_WS.Mul( m_BoltData.m_StartPos_WS, 0.25f );
					WeightedEndPos_WS.Mul( m_BoltData.m_EndPos_WS, 0.75f );

					LightPos_WS.Add( WeightedStartPos_WS, WeightedEndPos_WS );

					CMuzzleFlash::MuzzleLight( &LightPos_WS, fLightRadius, 1.0f, &ColorMotif );
				}
			}
		}
	}
}


// Assumes the FDraw renderer is currently active.
// Assumes the game's main perspective camera is
// set up. Assumes there are no model Xfms on the
// stack.
//
// Does not preserve the current FDraw state.
// Does not preserve the current viewport.
// Will preserve the Xfm stack.
// Will preserve the frenderer fog state.
void CWeaponRecruiter::DrawAll( void ) {
	if( m_pIconMeshPool == NULL ) {
		return;
	}

	u32 i;
	CRecruitData *pRecruitData;

	for( i=0; i<m_pIconMeshPool->GetMaxItemCount(); ++i ) {
		pRecruitData = _pRecruitDataArray + i;

		if( pRecruitData->m_nState == CRecruitData::STATE_SHOCKING ) {
			if( pRecruitData->m_bEpicenterProvided ) {
				m_BoltData.m_StartPos_WS.Set( pRecruitData->m_Epicenter_WS );

				_ComputeBotCylinderCenter( &m_BoltData.m_EndPos_WS, pRecruitData->m_pBot );

				m_BoltData.m_fTrumpetRadius = 2.0f * FMATH_MAX( 0.5f * pRecruitData->m_pBot->m_fCollCylinderHeight_WS, pRecruitData->m_pBot->m_fCollCylinderRadius_WS );

				CFBolt::Draw( &m_BoltData );
			}
		}
	}
}


void CWeaponRecruiter::_ComputeBotCylinderCenter( CFVec3A *pPos_WS, CBot *pBot ) {
	*pPos_WS = pBot->MtxToWorld()->m_vPos;
	pPos_WS->y += 0.5f * pBot->m_fCollCylinderHeight_WS;
}


